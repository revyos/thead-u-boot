// SPDX-License-Identifier: MIT OR BSD-3-Clause
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

/* Implementation of RSA signature verification which uses a pre-processed
 * key for computation. The code extends libmincrypt RSA verification code to
 * support multiple RSA key lengths and hash digest algorithms.
 */

#include "avb_rsa.h"
#include "avb_sha.h"
#include "avb_util.h"
#include "avb_vbmeta_image.h"

#if defined(CONFIG_AVB_HW_ENGINE_ENABLE)
#include "sec_library.h"
#endif

typedef struct IAvbKey {
  unsigned int len; /* Length of n[] in number of uint32_t */
  uint32_t n0inv;   /* -1 / n[0] mod 2^32 */
  uint32_t* n;      /* modulus as array (host-byte order) */
  uint32_t* rr;     /* R^2 as array (host-byte order) */
} IAvbKey;

static IAvbKey* iavb_parse_key_data(const uint8_t* data, size_t length) {
  AvbRSAPublicKeyHeader h;
  IAvbKey* key = NULL;
  size_t expected_length;
  unsigned int i;
  const uint8_t* n;
  const uint8_t* rr;

  if (!avb_rsa_public_key_header_validate_and_byteswap(
          (const AvbRSAPublicKeyHeader*)data, &h)) {
    avb_error("Invalid key.\n");
    goto fail;
  }

  if (!(h.key_num_bits == 2048 || h.key_num_bits == 4096 ||
        h.key_num_bits == 8192)) {
    avb_error("Unexpected key length.\n");
    goto fail;
  }

  expected_length = sizeof(AvbRSAPublicKeyHeader) + 2 * h.key_num_bits / 8;
  if (length != expected_length) {
    avb_error("Key does not match expected length.\n");
    goto fail;
  }

  n = data + sizeof(AvbRSAPublicKeyHeader);
  rr = data + sizeof(AvbRSAPublicKeyHeader) + h.key_num_bits / 8;

  /* Store n and rr following the key header so we only have to do one
   * allocation.
   */
  key = (IAvbKey*)(avb_malloc(sizeof(IAvbKey) + 2 * h.key_num_bits / 8));
  if (key == NULL) {
    goto fail;
  }

  key->len = h.key_num_bits / 32;
  key->n0inv = h.n0inv;
  key->n = (uint32_t*)(key + 1); /* Skip ahead sizeof(IAvbKey) bytes. */
  key->rr = key->n + key->len;

  /* Crypto-code below (modpowF4() and friends) expects the key in
   * little-endian format (rather than the format we're storing the
   * key in), so convert it.
   */
  for (i = 0; i < key->len; i++) {
    key->n[i] = avb_be32toh(((uint32_t*)n)[key->len - i - 1]);
    key->rr[i] = avb_be32toh(((uint32_t*)rr)[key->len - i - 1]);
  }
  return key;

fail:
  if (key != NULL) {
    avb_free(key);
  }
  return NULL;
}

static void iavb_free_parsed_key(IAvbKey* key) {
  avb_free(key);
}
#if defined(CONFIG_AVB_HW_ENGINE_ENABLE)

static void hw_crypto_accel_init(void)
{
	static bool init = false;

	if (!init) {
		rambus_crypto_init();
		init = true;
	}
}

#else
/* a[] -= mod */
static void subM(const IAvbKey* key, uint32_t* a) {
  int64_t A = 0;
  uint32_t i;
  for (i = 0; i < key->len; ++i) {
    A += (uint64_t)a[i] - key->n[i];
    a[i] = (uint32_t)A;
    A >>= 32;
  }
}

/* return a[] >= mod */
static int geM(const IAvbKey* key, uint32_t* a) {
  uint32_t i;
  for (i = key->len; i;) {
    --i;
    if (a[i] < key->n[i]) {
      return 0;
    }
    if (a[i] > key->n[i]) {
      return 1;
    }
  }
  return 1; /* equal */
}

