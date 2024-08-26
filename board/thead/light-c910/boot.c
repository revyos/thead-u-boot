// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <command.h>
#include <net.h>
#include <asm/io.h>
#include <dm.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <mmc.h>
#include <opensbi.h>
#include <asm/csr.h>
#include <asm/arch-thead/boot_mode.h>
#include "../../../lib/sec_library/include/csi_efuse_api.h"
#include "../../../lib/sec_library/include/sec_crypto_sha.h"
#include "../../../lib/sec_library/include/kdf.h"
#include "../../../lib/sec_library/include/sec_crypto_mac.h"
#include "fastboot.h"

#if CONFIG_IS_ENABLED(LIGHT_SEC_UPGRADE)

/* The macro is used to enable NON-COT boot with non-signed image */
#define LIGHT_NON_COT_BOOT	1

/* The macro is used to enable uboot version in efuse */
#define	LIGHT_UBOOT_VERSION_IN_ENV	1

/* The macro is used to enable secimg version in env */
#define LIGHT_SECIMG_VERSION_IN_ENV 1

/* vimage return value */
#define VIMAGE_UPGRADE_NOT_REQUIRED  1
#define VIMAGE_BREAK_VERSION_RULE_ERROR  2
#define VIMAGE_SIGNATRE_VERIFICATION_FAILED  3

/* The macro is used to enble RPMB ACCESS KEY from KDF */
//#define LIGHT_KDF_RPMB_KEY	1

/* The macro is used to enable secure image version check in boot */
//#define LIGHT_IMG_VERSION_CHECK_IN_BOOT	1

