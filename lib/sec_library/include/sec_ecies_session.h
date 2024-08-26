#ifndef __SEC_ECIES_SESSION_H__
#define __SEC_ECIES_SESSION_H__

#ifdef _WIN32
#include "win_ctypes.h"
#else
#include <linux/types.h>
#endif

#include <stdint.h>

#define ECIES_SDATA_MAX_LEN (1024*1024*16)
#define ECIES_SDATA_RESPONSE_LEN 256
#define ECIES_INIT_COMMAND_LEN   256
#define ECIES_INIT_RESPONSE_LEN  256
#define ECIES_CLOSE_COMMAND_LEN  32
#define ECIES_CLOSE_RESPONSE_LEN 32
#define ECIES_FIXED_COMMAND_LEN  16
#define ECIES_KEY_LEN 128
#define ECIES_TAG_LEN 16
#define ECIES_TAG_LEN 16
#define ECIES_DID_LEN 16

#define ECIES_SEQ_DEFAULT  0x8000E000
#define ECIES_TIMEOUT_TIME 5 /* seconds */

#define ECIES_CLA_INITIALIZE_UPDATE 0x10
#define ECIES_CLA_INITIALIZE_UPDATE_RESPONSE 0x20
#define ECIES_CLA_SDATA_SEND 0x30
#define ECIES_CLA_SDATA_SEND_RESPONSE 0x40
#define ECIES_CLA_SESSION_CLOSE 0x50
#define ECIES_CLA_SESSION_CLOSE_RESPONSE 0x60

#define ECIES_RESPONSE_OK 0x90
#define ECIES_RESPONSE_SESSION_OPENED_ERROR 0x91
#define ECIES_RESPONSE_GEN_KEY_PAIR_ERROR 0x92
#define ECIES_RESPONSE_GEN_SHARE_KEY_ERROR 0x93
#define ECIES_RESPONSE_GEN_AUTH_DECRYPT_ERROR 0x94
#define ECIES_RESPONSE_SESSION_MEM_ERROR 0x95
#define ECIES_RESPONSE_SESSION_SEQ_ERROR 0x96
#define ECIES_RESPONSE_SESSION_SID_ERROR 0x97

#define ECIES_CLA_LEN 0x01
#define ECIES_INS_LEN 0x01
#define ECIES_P0_LEN  0x04
#define ECIES_P1_LEN  0x04
#define ECIES_P2_LEN  0x01
#define ECIES_LC_LEN  0x04
#define ECIES_LE_LEN  0x01

#define ECIES_CLA_OFFSET   0x00
#define ECIES_INS_OFFSET   (ECIES_CLA_OFFSET + ECIES_CLA_LEN)
#define ECIES_P0_OFFSET    (ECIES_INS_OFFSET + ECIES_INS_LEN)
#define ECIES_P1_OFFSET    (ECIES_P0_OFFSET + ECIES_P0_LEN)
#define ECIES_P2_OFFSET    (ECIES_P1_OFFSET + ECIES_P1_LEN)
#define ECIES_LC_OFFSET    (ECIES_P2_OFFSET + ECIES_P2_LEN)
#define ECIES_DATA_OFFSET  (ECIES_LC_OFFSET + ECIES_LC_LEN)

enum {
    ECIES_SS_CLOSE = 0,
    ECIES_SS_OPEN,
};

typedef struct _ecies_dev {
    void *context;
    void *transmit;
    uint8_t DID[ECIES_DID_LEN];
    uint8_t DIDLen;
} ecies_dev;

typedef struct _ecies_session_t {
    ecies_dev dev;
    uint8_t   schemeType;
    uint8_t   shareKeyLen;
    uint8_t   encKeyLen;
    uint8_t   macKeyLen;
    uint8_t   shareKey[ECIES_KEY_LEN];
    uint8_t   encKey[ECIES_KEY_LEN];
    uint8_t   macKey[ECIES_KEY_LEN];
    uint32_t  sequenceNumber;
    uint32_t  SID;
    uint32_t  timeout;
    uint32_t  isOpen;
} ecies_session_t;

typedef enum {
    EC_PRIME256V1_AES_GCM_256 = 0,
}scheme_type;

/**
  \brief       Ecies host initialization session
  \param[in]   session      session handle to operate
  \param[in]   context      Pointer to the user-defined context
  \param[in]   transmit     Pointer to the user-implemented transfer
  \param[in]   schemeType   Security scheme selection
  \param[in]   DID          Pointer to the device ID buf
  \param[in]   DIDLen       Length of the device ID
  \return      error code \ref int
*/
int hal_ecies_host_init(ecies_session_t *session, void *context, void *transmit, uint8_t schemeType, uint8_t *DID, uint32_t DIDLen);

/**
  \brief       Ecies host session
  \param[in]   session    session handle to operate
  \return      error code \ref int
*/
int hal_ecies_host_session_open(ecies_session_t *session);

/**
  \brief       Ecies host comm session
  \param[in]   session      session handle to operate
  \param[in]   data         Pointer to the data to be encrypted
  \param[in]   data_len     Length of the data
  \return      error code \ref int
*/
int hal_ecies_host_session_comm(ecies_session_t *session, uint8_t *data, uint32_t dataLen);

/**
  \brief       Ecies host close session
  \param[in]   session      session handle to operate
  \return      error code \ref int
*/
int hal_ecies_host_session_close(ecies_session_t *session);

/**
  \brief       Ecies host uninitiated session
  \param[in]   session      session handle to operate
  \return      error code \ref int
*/
int hal_ecies_host_uninit(ecies_session_t *session);

/**
  \brief       Ecies slave initialization session
  \param[in]   session      session handle to operate
  \return      error code \ref int
*/
int hal_ecies_slave_init(ecies_session_t *session);

/**
  \brief       Ecies slave comm session
  \param[in]   session      session handle to operate
  \param[in]   apdu         Pointer to the apdu buf
  \param[in]   apduLen      Length of the apdu buffer
  \param[out]  apduResponse Pointer to the apduResponse buf
  \param[out]  apduResponseLen     Length of the apduResponseLen buffer
  \param[out]  out          Pointer to the out buf to be decrypted
  \param[out]  outLen       Length of the out buffer
  \return      error code \ref int
*/
int hal_ecies_slave_session_comm(ecies_session_t *session, uint8_t *apdu, uint32_t apduLen,
                                    uint8_t *apduResponse, uint32_t *apduResponseLen, uint8_t *out, uint32_t *outLen);

/**
  \brief       Ecies slave uninitiated session
  \param[in]   session      session handle to operate
  \return      error code \ref int
*/
int hal_ecies_slave_uninit(ecies_session_t *session);

/**
  \brief       Ecies host get CLA and errcode
  \param[in]   apduResponse      Pointer to the apduResponse buf
  \param[in]   apduResponseLen   Length of the apduResponseLen buffer
  \param[out]  CLA          CLA one byte len
  \param[out]  errcode      Error code one byte len
  \return      error code \ref int
*/
int hal_ecies_status_get(uint8_t *apduResponse, uint32_t apduResponseLen, uint8_t *CLA, uint8_t *status);

#endif /* __SEC_ECIES_SESSION_H__ */