/* montgomery c[] += a * b[] / R % mod */
static void montMulAdd(const IAvbKey* key,
                       uint32_t* c,
                       const uint32_t a,
                       const uint32_t* b) {
  uint64_t A = (uint64_t)a * b[0] + c[0];
  uint32_t d0 = (uint32_t)A * key->n0inv;
  uint64_t B = (uint64_t)d0 * key->n[0] + (uint32_t)A;
  uint32_t i;

  for (i = 1; i < key->len; ++i) {
    A = (A >> 32) + (uint64_t)a * b[i] + c[i];
    B = (B >> 32) + (uint64_t)d0 * key->n[i] + (uint32_t)A;
    c[i - 1] = (uint32_t)B;
  }

  A = (A >> 32) + (B >> 32);

  c[i - 1] = (uint32_t)A;

  if (A >> 32) {
    subM(key, c);
  }
}

/* montgomery c[] = a[] * b[] / R % mod */
static void montMul(const IAvbKey* key, uint32_t* c, uint32_t* a, uint32_t* b) {
  uint32_t i;
  for (i = 0; i < key->len; ++i) {
    c[i] = 0;
  }
  for (i = 0; i < key->len; ++i) {
    montMulAdd(key, c, a[i], b);
  }
}

/* In-place public exponentiation. (65537}
 * Input and output big-endian byte array in inout.
 */
static void modpowF4(const IAvbKey* key, uint8_t* inout) {
  uint32_t* a = (uint32_t*)avb_malloc(key->len * sizeof(uint32_t));
  uint32_t* aR = (uint32_t*)avb_malloc(key->len * sizeof(uint32_t));
  uint32_t* aaR = (uint32_t*)avb_malloc(key->len * sizeof(uint32_t));
  if (a == NULL || aR == NULL || aaR == NULL) {
    goto out;
  }

  uint32_t* aaa = aaR; /* Re-use location. */
  int i;

  /* Convert from big endian byte array to little endian word array. */
  for (i = 0; i < (int)key->len; ++i) {
    uint32_t tmp = (inout[((key->len - 1 - i) * 4) + 0] << 24) |
                   (inout[((key->len - 1 - i) * 4) + 1] << 16) |
                   (inout[((key->len - 1 - i) * 4) + 2] << 8) |
                   (inout[((key->len - 1 - i) * 4) + 3] << 0);
    a[i] = tmp;
  }

  montMul(key, aR, a, key->rr); /* aR = a * RR / R mod M   */
  for (i = 0; i < 16; i += 2) {
    montMul(key, aaR, aR, aR);  /* aaR = aR * aR / R mod M */
    montMul(key, aR, aaR, aaR); /* aR = aaR * aaR / R mod M */
  }
  montMul(key, aaa, aR, a); /* aaa = aR * a / R mod M */

  /* Make sure aaa < mod; aaa is at most 1x mod too large. */
  if (geM(key, aaa)) {
    subM(key, aaa);
  }

  /* Convert to bigendian byte array */
  for (i = (int)key->len - 1; i >= 0; --i) {
    uint32_t tmp = aaa[i];
    *inout++ = (uint8_t)(tmp >> 24);
    *inout++ = (uint8_t)(tmp >> 16);
    *inout++ = (uint8_t)(tmp >> 8);
    *inout++ = (uint8_t)(tmp >> 0);
  }

out:
  if (a != NULL) {
    avb_free(a);
  }
  if (aR != NULL) {
    avb_free(aR);
  }
  if (aaR != NULL) {
    avb_free(aaR);
  }
}
#endif
/* Verify a RSA PKCS1.5 signature against an expected hash.
 * Returns false on failure, true on success.
 */