/* the sample rpmb key is only used for testing */
#ifndef LIGHT_KDF_RPMB_KEY
static const unsigned char emmc_rpmb_key_sample[32] = {0x33, 0x22, 0x11, 0x00, 0x77, 0x66, 0x55, 0x44, \
												0xbb, 0xaa, 0x99, 0x88, 0xff, 0xee, 0xdd, 0xcc, \
												0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
												0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#endif
static unsigned int upgrade_image_version = 0;
static char *current_slot = "a";
static char *update_slot = "b";
#define RPMB_EMMC_CID_SIZE 16
#define RPMB_CID_PRV_OFFSET             9
#define RPMB_CID_CRC_OFFSET             15
#ifdef LIGHT_KDF_RPMB_KEY
static int tee_rpmb_key_gen(uint8_t* key, uint32_t * length)
{
	uint32_t data[RPMB_EMMC_CID_SIZE / 4];
	uint8_t huk[32];
	uint32_t huk_len;
	struct mmc *mmc = find_mmc_device(0);
	int i;
	sc_mac_t mac_handle;
	int ret = 0;

	if (!mmc)
		return -1;

	if (!mmc->ext_csd)
		return -1;

	for (i = 0; i < ARRAY_SIZE(mmc->cid); i++)
		data[i] = cpu_to_be32(mmc->cid[i]);
	/*
	 * PRV/CRC would be changed when doing eMMC FFU
	 * The following fields should be masked off when deriving RPMB key
	 *
	 * CID [55: 48]: PRV (Product revision)
	 * CID [07: 01]: CRC (CRC7 checksum)
	 * CID [00]: not used
	 */
	memset((void *)((uint64_t)data + RPMB_CID_PRV_OFFSET), 0, 1);
	memset((void *)((uint64_t)data + RPMB_CID_CRC_OFFSET), 0, 1);

    	/* Step1: Derive HUK from KDF function */
	ret = csi_kdf_gen_hmac_key(huk, &huk_len);
	if (ret) {
		printf("kdf gen hmac key faild[%d]\r\n", ret);
		return -1;
	}

    	/* Step2: Using HUK and data to generate RPMB key */
	ret = sc_mac_init(&mac_handle, 0);
	if (ret) {
		printf("mac init faild[%d]\r\n", ret);
		ret = -1;
		return -1;
	}

	/* LSB 16 bytes are used as key */
	ret = sc_mac_set_key(&mac_handle, huk, 16);
	if (ret) {
		printf("mac set key faild[%d]\r\n", ret);
		ret = -1;
		goto func_exit;
	}

	ret = sc_mac_calc(&mac_handle, SC_SHA_MODE_256, (uint8_t *)&data, sizeof(data), key, length);
	if (ret) {
		printf("mac calc faild[%d]\r\n", ret);
		ret = -1;
		goto func_exit;
	}

func_exit:
	sc_mac_uninit(&mac_handle);

	return ret;

}
#endif

int csi_rpmb_write_access_key(void)
{
#ifdef LIGHT_KDF_RPMB_KEY
    unsigned long *temp_rpmb_key_addr = NULL;
    char runcmd[64] = {0};
    uint8_t blkdata[256] = {0};
    __attribute__((__aligned__(8))) uint8_t kdf_rpmb_key[32];
    uint32_t kdf_rpmb_key_length = 0;
	int ret = 0;
    /* Step1: retrive RPMB key from KDF function */
	ret = tee_rpmb_key_gen(kdf_rpmb_key, &kdf_rpmb_key_length);
	if (ret != 0) {
		return -1;
	}
    /* Make sure rpmb key length must be 32*/
    if (kdf_rpmb_key_length != 32) {
        return -1;
    }

	temp_rpmb_key_addr = (unsigned long *)kdf_rpmb_key;

    /* Step2: check whether RPMB key is available */
    sprintf(runcmd, "mmc rpmb read 0x%lx 0 1 0x%lx", (unsigned long)blkdata, (unsigned long)temp_rpmb_key_addr);
	ret = run_command(runcmd, 0);
    if (ret == CMD_RET_SUCCESS) {
        return -1;
    }

    /* Step3: Write RPMB key at once */
    sprintf(runcmd, "mmc rpmb key 0x%lx", (unsigned long)temp_rpmb_key_addr);
	ret = run_command(runcmd, 0);
    if (ret != CMD_RET_SUCCESS) {
        return -1;
    }
    return 0;
#else
    return 1;
#endif
}

int csi_tf_get_image_version(unsigned int *ver)
{
	int ret = 0;
#if !LIGHT_SECIMG_VERSION_IN_ENV
	char runcmd[64] = {0};
	unsigned char blkdata[256];
	/* tf version reside in RPMB block#0, offset#16*/
	sprintf(runcmd, "mmc rpmb read 0x%lx 0 1", (unsigned long)blkdata);
	ret = run_command(runcmd, 0);
	if (ret == 0) {
		*ver = (blkdata[16] << 8) + blkdata[17];
	}
#else
	*ver = env_get_hex("tf_version", 0);
#endif
	return ret;
}

int csi_tf_set_image_version(unsigned int ver)
{
#if !LIGHT_SECIMG_VERSION_IN_ENV
	char runcmd[64] = {0};
	unsigned char blkdata[256];
	unsigned long *temp_rpmb_key_addr = NULL;

	/* tf version reside in RPMB block#0, offset#16*/
	sprintf(runcmd, "mmc rpmb read 0x%lx 0 1", (unsigned long)blkdata);
	run_command(runcmd, 0);
	blkdata[16] = (ver & 0xFF00) >> 8;
	blkdata[17] = ver & 0xFF;

	/* tf version reside in RPMB block#0, offset#16*/
#ifndef LIGHT_KDF_RPMB_KEY
	temp_rpmb_key_addr = (unsigned long *)emmc_rpmb_key_sample;
#else
	uint8_t kdf_rpmb_key[32];
	uint32_t kdf_rpmb_key_length = 0;
	int ret = 0;
	ret = csi_kdf_gen_hmac_key(kdf_rpmb_key, &kdf_rpmb_key_length);
	if (ret != 0) {
		return -1;
	}
	temp_rpmb_key_addr = (unsigned long *)kdf_rpmb_key;
#endif

	sprintf(runcmd, "mmc rpmb write 0x%lx 0 1 0x%lx", (unsigned long)blkdata, (unsigned long)temp_rpmb_key_addr);
	run_command(runcmd, 0);
#else
	env_set_hex("tf_version", ver);
#endif
	return 0;
}

int csi_tf_set_upgrade_version(void)
{
	return csi_tf_set_image_version(upgrade_image_version);
}

int csi_tee_get_image_version(unsigned int *ver)
{
	int ret = 0;
#if !LIGHT_SECIMG_VERSION_IN_ENV
	char runcmd[64] = {0};
	unsigned char blkdata[256];
	/* tf version reside in RPMB block#0, offset#0*/
	sprintf(runcmd, "mmc rpmb read 0x%lx 0 1", (unsigned long)blkdata);
	ret = run_command(runcmd, 0);
	if (ret == 0) {
		*ver = (blkdata[0] << 8) + blkdata[1];
	}
#else
	*ver = env_get_hex("tee_version", 0);
#endif
	return ret;
}

int csi_kernel_get_image_version(unsigned int *ver)
{
	char runcmd[64] = {0};
	unsigned char blkdata[256];

	/* kernel version reside in RPMB block#0, offset#32*/
	sprintf(runcmd, "mmc rpmb read 0x%lx 0 1", (unsigned long)blkdata);
	run_command(runcmd, 0);
	*ver = (blkdata[32] << 8) + blkdata[33];

	return 0;
}

int csi_tee_set_image_version(unsigned int ver)
{
#if !LIGHT_SECIMG_VERSION_IN_ENV
	char runcmd[64] = {0};
	unsigned char blkdata[256];
	unsigned long *temp_rpmb_key_addr = NULL;

	/* tf version reside in RPMB block#0, offset#0*/
	sprintf(runcmd, "mmc rpmb read 0x%lx 0 1", (unsigned long)blkdata);
	run_command(runcmd, 0);
	blkdata[0] = (ver & 0xFF00) >> 8;
	blkdata[1] = ver & 0xFF;

	/* tf version reside in RPMB block#0, offset#16*/
#ifndef LIGHT_KDF_RPMB_KEY
	temp_rpmb_key_addr = (unsigned long *)emmc_rpmb_key_sample;
#else
	uint8_t kdf_rpmb_key[32];
	uint32_t kdf_rpmb_key_length = 0;
	int ret = 0;
	ret = csi_kdf_gen_hmac_key(kdf_rpmb_key, &kdf_rpmb_key_length);
	if (ret != 0) {
		return -1;
	}
	temp_rpmb_key_addr = (unsigned long *)kdf_rpmb_key;
#endif
	sprintf(runcmd, "mmc rpmb write 0x%lx 0 1 0x%lx", (unsigned long)blkdata, (unsigned long)temp_rpmb_key_addr);
	run_command(runcmd, 0);
#else
	env_set_hex("tee_version", ver);
#endif
	return 0;
}

int csi_tee_set_upgrade_version(void)
{
	return csi_tee_set_image_version(upgrade_image_version);
}

int csi_sbmeta_get_image_version(unsigned int *ver)
{
	int ret = 0;
#if !LIGHT_SECIMG_VERSION_IN_ENV
	char runcmd[64] = {0};
	unsigned char blkdata[256];
	/* sbmeta version reside in RPMB block#0, offset#48*/
	sprintf(runcmd, "mmc rpmb read 0x%lx 0 1", (unsigned long)blkdata);
	ret = run_command(runcmd, 0);
	if (ret == 0) {
		*ver = (blkdata[48] << 8) + blkdata[49];
	}
#else
	*ver = env_get_hex("sbmeta_version", 0);
#endif
	return ret;
}

int csi_sbmeta_set_image_version(unsigned int ver)
{
#if !LIGHT_SECIMG_VERSION_IN_ENV
	char runcmd[64] = {0};
	unsigned char blkdata[256];
	unsigned long *temp_rpmb_key_addr = NULL;

	/* sbmeta version reside in RPMB block#0, offset#48*/
	sprintf(runcmd, "mmc rpmb read 0x%lx 0 1", (unsigned long)blkdata);
	run_command(runcmd, 0);
	blkdata[48] = (ver & 0xFF00) >> 8;
	blkdata[49] = ver & 0xFF;
	/* sbmeta version reside in RPMB block#0, offset#48*/
#ifndef LIGHT_KDF_RPMB_KEY
	temp_rpmb_key_addr = (unsigned long *)emmc_rpmb_key_sample;
#else
	uint8_t kdf_rpmb_key[32];
	uint32_t kdf_rpmb_key_length = 0;
	int ret = 0;
	ret = csi_kdf_gen_hmac_key(kdf_rpmb_key, &kdf_rpmb_key_length);
	if (ret != 0) {
		return -1;
	}
	temp_rpmb_key_addr = (unsigned long *)kdf_rpmb_key;
#endif
	sprintf(runcmd, "mmc rpmb write 0x%lx 0 1 0x%lx", (unsigned long)blkdata, (unsigned long)temp_rpmb_key_addr);
	run_command(runcmd, 0);
#else
	env_set_hex("sbmeta_version", ver);
#endif
	return 0;
}

int csi_sbmeta_set_upgrade_version(void)
{
	return csi_sbmeta_set_image_version(upgrade_image_version);
}

int csi_uboot_get_image_version(unsigned int *ver)
{
#ifdef	LIGHT_UBOOT_VERSION_IN_ENV
	long long uboot_ver = 0;
	unsigned char ver_x = 1;
	int i;
	// TODO
	// To avoid waste efuse resource, we define uboot_version env parameter to standd for BL1_VERSION in efuse
	uboot_ver = env_get_hex("uboot_version", 0xffffffffffffffff);
	// Add getting uboot version here.
	for (i = 0; i < (UBOOT_MAX_VER-1); i++) {
		if ((uboot_ver >> i) & 0x1) {
			ver_x ++;
		} else {
			break;
		}
	}

	if ( i <= (UBOOT_MAX_VER-1) ) {
		*ver = ver_x << 8;
	} else {
		*ver = 1 << 8;
	}
#else
	unsigned int ver_x = 0;
	int ret = 0;

	ret = csi_efuse_api_init();
	if (ret) {
		printf("efuse api init fail \n");
		return -1;
	}
	ret = csi_efuse_get_bl1_version(&ver_x);
	if (ret) {
        printf("csi_efuse_get_bl1_version fail\n");
		return -1;
	}

    csi_efuse_api_uninit();

	*ver = (ver_x + 1) << 8;

#endif

	return 0;
}

int csi_uboot_set_image_version(unsigned int ver)
{
#ifdef	LIGHT_UBOOT_VERSION_IN_ENV
	//TODO
	unsigned long long uboot_ver = 0;
	unsigned char ver_x = (ver & 0xff00) >> 8;

	uboot_ver = env_get_hex("uboot_version", 0xffffffffffffffff);

	// Add getting uboot version here.
	if (ver_x == 1) {
		printf("This is initial version !");
		return 0;
	}
	uboot_ver |= ((unsigned long long)1 << (ver_x - 2));

	// To avoid waste efuse resource, we define uboot_version env parameter to stand for BL1_VERSION in efuse
	env_set_hex("uboot_version", uboot_ver);
#else
	unsigned int ver_x = 0;
	int ret = 0;

    ver_x = (ver & 0xff00) >> 8;
	if (ver_x == 1) {
		printf("This is initial version !");
		return 0;
	}

	ret = csi_efuse_api_init();
	if (ret) {
		printf("efuse api init fail \n");
		return -1;
	}
	
	ver_x = ver_x - 1;
	ret = csi_efuse_set_bl1_version(ver_x);
	if (ret) {
        printf("csi_efuse_set_bl1_version fail \n");
		return -1;
	}

    csi_efuse_api_uninit();

#endif
	return 0;
}

int csi_uboot_set_upgrade_version(void)
{
	return csi_uboot_set_image_version(upgrade_image_version);
}

int verify_image_version_rule(unsigned int new_ver, unsigned int cur_ver)
{
	unsigned char new_ver_x = 0, new_ver_y = 0;
	unsigned char cur_ver_x = 0, cur_ver_y = 0;

	/* Get secure version X from image version X.Y */
	new_ver_x = (new_ver & 0xFF00) >> 8;
	new_ver_y = new_ver & 0xFF;
	cur_ver_x = (cur_ver & 0xFF00) >> 8;
	cur_ver_y = cur_ver & 0xFF;

	printf("\n\n");
	printf("cur image version: %d.%d\n", cur_ver_x, cur_ver_y);
	printf("new image version: %d.%d\n", new_ver_x, new_ver_y);

	/* According the version rule, the X value must increase by 1 */
	if ((new_ver_x - cur_ver_x) == 0) {
		/* This is unsecure function */
		if ((new_ver_y - cur_ver_y) == 0) {
			printf("New version is equal to Current version, upgrade process terminates \n\n\n");
			return VIMAGE_UPGRADE_NOT_REQUIRED;
		}
		printf("This is unsecure function upgrade, going on uprade anyway\n");
	} else if ((new_ver_x - cur_ver_x) != 1) {
		/* Check the seure version rule */
		printf("The upgrade version(X) breaks against the rule\n\n\n");
		return VIMAGE_BREAK_VERSION_RULE_ERROR;
	}
	printf("check image verison rule pass\n\n\n");

	return 0;
}

int check_image_version_rule(unsigned int new_ver, unsigned int cur_ver)
{
	unsigned char new_ver_x = 0, new_ver_y = 0;
	unsigned char cur_ver_x = 0, cur_ver_y = 0;

	/* Get secure version X from image version X.Y */
	new_ver_x = (new_ver & 0xFF00) >> 8;
	new_ver_y = new_ver & 0xFF;
	cur_ver_x = (cur_ver & 0xFF00) >> 8;
	cur_ver_y = cur_ver & 0xFF;

    (void)new_ver_y;
    (void)cur_ver_y;

	/* Ensure image version must be less than expected version */
	if (new_ver_x < cur_ver_x) {
		return -1;
	}

	return 0;
}

int check_tf_version_in_boot(unsigned long tf_addr)
{
	int ret = 0;
	unsigned int img_version = 0;
	unsigned int expected_img_version = 0;
	
	img_version = get_image_version(tf_addr);
	if (img_version == 0) {
		printf("get tf image version fail\n");
		return -1;
	}

	ret = csi_tf_get_image_version(&expected_img_version);
	if (ret != 0) {
		printf("Get tf expected img version fail\n");
		return -1;
	}

	ret = check_image_version_rule(img_version, expected_img_version);
	if (ret != 0) {
		printf("Image version breaks the rule\n");
		return -1;
	}

	return 0;
}

int check_tee_version_in_boot(unsigned long tee_addr)
{
	int ret = 0;
	unsigned int img_version = 0;
	unsigned int expected_img_version = 0;
	
	img_version = get_image_version(tee_addr);
	if (img_version == 0) {
		printf("get tee image version fail\n");
		return -1;
	}

	ret = csi_tee_get_image_version(&expected_img_version);
	if (ret != 0) {
		printf("Get tee expected img version fail\n");
		return -1;
	}

	ret = check_image_version_rule(img_version, expected_img_version);
	if (ret != 0) {
		printf("Image version breaks the rule\n");
		return -1;
	}

	return 0;
}

int check_sbmeta_version_in_boot(unsigned long sbmeta_addr)
{
	int ret = 0;
	unsigned int img_version = 0;
	unsigned int expected_img_version = 0;

	img_version = get_image_version(sbmeta_addr);
	if (img_version == 0) {
		printf("get sbmeta image version fail\n");
		return -1;
	}

	ret = csi_sbmeta_get_image_version(&expected_img_version);
	if (ret != 0) {
		printf("Get sbmeta expected img version fail\n");
		return -1;
	}

	ret = check_image_version_rule(img_version, expected_img_version);
	if (ret != 0) {
		printf("Image version breaks the rule\n");
		return -1;
	}

	return 0;
}

int light_vimage(int argc, char *const argv[])
{
	int ret = 0;
	unsigned long vimage_addr = 0;
	unsigned int new_img_version = 0;
	unsigned int cur_img_version = 0;
	char imgname[32] = {0};

	if (argc < 3)
		return CMD_RET_USAGE;

	/* Parse input parameters */
	vimage_addr = simple_strtoul(argv[1], NULL, 16);
	strcpy(imgname, argv[2]);

	/* Retrieve desired information from image header */
	new_img_version = get_image_version(vimage_addr);
	if (new_img_version == 0) {
		printf("get new img version fail\n");
		return CMD_RET_FAILURE;
	}
	if (strcmp(imgname, UBOOT_PART_NAME) == 0) {
		new_img_version = (((new_img_version & 0xff )+1) << 8) | ((new_img_version & 0xff00)>>8);
	}
	printf("Get new image version from image header: v%d.%d\n", (new_img_version & 0xff00)>>8, new_img_version & 0xff);

	/* Check image version for ROLLBACK resisance */
	if (strcmp(imgname, TF_PART_NAME) == 0) {
		ret = csi_tf_get_image_version(&cur_img_version);
		if (ret != 0) {
			printf("Get tf img version fail\n");
			return CMD_RET_FAILURE;
		}
#if LIGHT_NON_COT_BOOT
		/* if in non-cot mode, tf and tee will not be signed at first */
		if (image_have_head(vimage_addr) == 0 && ((cur_img_version & 0xFF00) >> 8 == 0)) {
			return VIMAGE_UPGRADE_NOT_REQUIRED;
		}
#endif
	} else if (strcmp(imgname, TEE_PART_NAME) == 0){
		ret = csi_tee_get_image_version(&cur_img_version);
		if (ret != 0) {
			printf("Get tee img version fail\n");
			return CMD_RET_FAILURE;
		}
#if LIGHT_NON_COT_BOOT
		/* if in non-cot mode, tf and tee will not be signed at first */
		if (image_have_head(vimage_addr) == 0 && ((cur_img_version & 0xFF00) >> 8 == 0)) {
			return VIMAGE_UPGRADE_NOT_REQUIRED;
		}
#endif
	} else if (strcmp(imgname, KERNEL_PART_NAME) == 0){
		ret = csi_kernel_get_image_version(&cur_img_version);
		if (ret != 0) {
			printf("Get kernel img version fail\n");
			return CMD_RET_FAILURE;
		}
	} else if (strcmp(imgname, SBMETA_PART_NAME) == 0){
		ret = csi_sbmeta_get_image_version(&cur_img_version);
		if (ret != 0) {
			printf("Get sbmeta img version fail\n");
			return CMD_RET_FAILURE;
		}
	}  else if (strcmp(imgname, UBOOT_PART_NAME) == 0) {
		ret = csi_uboot_get_image_version(&cur_img_version);
		if (ret != 0) {
			printf("Get uboot img version fail\n");
			return CMD_RET_FAILURE;
		}

		// Check uboot maximization version > 64
		if (((new_img_version & 0xFF00) >> 8) > UBOOT_MAX_VER) {
			printf("UBOOT Image version has reached to max-version\n");
			return CMD_RET_FAILURE;
		}

	} else {
		printf("unsupport image file\n");
		return CMD_RET_FAILURE;
	}

	/* Verify image version rule */
	ret = verify_image_version_rule(new_img_version, cur_img_version);
	if (ret != 0) {
                return ret;
	}

	/* Save new image version to allow caller upgrade image version */
	upgrade_image_version = new_img_version;

	/* Initialize secure basis of functions */
	ret = csi_sec_init();
	if (ret != 0) {
		return CMD_RET_FAILURE;
	}

	/* Do image verification for TF and TEE */
	if (strcmp(imgname, TF_PART_NAME) == 0) {
		ret = verify_customer_image(T_TF, vimage_addr);
		if (ret != 0) {
			return VIMAGE_SIGNATRE_VERIFICATION_FAILED;
		}
	} else if (strcmp(imgname, TEE_PART_NAME) == 0) {
		ret = verify_customer_image(T_TEE, vimage_addr);
		if (ret != 0) {
			return VIMAGE_SIGNATRE_VERIFICATION_FAILED;
		}
	} else if (strcmp(imgname, KERNEL_PART_NAME) == 0) {
		ret = verify_customer_image(T_KRLIMG, vimage_addr);
		if (ret != 0) {
			return VIMAGE_SIGNATRE_VERIFICATION_FAILED;
		}
	} else if (strcmp(imgname, UBOOT_PART_NAME) == 0) {
		ret = verify_customer_image(T_UBOOT, vimage_addr);
		if (ret != 0) {
			return VIMAGE_SIGNATRE_VERIFICATION_FAILED;
		}
	} else if (strcmp(imgname, SBMETA_PART_NAME) == 0) {
		ret = verify_customer_image(T_SBMETA, vimage_addr);
		if (ret != 0) {
			return VIMAGE_SIGNATRE_VERIFICATION_FAILED;
		}
	} else {
		printf("Error: unknow image name\n");
		return CMD_RET_FAILURE;
	}

	return 0;
}

int light_secboot(int argc, char * const argv[])
{
	int ret = 0;
	unsigned long tf_addr = LIGHT_TF_FW_ADDR;
	unsigned long tee_addr = LIGHT_TEE_FW_ADDR;
	unsigned int tf_image_size = 0;
	unsigned int tee_image_size = 0;

	printf("\n\n");
	printf("Now, we start to verify all trust firmware before boot kernel !\n");

	/* Enject RPMB KEY directly in startup */
	csi_rpmb_write_access_key();

	/* Initialize secure basis of functions */
	ret = csi_sec_init();
	if (ret != 0) {
		return CMD_RET_FAILURE;
	}

	/* Step1. Check and verify TF image */
	if (image_have_head(LIGHT_TF_FW_TMP_ADDR) == 1) {
#ifdef LIGHT_IMG_VERSION_CHECK_IN_BOOT
		printf("check TF version in boot \n");
		ret = check_tf_version_in_boot(LIGHT_TF_FW_TMP_ADDR);
		if (ret != 0) {
			return CMD_RET_FAILURE;
		}
#endif

		printf("Process TF image verification ...\n");
		ret = verify_customer_image(T_TF, LIGHT_TF_FW_TMP_ADDR);
		if (ret != 0) {
			return CMD_RET_FAILURE;
		}

		tf_image_size = get_image_size(LIGHT_TF_FW_TMP_ADDR);
		printf("TF image size: %d\n", tf_image_size);
		if (tf_image_size  < 0) {
			printf("GET TF image size error\n");
			return CMD_RET_FAILURE;
		}

		memmove((void *)tf_addr, (const void *)(LIGHT_TF_FW_TMP_ADDR + HEADER_SIZE), tf_image_size);
	} else {
		#ifdef LIGHT_NON_COT_BOOT
			run_command("ext4load mmc 0:3 0x0 trust_firmware.bin", 0);
		#else
			return CMD_RET_FAILURE;
		#endif
	}

	/* Step2. Check and verify TEE image */
	if (image_have_head(tee_addr) == 1) {
#ifdef LIGHT_IMG_VERSION_CHECK_IN_BOOT
		printf("check TEE version in boot \n");
		ret = check_tee_version_in_boot(tee_addr);
		if (ret != 0) {
			return CMD_RET_FAILURE;
		}
#endif

		printf("Process TEE image verification ...\n");
		ret = verify_customer_image(T_TEE, tee_addr);
		if (ret != 0) {
			return CMD_RET_FAILURE;
		}

		tee_image_size = get_image_size(tee_addr);
		printf("TEE image size: %d\n", tee_image_size);
		if (tee_image_size  < 0) {
			printf("GET TEE image size error\n");
			return CMD_RET_FAILURE;
		}

		memmove((void *)tee_addr, (const void *)(tee_addr + HEADER_SIZE), tee_image_size);
	} else {
		#ifndef LIGHT_NON_COT_BOOT
			return CMD_RET_FAILURE;
		#endif
	}

    // /* Step3. Check and verify light kernel image */
	// if (image_have_head(kernel_addr) == 1) {
	// 	printf("Process kernel image verification ...\n");
	// 	ret = verify_customer_image(T_KRLIMG, kernel_addr);
	// 	if (ret != 0) {
	// 		return CMD_RET_FAILURE;
	// 	}

	// 	kernel_image_size = get_image_size(kernel_addr);
	// 	printf("Kernel image size: %d\n", kernel_image_size);
	// 	if (kernel_image_size < 0) {
	// 		printf("GET kernel image size error\n");
	// 		return CMD_RET_FAILURE;
	// 	}

	// 	memmove((void *)kernel_addr, (const void *)(kernel_addr + HEADER_SIZE), kernel_image_size);
	// } else {
	// 	#ifndef LIGHT_NON_COT_BOOT
	// 		return CMD_RET_FAILURE;
	// 	#endif
	// }

	return 0;
}

void sec_firmware_version_dump(void)
{
	unsigned int tf_ver = 0;
	unsigned int tee_ver = 0;
	unsigned int uboot_ver = 0;
	unsigned int sbmeta_ver = 0;
	unsigned int tf_ver_env = 0;
	unsigned int tee_ver_env = 0;
	unsigned int sbmeta_ver_env = 0;

	csi_uboot_get_image_version(&uboot_ver);
	csi_tf_get_image_version(&tf_ver);
	csi_tee_get_image_version(&tee_ver);
	csi_sbmeta_get_image_version(&sbmeta_ver);
	/* Keep sync with version in RPMB, the Following version could be leveraged by OTA client */
	tee_ver_env = env_get_hex("tee_version", 0);
	tf_ver_env = env_get_hex("tf_version", 0);
	sbmeta_ver_env = env_get_hex("sbmeta_version", 0);
	if ((tee_ver_env != tee_ver) && (tee_ver != 0)) {
		env_set_hex("tee_version", tee_ver);
		run_command("saveenv", 0);
	}

	if ((tf_ver_env != tf_ver) && (tf_ver != 0)) {
		env_set_hex("tf_version", tf_ver);
		run_command("saveenv", 0);
	}

	if ((sbmeta_ver_env != sbmeta_ver) && (sbmeta_ver != 0)) {
		env_set_hex("sbmeta_version", sbmeta_ver);
		run_command("saveenv", 0);
	}

	printf("\n\n");
	printf("Secure Firmware image version info: \n");
	printf("uboot Firmware v%d.0\n", (uboot_ver & 0xff00) >> 8);
	printf("Trust Firmware v%d.%d\n", (tf_ver & 0xff00) >> 8, tf_ver & 0xff);
	printf("TEE OS v%d.%d\n", (tee_ver & 0xff00) >> 8, tee_ver & 0xff);
	printf("SBMETA v%d.%d\n", (sbmeta_ver & 0xff00) >> 8, sbmeta_ver & 0xff);
	printf("\n\n");
}

struct sec_img_upgrade_entry {
	const char* filename;
	const char* imgtype;
	int (*set_version_func)(void);
	const char *part_str;
};

static struct sec_img_upgrade_entry sec_img_list[] = {
	{"sbmeta.bin", "sbmeta", csi_sbmeta_set_upgrade_version, "sbmeta"},
	{"trust_firmware.bin", "tf", csi_tf_set_upgrade_version, "tee"},
	{"tee.bin", "tee", csi_tee_set_upgrade_version, "tee"},
        {NULL, NULL, NULL, NULL},
};

static struct blk_desc *dev_desc;
static int ab_get_blk(void)
{
        struct disk_partition part_info;
        dev_desc = blk_get_dev("mmc", CONFIG_FASTBOOT_FLASH_MMC_DEV);
	if (dev_desc == NULL) {
		printf("Failed to find MMC device\n");
		return -1;
	}
        return 0;
}

static int ab_get_sec_part(const char *part_str, int update_part)
{
	struct disk_partition part_info;
        char partname[10] = {0};
        int part = 0;

        if (update_part) {
                sprintf(partname, "%s_%s", part_str, update_slot);
        } else {
                sprintf(partname, "%s_%s", part_str, current_slot);
        }

        part = part_get_info_by_name(dev_desc, partname, &part_info);
        if (part < 0) {
		printf("Failed to find MMC device\n");
	}

        return part;
}

static int single_img_upgrade(struct sec_img_upgrade_entry *sec_img_entry)
{
	const unsigned long temp_addr=0x200000;
	char runcmd[80];
	uint8_t * image_buffer = NULL;
	uint8_t * image_malloc_buffer = NULL;
	unsigned int upgrade_file_size = 0;
	const char *filename = NULL;
	int update_part = 0;
        int current_part = 0;
	int ret = 0;
        char *argv[3] = {"vimage", NULL, NULL};

	if (sec_img_entry == NULL) {
		return -1;
	}

	update_part = ab_get_sec_part(sec_img_entry->part_str, 1);
        if (update_part < 0) {
                return -1;
        }

	filename = sec_img_entry->filename;

	/* STEP 1: read upgrade image from storage */
	printf("read upgrade image (%s) from storage \n", filename);
	sprintf(runcmd, "ext4load mmc ${mmcdev}:%x 0x%p %s", update_part, (void *)temp_addr, filename);
	printf("runcmd:%s\n", runcmd);
	ret = run_command(runcmd, 0);
	if (ret != 0) {
		printf("%s upgrade process is terminated due to some reason\n", filename);
		return -1;
	}
	/* Fetch the total file size after read out operation end */
	upgrade_file_size = env_get_hex("filesize", 0);
	printf("upgrade file size: %d\n", upgrade_file_size);

	/*store image to temp buffer as temp_addr may be decrypted*/
	image_malloc_buffer = malloc(upgrade_file_size);
	if (image_malloc_buffer == NULL) {
		image_buffer = (uint8_t*)temp_addr + upgrade_file_size;
	} else {
		image_buffer = image_malloc_buffer;
	}
	memcpy(image_buffer, (void*)temp_addr, upgrade_file_size);

	/* STEP 2: verify secure image */
        sprintf(runcmd, "0x%lx", temp_addr);
        argv[1] = runcmd;
        argv[2] = sec_img_entry->imgtype;
        ret = light_vimage(3, argv);
        if (ret == VIMAGE_UPGRADE_NOT_REQUIRED) {
                printf("%s Image may not need upgrade\n", sec_img_entry->imgtype);
                return 0;
        } else if (ret != 0) {
                return -1;
        }

	/* STEP 3: update partition image in another slot */
        current_part = ab_get_sec_part(sec_img_entry->part_str, 0);
        if (current_part < 0) {
                return -1;
        }
	printf("write upgrade image (%s) into another slot \n", filename);
	sprintf(runcmd, "ext4write mmc 0:%x 0x%p /%s 0x%x", current_part, (void *)image_buffer, filename, upgrade_file_size);
	printf("runcmd:%s\n", runcmd);
	ret = run_command(runcmd, 0);
	if (ret != 0) {
		printf("%s upgrade process is terminated due to some reason\n", filename);
		return -1;
	}

	/* STEP 4: update secure image version */
        sec_img_entry->set_version_func();

	printf("\n\n%s image ugprade process is successful\n\n", filename);
	return 0;
}

static int sec_img_upgrade(void)
{
        int ret = 0;
        struct sec_img_upgrade_entry *sec_img_entry = sec_img_list;

        ab_get_blk();

        while (sec_img_entry->filename != NULL) {
                ret = single_img_upgrade(sec_img_entry);
                if (ret) {
                        printf("Fail to upgrade image\n");
                        return -1;
                }

                sec_img_entry++;
        }
	return 0;
}

extern int hibernate_image_cleaned_flag;
extern void clean_hibernate_image_header(char *response);
static char *response[FASTBOOT_RESPONSE_LEN] = {0};
void sec_upgrade_thread(void)
{
        const unsigned long temp_addr=0x200000;
        char runcmd[80];
        int ret = 0;
        unsigned int sec_upgrade_flag = 0;
        unsigned int upgrade_file_size = 0;

        sec_upgrade_flag = env_get_hex("sec_upgrade_mode", 0);
        current_slot = env_get("slot_suffix");
        update_slot = strcmp(current_slot, "a") == 0 ? "b" : "a";

        if (sec_upgrade_flag == 0) {
		return;
        }

	clean_hibernate_image_header(response);
        printf("bootstrap: sec_upgrade_flag: %x\n", sec_upgrade_flag);
        if (sec_upgrade_flag == UBOOT_SEC_UPGRADE_FLAG) {
                unsigned int block_cnt;
                struct blk_desc *dev_desc;
                const unsigned long uboot_temp_addr=0x80000000;
                #define BLOCK_SIZE 512
                #define PUBKEY_HEADER_SIZE	0x1000

                /* STEP 1: read upgrade image (u-boot-with-spl.bin) from stash partition */
                printf("read upgrade image (u-boot-with-spl.bin) from stash partition \n");
                sprintf(runcmd, "ext4load mmc 0:4 0x%p u-boot-with-spl.bin", (void *)temp_addr);
                printf("runcmd:%s\n", runcmd);
                ret = run_command(runcmd, 0);
                if (ret != 0) {
                        printf("UBOOT Upgrade process is terminated due to some reason\n");
                        goto _upgrade_uboot_exit;
                }

                /* Fetch the total file size after read out operation end */
                upgrade_file_size = env_get_hex("filesize", 0);
                printf("uboot upgrade file size: %d\n", upgrade_file_size);

                /* STEP 2: verify its authentiticy here */
                memmove((void *)uboot_temp_addr, (const void *)temp_addr, upgrade_file_size);
                sprintf(runcmd, "vimage 0x%p uboot", (void *)(uboot_temp_addr+PUBKEY_HEADER_SIZE));
                printf("runcmd:%s\n", runcmd);
                ret = run_command(runcmd, 0);
                if (ret != 0) {
                        printf("UBOOT Image verification fail and upgrade process terminates\n");
                        goto _upgrade_uboot_exit;
                }

                /* STEP 3: update uboot partition */
                printf("write upgrade image (u-boot-with-spl.bin) into uboot partition \n");
                dev_desc = blk_get_dev("mmc", CONFIG_FASTBOOT_FLASH_MMC_DEV);
                if (!dev_desc || dev_desc->type == DEV_TYPE_UNKNOWN) {
                        printf("Invalid mmc device\n");
                        goto _upgrade_uboot_exit;
                }
                block_cnt = upgrade_file_size / BLOCK_SIZE;
                if (upgrade_file_size % BLOCK_SIZE) {
                        block_cnt = block_cnt +1;
                }

                run_command("mmc partconf 0 1 0 1", 0);
                sprintf(runcmd, "mmc write 0x%p 0 %x", (void *)temp_addr, block_cnt);
                printf("runcmd:%s\n", runcmd);
                ret = run_command(runcmd, 0);
                run_command("mmc partconf 0 1 0 0", 0);
                if (ret != 0) {
                        printf("UBOOT upgrade process is terminated due to some reason\n");
                        goto _upgrade_uboot_exit;
                }

                /* STEP 4: update tee version */
                ret = csi_uboot_set_upgrade_version();
                if (ret != 0) {
                        printf("Set uboot upgrade version fail\n");
                        goto _upgrade_uboot_exit;
                }

                printf("\n\nUBOOT image ugprade process is successful\n\n");
_upgrade_uboot_exit:
                /* set secure upgrade flag to 0 that indicate upgrade over */
                run_command("env set sec_upgrade_mode 0", 0);
                run_command("saveenv", 0);
                run_command("reset", 0);
        } else if ((sec_upgrade_flag >> 16) == SEC_IMG_UPGRADE_FLAG) {
                ret = sec_img_upgrade();
                if (ret) {
                        printf("secure image upgrade failed\n");
                        /* if failed, clear upgrade flag, terminate upgradation */
                        sec_upgrade_flag = 0;
                } else  {
                        /* if succeed, clear secure flag */
                        sec_upgrade_flag = sec_upgrade_flag & 0x0000FFFF;
                        /* if boot need not update, switch current slot to update slot */
                        if ((sec_upgrade_flag & 0xFF00) != BOOT_IMG_UPGRADE_FLAG) {
                                sprintf(runcmd, "env set slot_suffix %s", update_slot);
                                run_command(runcmd, 0);
                        }
                }
                /* set upgrade flag */
                sprintf(runcmd, "env set sec_upgrade_mode %x", sec_upgrade_flag);
                run_command(runcmd, 0);

                run_command("saveenv", 0);
                run_command("reset", 0);
        } else if (((sec_upgrade_flag & 0xFF00) != BOOT_IMG_UPGRADE_FLAG) &&
			((sec_upgrade_flag & 0xFF) != ROOT_IMG_UPGRADE_FLAG)) {
                printf("Unknown bootstrap, Force sysem reboot\n");
                run_command("env set sec_upgrade_mode 0", 0);
                run_command("saveenv", 0);
                run_command("reset", 0);
        }
}

void nonsec_upgrade_thread(void)
{
	unsigned int sec_upgrade_flag;
	unsigned long retries;
        char runcmd[32] = {0};

	sec_upgrade_flag = env_get_hex("sec_upgrade_mode", 0);
	retries = env_get_ulong("retries", 10, 5);
        if (sec_upgrade_flag == 0) {
		if (retries < 5 && retries > 0) {
			printf("boot upgradation is successful!\n");
                	run_command("env set retries 5", 0);
                	run_command("env save", 0);
		}
                return;
        }

	/* when sec_upgrade_mode != 0 and the first time try to boot. switch current slot to update slot*/
	if (retries == 5) {
		printf("upgrade images are in slot %s...\n", update_slot);
		sprintf(runcmd, "env set slot_suffix %s", update_slot);
		run_command(runcmd, 0);
	}
	/* if ROOT image need upgrade, clear flag */
	if ((sec_upgrade_flag & 0xFF) == ROOT_IMG_UPGRADE_FLAG) {
		printf("in root image upgrade process...\n");
		sec_upgrade_flag = sec_upgrade_flag & 0xFF00;
		sprintf(runcmd, "env set sec_upgrade_mode %X", sec_upgrade_flag);
		run_command(runcmd, 0);
	}

	/* if boot image need upgrade, decrement retries */
	if ((sec_upgrade_flag & 0xFF00) == BOOT_IMG_UPGRADE_FLAG) {
		printf("in boot image upgrade process...\n");
		retries--;
                printf("remaining retry times: %ld\n", retries);
		if (retries == 0 || retries > 5) {
			/*
                         * upgrade failed. Now updated images are in current slot
                         * switch to original slot
                         */
                        printf("boot images upgrade failed. switch slot to %s...\n", update_slot);
                        sprintf(runcmd, "env set slot_suffix %s", update_slot);
                        run_command(runcmd, 0);
			run_command("env set sec_upgrade_mode 0", 0);
			retries = 5;
		}
                sprintf(runcmd, "env set retries %ld", retries);
                run_command(runcmd, 0);
                run_command("env save", 0);
	}
}
#endif