bool avb_rsa_verify(const uint8_t* key,
                    size_t key_num_bytes,
                    const uint8_t* sig,
                    size_t sig_num_bytes,
                    const uint8_t* hash,
                    size_t hash_num_bytes,
                    const uint8_t* padding,
                    size_t padding_num_bytes) {
#if defined(CONFIG_AVB_HW_ENGINE_ENABLE)
  IAvbKey* parsed_key = NULL;
  uint8_t *nk = NULL;
  uint8_t *n = NULL;
  uint8_t *e = NULL;
  int i;
  bool success = false;
  uint32_t key_bytes = 0;
  sc_rsa_t  rsa;
  sc_rsa_context_t rsa_ctx;

  if (key == NULL || sig == NULL || hash == NULL || padding == NULL) {
    avb_error("Invalid input.\n");
    goto out;
  }

  parsed_key = iavb_parse_key_data(key, key_num_bytes);
  if (parsed_key == NULL) {
    avb_error("Error parsing key.\n");
    goto out;
  }
  
  if (padding_num_bytes != sig_num_bytes - hash_num_bytes) {
    avb_error("Padding length does not match hash and signature lengths.\n");
    goto out;
  }

  key_bytes = parsed_key->len * sizeof(uint32_t);
  /* Currently, we only support RSA key 2048bits and SHA256 */
  if ((key_bytes * 8 != 2048) || (hash_num_bytes * 8 != 256)) {
    avb_error("Error unsupported keybits length.\n");
    goto out;
  }

  nk = (uint8_t *)parsed_key->n;
  n = avb_malloc(key_bytes);
  if (n == NULL) {
    avb_error("Error malloc n.\n");
    goto out;
  }
  /* Reverse modular little endian */
  for (i = 0; i < key_bytes; i++) {
    n[i] = nk[key_bytes - i - 1];
  }

  e = avb_malloc(key_bytes);
  if (e == NULL) {
    avb_error("Error malloc e.\n");
    goto out;
  }
  memset(e, 0, key_bytes);
  /* public exponentiation. (65537} */
  e[key_bytes-1] = 0x01; e[key_bytes-2] = 0x00; e[key_bytes-3] = 0x01; e[key_bytes-4] = 0x00;

  hw_crypto_accel_init();
  sc_rsa_init(&rsa, 0, SC_RSA_KEY_BITS_2048);

  rsa_ctx.padding_type = SC_RSA_PADDING_MODE_PKCS1;
  rsa_ctx.n = n;
  rsa_ctx.e = e;
  rsa_ctx.hash_type = SC_RSA_HASH_TYPE_SHA256;
  rsa_ctx.is_crt = SC_RSA_CRT_DISABLE;
  rsa_ctx.is_hash = SC_RSA_HASH_DISABLE;

  success = sc_rsa_verify(&rsa, &rsa_ctx, (void *)hash, hash_num_bytes, (void *)sig, sig_num_bytes, SC_RSA_HASH_TYPE_SHA256);
  sc_rsa_uninit(&rsa);

out:
  if (parsed_key != NULL) {
    iavb_free_parsed_key(parsed_key);
  }
  if (e != NULL) {
    avb_free(e);
  }

  return success;
#else
  uint8_t* buf = NULL;
  IAvbKey* parsed_key = NULL;
  bool success = false;

  if (key == NULL || sig == NULL || hash == NULL || padding == NULL) {
    avb_error("Invalid input.\n");
    goto out;
  }

  parsed_key = iavb_parse_key_data(key, key_num_bytes);
  if (parsed_key == NULL) {
    avb_error("Error parsing key.\n");
    goto out;
  }

  if (sig_num_bytes != (parsed_key->len * sizeof(uint32_t))) {
    avb_error("Signature length does not match key length.\n");
    goto out;
  }

  if (padding_num_bytes != sig_num_bytes - hash_num_bytes) {
    avb_error("Padding length does not match hash and signature lengths.\n");
    goto out;
  }

  buf = (uint8_t*)avb_malloc(sig_num_bytes);
  if (buf == NULL) {
    avb_error("Error allocating memory.\n");
    goto out;
  }
  avb_memcpy(buf, sig, sig_num_bytes);

  modpowF4(parsed_key, buf);

  /* Check padding bytes.
   *
   * Even though there are probably no timing issues here, we use
   * avb_safe_memcmp() just to be on the safe side.
   */
  if (avb_safe_memcmp(buf, padding, padding_num_bytes)) {
    avb_error("Padding check failed.\n");
    goto out;
  }

  /* Check hash. */
  if (avb_safe_memcmp(buf + padding_num_bytes, hash, hash_num_bytes)) {
    avb_error("Hash check failed.\n");
    goto out;
  }

  success = true;

out:
  if (parsed_key != NULL) {
    iavb_free_parsed_key(parsed_key);
  }
  if (buf != NULL) {
    avb_free(buf);
  }
  return success;
#endif
}
