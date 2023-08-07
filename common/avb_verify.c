/*
 * (C) Copyright 2018, Linaro Limited
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <avb_verify.h>
#include <blk.h>
#include <cpu_func.h>
#include <fastboot.h>
#include <image.h>
#include <malloc.h>
#include <part.h>
#include <tee.h>
#include <tee/optee_ta_avb.h>

#ifdef CONFIG_AVB_ROLLBACK_ENABLE
extern int sec_write_rollback_index(size_t rollback_index_slot, uint64_t rollback_index);
extern int sec_read_rollback_index(size_t rollback_index_slot, uint64_t *out_rollback_index);
#endif

#if defined (CONFIG_AVB_USE_OEM_KEY)
static const unsigned char avb_root_oem_pub[520] = {
	0x00,0x00,0x08,0x00,0x11,0x70,0xEA,0xC9,0xC2,0xAD,0x66,0x2A,0x57,0x2A,0x89,0x68,
	0x8B,0x40,0x33,0xF2,0xEA,0x22,0xD7,0x3E,0x31,0x5F,0x9D,0xB8,0xD1,0x16,0x5C,0x22,
	0xC3,0xFE,0xE6,0x35,0x3F,0x96,0x6D,0xD8,0x1A,0x11,0xE9,0x53,0x90,0x88,0xA9,0xCE,
	0xA7,0x33,0xB2,0x16,0x60,0x18,0xBE,0x23,0xCC,0x5C,0xAB,0x29,0x0E,0x7B,0x35,0x16,
	0xB0,0x59,0x3A,0x2F,0x62,0xF1,0x9B,0x39,0x0A,0x21,0x00,0xFE,0x75,0xEB,0x00,0xDF,
	0x17,0xAF,0x44,0x11,0x42,0x4E,0x4C,0x7C,0xA6,0xDC,0xC5,0xAD,0xB3,0x7C,0xC3,0xB1,
	0x52,0xAD,0x0C,0xEF,0x73,0x69,0x7E,0xFC,0xF9,0x12,0xA7,0x5C,0x60,0x47,0xEF,0x8F,
	0xC7,0x9D,0xD9,0x62,0xF5,0x0E,0x62,0xBB,0x3E,0x80,0x23,0xFA,0x19,0x4C,0x0A,0xD6,
	0xE0,0xA7,0x0E,0x65,0xEA,0xD5,0xB8,0xA9,0xF2,0xA3,0xDA,0x18,0xBE,0x5D,0x4B,0x37,
	0x91,0xBA,0xDB,0x0D,0x50,0x7E,0xEE,0x52,0xDF,0x90,0xE6,0xFC,0x8F,0xB8,0x24,0x2A,
	0x2B,0xBE,0xA6,0xC9,0x5B,0x89,0x3E,0xE8,0x24,0xBD,0x6A,0x03,0x31,0x6C,0xFC,0x4A,
	0xBA,0x6B,0xEE,0x08,0xAE,0x33,0x6E,0xC0,0x64,0x87,0xC1,0x35,0x65,0x42,0x34,0xE5,
	0xF8,0x3B,0x82,0x36,0xE9,0xFA,0x23,0xD7,0x12,0xC5,0x7B,0x27,0x16,0xB0,0xC7,0x40,
	0x5A,0xA4,0x8A,0x56,0xA4,0x54,0x0F,0xD9,0xA7,0x25,0x25,0xE3,0x7F,0x72,0x6E,0x4B,
	0x63,0x1B,0x05,0xFE,0x4E,0x1F,0x1D,0x05,0xDD,0x91,0xA7,0xBF,0xA4,0x90,0xDA,0x7D,
	0x0F,0xB6,0xFC,0x5D,0x8E,0xFB,0xE2,0xF7,0x5F,0x76,0xFA,0xD5,0x12,0xEC,0x87,0xD6,
	0x07,0xA0,0xAC,0xB6,0xE6,0xBA,0xB0,0x87,0xBA,0x51,0xD7,0x6C,0x19,0xC0,0x2C,0xA5,
	0x2C,0x08,0x52,0x2A,0x63,0x18,0x10,0x94,0xEA,0x5C,0x13,0xBF,0x42,0x8E,0x12,0xB6,
	0x7D,0x34,0xD9,0x1C,0x42,0xBC,0xCE,0x44,0x8A,0x13,0x5B,0x93,0x6F,0x34,0x41,0xA1,
	0xA3,0xD3,0x2E,0xF4,0xD3,0x28,0xAD,0x8F,0x8B,0x89,0x9D,0x42,0x43,0xD6,0xBF,0xDE,
	0x9F,0xBD,0x32,0x06,0xE3,0x67,0xD3,0x14,0x2F,0x4C,0xE7,0x6B,0x9A,0xD9,0x04,0xFA,
	0x4B,0x46,0x88,0xE7,0x04,0xAA,0x56,0xFF,0xBF,0x89,0x1D,0xFD,0x32,0xDF,0x47,0xC0,
	0x34,0x0D,0x25,0x8C,0xF6,0xE1,0x5E,0xA9,0x3C,0x30,0x3A,0x53,0x0C,0xDB,0xAD,0x8E,
	0x01,0xCB,0x46,0xE0,0xF5,0x97,0x2D,0xB8,0xF8,0x29,0xC3,0x19,0x4C,0x05,0x8B,0x74,
	0xE0,0xA6,0x3B,0x3C,0x96,0x4F,0x91,0x74,0x62,0xAA,0x50,0x0F,0x11,0x30,0x59,0xAE,
	0x7A,0x80,0xD3,0xAC,0xB3,0xDB,0x24,0x3A,0x79,0xD4,0xDB,0x79,0x10,0x63,0x27,0xD0,
	0x6B,0xF9,0xA3,0xF4,0x27,0x24,0x89,0x0C,0xAC,0x31,0x15,0x08,0x10,0x59,0x08,0x2D,
	0x00,0x3D,0xD8,0xD6,0x3B,0x91,0xC8,0x55,0xCF,0x28,0x3A,0xFB,0xD7,0xF7,0xF7,0x9D,
	0x41,0xBD,0x3E,0xD1,0x77,0xA3,0xF6,0xFA,0x33,0x05,0x5A,0x36,0xCE,0xB9,0x02,0x12,
	0x10,0xEB,0xCA,0xA7,0x3C,0xC8,0x5D,0xCD,0x33,0xD9,0xFA,0x16,0xD4,0x52,0x12,0xB6,
	0x35,0xD5,0x84,0x53,0xC4,0x21,0xDC,0x72,0x2F,0xF9,0x1E,0x59,0x0A,0xCD,0x7D,0x89,
	0xD4,0xCF,0x8E,0x2E,0x09,0x36,0xF5,0x12,0x35,0x43,0x64,0x6C,0xD1,0x70,0xBF,0x67,
	0x3A,0x54,0x72,0x84,0xF3,0xF1,0x4A,0x6A
};
#else
static const unsigned char avb_root_pub[1032] = {
	0x0, 0x0, 0x10, 0x0, 0x55, 0xd9, 0x4, 0xad, 0xd8, 0x4,
	0xaf, 0xe3, 0xd3, 0x84, 0x6c, 0x7e, 0xd, 0x89, 0x3d, 0xc2,
	0x8c, 0xd3, 0x12, 0x55, 0xe9, 0x62, 0xc9, 0xf1, 0xf, 0x5e,
	0xcc, 0x16, 0x72, 0xab, 0x44, 0x7c, 0x2c, 0x65, 0x4a, 0x94,
	0xb5, 0x16, 0x2b, 0x0, 0xbb, 0x6, 0xef, 0x13, 0x7, 0x53,
	0x4c, 0xf9, 0x64, 0xb9, 0x28, 0x7a, 0x1b, 0x84, 0x98, 0x88,
	0xd8, 0x67, 0xa4, 0x23, 0xf9, 0xa7, 0x4b, 0xdc, 0x4a, 0xf,
	0xf7, 0x3a, 0x18, 0xae, 0x54, 0xa8, 0x15, 0xfe, 0xb0, 0xad,
	0xac, 0x35, 0xda, 0x3b, 0xad, 0x27, 0xbc, 0xaf, 0xe8, 0xd3,
	0x2f, 0x37, 0x34, 0xd6, 0x51, 0x2b, 0x6c, 0x5a, 0x27, 0xd7,
	0x96, 0x6, 0xaf, 0x6b, 0xb8, 0x80, 0xca, 0xfa, 0x30, 0xb4,
	0xb1, 0x85, 0xb3, 0x4d, 0xaa, 0xaa, 0xc3, 0x16, 0x34, 0x1a,
	0xb8, 0xe7, 0xc7, 0xfa, 0xf9, 0x9, 0x77, 0xab, 0x97, 0x93,
	0xeb, 0x44, 0xae, 0xcf, 0x20, 0xbc, 0xf0, 0x80, 0x11, 0xdb,
	0x23, 0xc, 0x47, 0x71, 0xb9, 0x6d, 0xd6, 0x7b, 0x60, 0x47,
	0x87, 0x16, 0x56, 0x93, 0xb7, 0xc2, 0x2a, 0x9a, 0xb0, 0x4c,
	0x1, 0xc, 0x30, 0xd8, 0x93, 0x87, 0xf0, 0xed, 0x6e, 0x8b,
	0xbe, 0x30, 0x5b, 0xf6, 0xa6, 0xaf, 0xdd, 0x80, 0x7c, 0x45,
	0x5e, 0x8f, 0x91, 0x93, 0x5e, 0x44, 0xfe, 0xb8, 0x82, 0x7,
	0xee, 0x79, 0xca, 0xbf, 0x31, 0x73, 0x62, 0x58, 0xe3, 0xcd,
	0xc4, 0xbc, 0xc2, 0x11, 0x1d, 0xa1, 0x4a, 0xbf, 0xfe, 0x27,
	0x7d, 0xa1, 0xf6, 0x35, 0xa3, 0x5e, 0xca, 0xdc, 0x57, 0x2f,
	0x3e, 0xf0, 0xc9, 0x5d, 0x86, 0x6a, 0xf8, 0xaf, 0x66, 0xa7,
	0xed, 0xcd, 0xb8, 0xed, 0xa1, 0x5f, 0xba, 0x9b, 0x85, 0x1a,
	0xd5, 0x9, 0xae, 0x94, 0x4e, 0x3b, 0xcf, 0xcb, 0x5c, 0xc9,
	0x79, 0x80, 0xf7, 0xcc, 0xa6, 0x4a, 0xa8, 0x6a, 0xd8, 0xd3,
	0x31, 0x11, 0xf9, 0xf6, 0x2, 0x63, 0x2a, 0x1a, 0x2d, 0xd1,
	0x1a, 0x66, 0x1b, 0x16, 0x41, 0xbd, 0xbd, 0xf7, 0x4d, 0xc0,
	0x4a, 0xe5, 0x27, 0x49, 0x5f, 0x7f, 0x58, 0xe3, 0x27, 0x2d,
	0xe5, 0xc9, 0x66, 0xe, 0x52, 0x38, 0x16, 0x38, 0xfb, 0x16,
	0xeb, 0x53, 0x3f, 0xe6, 0xfd, 0xe9, 0xa2, 0x5e, 0x25, 0x59,
	0xd8, 0x79, 0x45, 0xff, 0x3, 0x4c, 0x26, 0xa2, 0x0, 0x5a,
	0x8e, 0xc2, 0x51, 0xa1, 0x15, 0xf9, 0x7b, 0xf4, 0x5c, 0x81,
	0x9b, 0x18, 0x47, 0x35, 0xd8, 0x2d, 0x5, 0xe9, 0xad, 0xf,
	0x35, 0x74, 0x15, 0xa3, 0x8e, 0x8b, 0xcc, 0x27, 0xda, 0x7c,
	0x5d, 0xe4, 0xfa, 0x4, 0xd3, 0x5, 0xb, 0xba, 0x3a, 0xb2,
	0x49, 0x45, 0x2f, 0x47, 0xc7, 0xd, 0x41, 0x3f, 0x97, 0x80,
	0x4d, 0x3f, 0xc1, 0xb5, 0xbb, 0x70, 0x5f, 0xa7, 0x37, 0xaf,
	0x48, 0x22, 0x12, 0x45, 0x2e, 0xf5, 0xf, 0x87, 0x92, 0xe2,
	0x84, 0x1, 0xf9, 0x12, 0xf, 0x14, 0x15, 0x24, 0xce, 0x89,
	0x99, 0xee, 0xb9, 0xc4, 0x17, 0x70, 0x70, 0x15, 0xea, 0xbe,
	0xc6, 0x6c, 0x1f, 0x62, 0xb3, 0xf4, 0x2d, 0x16, 0x87, 0xfb,
	0x56, 0x1e, 0x45, 0xab, 0xae, 0x32, 0xe4, 0x5e, 0x91, 0xed,
	0x53, 0x66, 0x5e, 0xbd, 0xed, 0xad, 0xe6, 0x12, 0x39, 0xd,
	0x83, 0xc9, 0xe8, 0x6b, 0x6c, 0x2d, 0xa5, 0xee, 0xc4, 0x5a,
	0x66, 0xae, 0x8c, 0x97, 0xd7, 0xd, 0x6c, 0x49, 0xc7, 0xf5,
	0xc4, 0x92, 0x31, 0x8b, 0x9, 0xee, 0x33, 0xda, 0xa9, 0x37,
	0xb6, 0x49, 0x18, 0xf8, 0xe, 0x60, 0x45, 0xc8, 0x33, 0x91,
	0xef, 0x20, 0x57, 0x10, 0xbe, 0x78, 0x2d, 0x83, 0x26, 0xd6,
	0xca, 0x61, 0xf9, 0x2f, 0xe0, 0xbf, 0x5, 0x30, 0x52, 0x5a,
	0x12, 0x1c, 0x0, 0xa7, 0x5d, 0xcc, 0x7c, 0x2e, 0xc5, 0x95,
	0x8b, 0xa3, 0x3b, 0xf0, 0x43, 0x2e, 0x5e, 0xdd, 0x0, 0xdb,
	0xd, 0xb3, 0x37, 0x99, 0xa9, 0xcd, 0x9c, 0xb7, 0x43, 0xf7,
	0x35, 0x44, 0x21, 0xc2, 0x82, 0x71, 0xab, 0x8d, 0xaa, 0xb4,
	0x41, 0x11, 0xec, 0x1e, 0x8d, 0xfc, 0x14, 0x82, 0x92, 0x4e,
	0x83, 0x6a, 0xa, 0x6b, 0x35, 0x5e, 0x5d, 0xe9, 0x5c, 0xcc,
	0x8c, 0xde, 0x39, 0xd1, 0x4a, 0x5b, 0x5f, 0x63, 0xa9, 0x64,
	0xe0, 0xa, 0xcb, 0xb, 0xb8, 0x5a, 0x7c, 0xc3, 0xb, 0xe6,
	0xbe, 0xfe, 0x8b, 0xf, 0x7d, 0x34, 0x8e, 0x2, 0x66, 0x74,
	0x1, 0x6c, 0xca, 0x76, 0xac, 0x7c, 0x67, 0x8, 0x2f, 0x3f,
	0x1a, 0xa6, 0x2c, 0x60, 0xb3, 0xff, 0xda, 0x8d, 0xb8, 0x12,
	0xc, 0x0, 0x7f, 0xcc, 0x50, 0xa1, 0x5c, 0x64, 0xa1, 0xe2,
	0x5f, 0x32, 0x65, 0xc9, 0x9c, 0xbe, 0xd6, 0xa, 0x13, 0x87,
	0x3c, 0x2a, 0x45, 0x47, 0xc, 0xca, 0x42, 0x82, 0xfa, 0x89,
	0x65, 0xe7, 0x89, 0xb4, 0x8f, 0xf7, 0x1e, 0xe6, 0x23, 0xa5,
	0xd0, 0x59, 0x37, 0x79, 0x92, 0xd7, 0xce, 0x3d, 0xfd, 0xe3,
	0xa1, 0xb, 0xcf, 0x6c, 0x85, 0xa0, 0x65, 0xf3, 0x5c, 0xc6,
	0x4a, 0x63, 0x5f, 0x6e, 0x3a, 0x3a, 0x2a, 0x8b, 0x6a, 0xb6,
	0x2f, 0xbb, 0xf8, 0xb2, 0x4b, 0x62, 0xbc, 0x1a, 0x91, 0x25,
	0x66, 0xe3, 0x69, 0xca, 0x60, 0x49, 0xb, 0xf6, 0x8a, 0xbe,
	0x3e, 0x76, 0x53, 0xc2, 0x7a, 0xa8, 0x4, 0x17, 0x75, 0xf1,
	0xf3, 0x3, 0x62, 0x1b, 0x85, 0xb2, 0xb0, 0xef, 0x80, 0x15,
	0xb6, 0xd4, 0x4e, 0xdf, 0x71, 0xac, 0xdb, 0x2a, 0x4, 0xd4,
	0xb4, 0x21, 0xba, 0x65, 0x56, 0x57, 0xe8, 0xfa, 0x84, 0xa2,
	0x7d, 0x13, 0xe, 0xaf, 0xd7, 0x9a, 0x58, 0x2a, 0xa3, 0x81,
	0x84, 0x8d, 0x9, 0xa0, 0x6a, 0xc1, 0xbb, 0xd9, 0xf5, 0x86,
	0xac, 0xbd, 0x75, 0x61, 0x9, 0xe6, 0x8c, 0x3d, 0x77, 0xb2,
	0xed, 0x30, 0x20, 0xe4, 0x0, 0x1d, 0x97, 0xe8, 0xbf, 0xc7,
	0x0, 0x1b, 0x21, 0xb1, 0x16, 0xe7, 0x41, 0x67, 0x2e, 0xec,
	0x38, 0xbc, 0xe5, 0x1b, 0xb4, 0x6, 0x23, 0x31, 0x71, 0x1c,
	0x49, 0xcd, 0x76, 0x4a, 0x76, 0x36, 0x8d, 0xa3, 0x89, 0x8b,
	0x4a, 0x7a, 0xf4, 0x87, 0xc8, 0x15, 0xf, 0x37, 0x39, 0xf6,
	0x6d, 0x80, 0x19, 0xef, 0x5c, 0xa8, 0x66, 0xce, 0x1b, 0x16,
	0x79, 0x21, 0xdf, 0xd7, 0x31, 0x30, 0xc4, 0x21, 0xdd, 0x34,
	0x5b, 0xd2, 0x1a, 0x2b, 0x3e, 0x5d, 0xf7, 0xea, 0xca, 0x5,
	0x8e, 0xb7, 0xcb, 0x49, 0x2e, 0xa0, 0xe3, 0xf4, 0xa7, 0x48,
	0x19, 0x10, 0x9c, 0x4, 0xa7, 0xf4, 0x28, 0x74, 0xc8, 0x6f,
	0x63, 0x20, 0x2b, 0x46, 0x24, 0x26, 0x19, 0x1d, 0xd1, 0x2c,
	0x31, 0x6d, 0x5a, 0x29, 0xa2, 0x6, 0xa6, 0xb2, 0x41, 0xcc,
	0xa, 0x27, 0x96, 0x9, 0x96, 0xac, 0x47, 0x65, 0x78, 0x68,
	0x51, 0x98, 0xd6, 0xd8, 0xa6, 0x2d, 0xa0, 0xcf, 0xec, 0xe2,
	0x74, 0xf2, 0x82, 0xe3, 0x97, 0xd9, 0x7e, 0xd4, 0xf8, 0xb,
	0x70, 0x43, 0x3d, 0xb1, 0x7b, 0x97, 0x80, 0xd6, 0xcb, 0xd7,
	0x19, 0xbc, 0x63, 0xb, 0xfd, 0x4d, 0x88, 0xfe, 0x67, 0xac,
	0xb8, 0xcc, 0x50, 0xb7, 0x68, 0xb3, 0x5b, 0xd6, 0x1e, 0x25,
	0xfc, 0x5f, 0x3c, 0x8d, 0xb1, 0x33, 0x7c, 0xb3, 0x49, 0x1,
	0x3f, 0x71, 0x55, 0xe, 0x51, 0xba, 0x61, 0x26, 0xfa, 0xea,
	0xe5, 0xb5, 0xe8, 0xaa, 0xcf, 0xcd, 0x96, 0x9f, 0xd6, 0xc1,
	0x5f, 0x53, 0x91, 0xad, 0x5, 0xde, 0x20, 0xe7, 0x51, 0xda,
	0x5b, 0x95, 0x67, 0xed, 0xf4, 0xee, 0x42, 0x65, 0x70, 0x13,
	0xb, 0x70, 0x14, 0x1c, 0xc9, 0xe0, 0x19, 0xca, 0x5f, 0xf5,
	0x1d, 0x70, 0x4b, 0x6c, 0x6, 0x74, 0xec, 0xb5, 0x2e, 0x77,
	0xe1, 0x74, 0xa1, 0xa3, 0x99, 0xa0, 0x85, 0x9e, 0xf1, 0xac,
	0xd8, 0x7e,
};
#endif
/**
 * ============================================================================
 * Boot states support (GREEN, YELLOW, ORANGE, RED) and dm_verity
 * ============================================================================
 */
char *avb_set_state(AvbOps *ops, enum avb_boot_state boot_state)
{
	struct AvbOpsData *data;
	char *cmdline = NULL;

	if (!ops)
		return NULL;

	data = (struct AvbOpsData *)ops->user_data;
	if (!data)
		return NULL;

	data->boot_state = boot_state;
	switch (boot_state) {
	case AVB_GREEN:
		cmdline = "androidboot.verifiedbootstate=green";
		break;
	case AVB_YELLOW:
		cmdline = "androidboot.verifiedbootstate=yellow";
		break;
	case AVB_ORANGE:
		cmdline = "androidboot.verifiedbootstate=orange";
	case AVB_RED:
		break;
	}

	return cmdline;
}

char *append_cmd_line(char *cmdline_orig, char *cmdline_new)
{
	char *cmd_line;

	if (!cmdline_new)
		return cmdline_orig;

	if (cmdline_orig)
		cmd_line = cmdline_orig;
	else
		cmd_line = " ";

	cmd_line = avb_strdupv(cmd_line, " ", cmdline_new, NULL);

	return cmd_line;
}

static int avb_find_dm_args(char **args, char *str)
{
	int i;

	if (!str)
		return -1;

	for (i = 0; i < AVB_MAX_ARGS && args[i]; ++i) {
		if (strstr(args[i], str))
			return i;
	}

	return -1;
}

static char *avb_set_enforce_option(const char *cmdline, const char *option)
{
	char *cmdarg[AVB_MAX_ARGS];
	char *newargs = NULL;
	int i = 0;
	int total_args;

	memset(cmdarg, 0, sizeof(cmdarg));
	cmdarg[i++] = strtok((char *)cmdline, " ");

	do {
		cmdarg[i] = strtok(NULL, " ");
		if (!cmdarg[i])
			break;

		if (++i >= AVB_MAX_ARGS) {
			printf("%s: Can't handle more then %d args\n",
			       __func__, i);
			return NULL;
		}
	} while (true);

	total_args = i;
	i = avb_find_dm_args(&cmdarg[0], VERITY_TABLE_OPT_LOGGING);
	if (i >= 0) {
		cmdarg[i] = (char *)option;
	} else {
		i = avb_find_dm_args(&cmdarg[0], VERITY_TABLE_OPT_RESTART);
		if (i < 0) {
			printf("%s: No verity options found\n", __func__);
			return NULL;
		}

		cmdarg[i] = (char *)option;
	}

	for (i = 0; i <= total_args; i++)
		newargs = append_cmd_line(newargs, cmdarg[i]);

	return newargs;
}

char *avb_set_ignore_corruption(const char *cmdline)
{
	char *newargs = NULL;

	newargs = avb_set_enforce_option(cmdline, VERITY_TABLE_OPT_LOGGING);
	if (newargs)
		newargs = append_cmd_line(newargs,
					  "androidboot.veritymode=eio");

	return newargs;
}

char *avb_set_enforce_verity(const char *cmdline)
{
	char *newargs;

	newargs = avb_set_enforce_option(cmdline, VERITY_TABLE_OPT_RESTART);
	if (newargs)
		newargs = append_cmd_line(newargs,
					  "androidboot.veritymode=enforcing");
	return newargs;
}

/**
 * ============================================================================
 * IO(mmc) auxiliary functions
 * ============================================================================
 */
static unsigned long mmc_read_and_flush(struct mmc_part *part,
					lbaint_t start,
					lbaint_t sectors,
					void *buffer)
{
	unsigned long blks;
	void *tmp_buf;
	size_t buf_size;
	bool unaligned = is_buf_unaligned(buffer);

	if (start < part->info.start) {
		printf("%s: partition start out of bounds\n", __func__);
		return 0;
	}
	if ((start + sectors) > (part->info.start + part->info.size)) {
		sectors = part->info.start + part->info.size - start;
		printf("%s: read sector aligned to partition bounds (%ld)\n",
		       __func__, sectors);
	}

	/*
	 * Reading fails on unaligned buffers, so we have to
	 * use aligned temporary buffer and then copy to destination
	 */

	if (unaligned) {
		printf("Handling unaligned read buffer..\n");
		tmp_buf = get_sector_buf();
		buf_size = get_sector_buf_size();
		if (sectors > buf_size / part->info.blksz)
			sectors = buf_size / part->info.blksz;
	} else {
		tmp_buf = buffer;
	}

	blks = blk_dread(part->mmc_blk,
			 start, sectors, tmp_buf);
	/* flush cache after read */
	flush_cache((ulong)tmp_buf, sectors * part->info.blksz);

	if (unaligned)
		memcpy(buffer, tmp_buf, sectors * part->info.blksz);

	return blks;
}

static unsigned long mmc_write(struct mmc_part *part, lbaint_t start,
			       lbaint_t sectors, void *buffer)
{
	void *tmp_buf;
	size_t buf_size;
	bool unaligned = is_buf_unaligned(buffer);

	if (start < part->info.start) {
		printf("%s: partition start out of bounds\n", __func__);
		return 0;
	}
	if ((start + sectors) > (part->info.start + part->info.size)) {
		sectors = part->info.start + part->info.size - start;
		printf("%s: sector aligned to partition bounds (%ld)\n",
		       __func__, sectors);
	}
	if (unaligned) {
		tmp_buf = get_sector_buf();
		buf_size = get_sector_buf_size();
		printf("Handling unaligned wrire buffer..\n");
		if (sectors > buf_size / part->info.blksz)
			sectors = buf_size / part->info.blksz;

		memcpy(tmp_buf, buffer, sectors * part->info.blksz);
	} else {
		tmp_buf = buffer;
	}

	return blk_dwrite(part->mmc_blk,
			  start, sectors, tmp_buf);
}

static struct mmc_part *get_partition(AvbOps *ops, const char *partition)
{
	int ret;
	u8 dev_num;
	int part_num = 0;
	struct mmc_part *part;
	struct blk_desc *mmc_blk;

	part = malloc(sizeof(struct mmc_part));
	if (!part)
		return NULL;

	dev_num = get_boot_device(ops);
	part->mmc = find_mmc_device(dev_num);
	if (!part->mmc) {
		printf("No MMC device at slot %x\n", dev_num);
		goto err;
	}

	if (mmc_init(part->mmc)) {
		printf("MMC initialization failed\n");
		goto err;
	}

	ret = mmc_switch_part(part->mmc, part_num);
	if (ret)
		goto err;

	mmc_blk = mmc_get_blk_desc(part->mmc);
	if (!mmc_blk) {
		printf("Error - failed to obtain block descriptor\n");
		goto err;
	}

	ret = part_get_info_by_name(mmc_blk, partition, &part->info);
	if (!ret) {
		printf("Can't find partition '%s'\n", partition);
		goto err;
	}

	part->dev_num = dev_num;
	part->mmc_blk = mmc_blk;

	return part;
err:
	free(part);
	return NULL;
}

static AvbIOResult mmc_byte_io(AvbOps *ops,
			       const char *partition,
			       s64 offset,
			       size_t num_bytes,
			       void *buffer,
			       size_t *out_num_read,
			       enum mmc_io_type io_type)
{
	ulong ret;
	struct mmc_part *part;
	u64 start_offset, start_sector, sectors, residue;
	u8 *tmp_buf;
	size_t io_cnt = 0;

	if (!partition || !buffer || io_type > IO_WRITE)
		return AVB_IO_RESULT_ERROR_IO;

	part = get_partition(ops, partition);
	if (!part)
		return AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	if (!part->info.blksz)
		return AVB_IO_RESULT_ERROR_IO;

	start_offset = calc_offset(part, offset);
	while (num_bytes) {
		start_sector = start_offset / part->info.blksz;
		sectors = num_bytes / part->info.blksz;
		/* handle non block-aligned reads */
		if (start_offset % part->info.blksz ||
		    num_bytes < part->info.blksz) {
			tmp_buf = get_sector_buf();
			if (start_offset % part->info.blksz) {
				residue = part->info.blksz -
					(start_offset % part->info.blksz);
				if (residue > num_bytes)
					residue = num_bytes;
			} else {
				residue = num_bytes;
			}

			if (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 1, tmp_buf);

				if (ret != 1) {
					printf("%s: read error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					return AVB_IO_RESULT_ERROR_IO;
				}
				/*
				 * if this is not aligned at sector start,
				 * we have to adjust the tmp buffer
				 */
				tmp_buf += (start_offset % part->info.blksz);
				memcpy(buffer, (void *)tmp_buf, residue);
			} else {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 1, tmp_buf);

				if (ret != 1) {
					printf("%s: read error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					return AVB_IO_RESULT_ERROR_IO;
				}
				memcpy((void *)tmp_buf +
					start_offset % part->info.blksz,
					buffer, residue);

				ret = mmc_write(part, part->info.start +
						start_sector, 1, tmp_buf);
				if (ret != 1) {
					printf("%s: write error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					return AVB_IO_RESULT_ERROR_IO;
				}
			}

			io_cnt += residue;
			buffer += residue;
			start_offset += residue;
			num_bytes -= residue;
			continue;
		}

		if (sectors) {
			if (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 sectors, buffer);
			} else {
				ret = mmc_write(part,
						part->info.start +
						start_sector,
						sectors, buffer);
			}

			if (!ret) {
				printf("%s: sector read error\n", __func__);
				return AVB_IO_RESULT_ERROR_IO;
			}

			io_cnt += ret * part->info.blksz;
			buffer += ret * part->info.blksz;
			start_offset += ret * part->info.blksz;
			num_bytes -= ret * part->info.blksz;
		}
	}

	/* Set counter for read operation */
	if (io_type == IO_READ && out_num_read)
		*out_num_read = io_cnt;

	return AVB_IO_RESULT_OK;
}

/**
 * ============================================================================
 * AVB 2.0 operations
 * ============================================================================
 */

/**
 * read_from_partition() - reads @num_bytes from  @offset from partition
 * identified by a string name
 *
 * @ops: contains AVB ops handlers
 * @partition_name: partition name, NUL-terminated UTF-8 string
 * @offset: offset from the beginning of partition
 * @num_bytes: amount of bytes to read
 * @buffer: destination buffer to store data
 * @out_num_read:
 *
 * @return:
 *      AVB_IO_RESULT_OK, if partition was found and read operation succeed
 *      AVB_IO_RESULT_ERROR_IO, if i/o error occurred from the underlying i/o
 *            subsystem
 *      AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION, if there is no partition with
 *      the given name
 */
static AvbIOResult read_from_partition(AvbOps *ops,
				       const char *partition_name,
				       s64 offset_from_partition,
				       size_t num_bytes,
				       void *buffer,
				       size_t *out_num_read)
{
	return mmc_byte_io(ops, partition_name, offset_from_partition,
			   num_bytes, buffer, out_num_read, IO_READ);
}

/**
 * write_to_partition() - writes N bytes to a partition identified by a string
 * name
 *
 * @ops: AvbOps, contains AVB ops handlers
 * @partition_name: partition name
 * @offset_from_partition: offset from the beginning of partition
 * @num_bytes: amount of bytes to write
 * @buf: data to write
 * @out_num_read:
 *
 * @return:
 *      AVB_IO_RESULT_OK, if partition was found and read operation succeed
 *      AVB_IO_RESULT_ERROR_IO, if input/output error occurred
 *      AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION, if partition, specified in
 *            @partition_name was not found
 */
static AvbIOResult write_to_partition(AvbOps *ops,
				      const char *partition_name,
				      s64 offset_from_partition,
				      size_t num_bytes,
				      const void *buffer)
{
	return mmc_byte_io(ops, partition_name, offset_from_partition,
			   num_bytes, (void *)buffer, NULL, IO_WRITE);
}

/**
 * validate_vmbeta_public_key() - checks if the given public key used to sign
 * the vbmeta partition is trusted
 *
 * @ops: AvbOps, contains AVB ops handlers
 * @public_key_data: public key for verifying vbmeta partition signature
 * @public_key_length: length of public key
 * @public_key_metadata:
 * @public_key_metadata_length:
 * @out_key_is_trusted:
 *
 * @return:
 *      AVB_IO_RESULT_OK, if partition was found and read operation succeed
 */
static AvbIOResult validate_vbmeta_public_key(AvbOps *ops,
					      const u8 *public_key_data,
					      size_t public_key_length,
					      const u8
					      *public_key_metadata,
					      size_t
					      public_key_metadata_length,
					      bool *out_key_is_trusted)
{
#if defined (CONFIG_AVB_USE_OEM_KEY)
	if (!public_key_length || !public_key_data || !out_key_is_trusted)
		return AVB_IO_RESULT_ERROR_IO;

	*out_key_is_trusted = false;
	if (public_key_length != sizeof(avb_root_oem_pub))
		return AVB_IO_RESULT_ERROR_IO;

	if (memcmp(avb_root_oem_pub, public_key_data, public_key_length) == 0)
		*out_key_is_trusted = true;
	
	return AVB_IO_RESULT_OK;
#else
	if (!public_key_length || !public_key_data || !out_key_is_trusted)
		return AVB_IO_RESULT_ERROR_IO;

	*out_key_is_trusted = false;
	if (public_key_length != sizeof(avb_root_pub))
		return AVB_IO_RESULT_ERROR_IO;

	if (memcmp(avb_root_pub, public_key_data, public_key_length) == 0)
		*out_key_is_trusted = true;

	return AVB_IO_RESULT_OK;
#endif
}

#ifdef CONFIG_OPTEE_TA_AVB
static int get_open_session(struct AvbOpsData *ops_data)
{
	struct udevice *tee = NULL;

	while (!ops_data->tee) {
		const struct tee_optee_ta_uuid uuid = TA_AVB_UUID;
		struct tee_open_session_arg arg;
		int rc;

		tee = tee_find_device(tee, NULL, NULL, NULL);
		if (!tee)
			return -ENODEV;

		memset(&arg, 0, sizeof(arg));
		tee_optee_ta_uuid_to_octets(arg.uuid, &uuid);
		rc = tee_open_session(tee, &arg, 0, NULL);
		if (!rc) {
			ops_data->tee = tee;
			ops_data->session = arg.session;
		}
	}

	return 0;
}

static AvbIOResult invoke_func(struct AvbOpsData *ops_data, u32 func,
			       ulong num_param, struct tee_param *param)
{
	struct tee_invoke_arg arg;

	if (get_open_session(ops_data))
		return AVB_IO_RESULT_ERROR_IO;

	memset(&arg, 0, sizeof(arg));
	arg.func = func;
	arg.session = ops_data->session;

	if (tee_invoke_func(ops_data->tee, &arg, num_param, param))
		return AVB_IO_RESULT_ERROR_IO;
	switch (arg.ret) {
	case TEE_SUCCESS:
		return AVB_IO_RESULT_OK;
	case TEE_ERROR_OUT_OF_MEMORY:
		return AVB_IO_RESULT_ERROR_OOM;
	case TEE_ERROR_STORAGE_NO_SPACE:
		return AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;
	case TEE_ERROR_ITEM_NOT_FOUND:
		return AVB_IO_RESULT_ERROR_NO_SUCH_VALUE;
	case TEE_ERROR_TARGET_DEAD:
		/*
		 * The TA has paniced, close the session to reload the TA
		 * for the next request.
		 */
		tee_close_session(ops_data->tee, ops_data->session);
		ops_data->tee = NULL;
		return AVB_IO_RESULT_ERROR_IO;
	default:
		return AVB_IO_RESULT_ERROR_IO;
	}
}
#endif

/**
 * read_rollback_index() - gets the rollback index corresponding to the
 * location of given by @out_rollback_index.
 *
 * @ops: contains AvbOps handlers
 * @rollback_index_slot:
 * @out_rollback_index: used to write a retrieved rollback index.
 *
 * @return
 *       AVB_IO_RESULT_OK, if the roolback index was retrieved
 */
static AvbIOResult read_rollback_index(AvbOps *ops,
				       size_t rollback_index_slot,
				       u64 *out_rollback_index)
{
#ifdef CONFIG_AVB_ROLLBACK_ENABLE
    if (sec_read_rollback_index(rollback_index_slot, out_rollback_index) != 0) {
        return AVB_IO_RESULT_ERROR_IO;
    }

	return AVB_IO_RESULT_OK;
#else
#ifndef CONFIG_OPTEE_TA_AVB
    /* For now we always return 0 as the stored rollback index. */
	printf("%s not supported yet\n", __func__);

	if (out_rollback_index)
		*out_rollback_index = 0;

	return AVB_IO_RESULT_OK;
#else
	AvbIOResult rc;
	struct tee_param param[2];

	if (rollback_index_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		return AVB_IO_RESULT_ERROR_NO_SUCH_VALUE;

	memset(param, 0, sizeof(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = rollback_index_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_ROLLBACK_INDEX,
			 ARRAY_SIZE(param), param);
	if (rc)
		return rc;

	*out_rollback_index = (u64)param[1].u.value.a << 32 |
			      (u32)param[1].u.value.b;

	return AVB_IO_RESULT_OK;
#endif
#endif
}

/**
 * write_rollback_index() - sets the rollback index corresponding to the
 * location of given by @out_rollback_index.
 *
 * @ops: contains AvbOps handlers
 * @rollback_index_slot:
 * @rollback_index: rollback index to write.
 *
 * @return
 *       AVB_IO_RESULT_OK, if the roolback index was retrieved
 */
static AvbIOResult write_rollback_index(AvbOps *ops,
					size_t rollback_index_slot,
					u64 rollback_index)
{
#ifdef CONFIG_AVB_ROLLBACK_ENABLE
    if (sec_write_rollback_index(rollback_index_slot, rollback_index) != 0) {
        return AVB_IO_RESULT_ERROR_IO;
    }
    
	return AVB_IO_RESULT_OK;
#else
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now this is a no-op. */
	printf("%s not supported yet\n", __func__);

	return AVB_IO_RESULT_OK;
#else
	struct tee_param param[2];

	if (rollback_index_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		return AVB_IO_RESULT_ERROR_NO_SUCH_VALUE;

	memset(param, 0, sizeof(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = rollback_index_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[1].u.value.a = (u32)(rollback_index >> 32);
	param[1].u.value.b = (u32)rollback_index;

	return invoke_func(ops->user_data, TA_AVB_CMD_WRITE_ROLLBACK_INDEX,
			   ARRAY_SIZE(param), param);
#endif
#endif
}

/**
 * read_is_device_unlocked() - gets whether the device is unlocked
 *
 * @ops: contains AVB ops handlers
 * @out_is_unlocked: device unlock state is stored here, true if unlocked,
 *       false otherwise
 *
 * @return:
 *       AVB_IO_RESULT_OK: state is retrieved successfully
 *       AVB_IO_RESULT_ERROR_IO: an error occurred
 */
static AvbIOResult read_is_device_unlocked(AvbOps *ops, bool *out_is_unlocked)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now we always return that the device is unlocked. */

	printf("%s not supported yet\n", __func__);

	*out_is_unlocked = true;

	return AVB_IO_RESULT_OK;
#else
	AvbIOResult rc;
	struct tee_param param = { .attr = TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT };

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_LOCK_STATE, 1, &param);
	if (rc)
		return rc;
	*out_is_unlocked = !param.u.value.a;
	return AVB_IO_RESULT_OK;
#endif
}

/**
 * get_unique_guid_for_partition() - gets the GUID for a partition identified
 * by a string name
 *
 * @ops: contains AVB ops handlers
 * @partition: partition name (NUL-terminated UTF-8 string)
 * @guid_buf: buf, used to copy in GUID string. Example of value:
 *      527c1c6d-6361-4593-8842-3c78fcd39219
 * @guid_buf_size: @guid_buf buffer size
 *
 * @return:
 *      AVB_IO_RESULT_OK, on success (GUID found)
 *      AVB_IO_RESULT_ERROR_IO, if incorrect buffer size (@guid_buf_size) was
 *             provided
 *      AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION, if partition was not found
 */
static AvbIOResult get_unique_guid_for_partition(AvbOps *ops,
						 const char *partition,
						 char *guid_buf,
						 size_t guid_buf_size)
{
	struct mmc_part *part;
	size_t uuid_size;

	part = get_partition(ops, partition);
	if (!part)
		return AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	uuid_size = sizeof(part->info.uuid);
	if (uuid_size > guid_buf_size)
		return AVB_IO_RESULT_ERROR_IO;

	memcpy(guid_buf, part->info.uuid, uuid_size);
	guid_buf[uuid_size - 1] = 0;

	return AVB_IO_RESULT_OK;
}

/**
 * get_size_of_partition() - gets the size of a partition identified
 * by a string name
 *
 * @ops: contains AVB ops handlers
 * @partition: partition name (NUL-terminated UTF-8 string)
 * @out_size_num_bytes: returns the value of a partition size
 *
 * @return:
 *      AVB_IO_RESULT_OK, on success (GUID found)
 *      AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE, out_size_num_bytes is NULL
 *      AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION, if partition was not found
 */
static AvbIOResult get_size_of_partition(AvbOps *ops,
					 const char *partition,
					 u64 *out_size_num_bytes)
{
	struct mmc_part *part;

	if (!out_size_num_bytes)
		return AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;

	part = get_partition(ops, partition);
	if (!part)
		return AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	*out_size_num_bytes = part->info.blksz * part->info.size;

	return AVB_IO_RESULT_OK;
}

#ifdef CONFIG_OPTEE_TA_AVB
static AvbIOResult read_persistent_value(AvbOps *ops,
					 const char *name,
					 size_t buffer_size,
					 u8 *out_buffer,
					 size_t *out_num_bytes_read)
{
	AvbIOResult rc;
	struct tee_shm *shm_name;
	struct tee_shm *shm_buf;
	struct tee_param param[2];
	struct udevice *tee;
	size_t name_size = strlen(name) + 1;

	if (get_open_session(ops->user_data))
		return AVB_IO_RESULT_ERROR_IO;

	tee = ((struct AvbOpsData *)ops->user_data)->tee;

	rc = tee_shm_alloc(tee, name_size,
			   TEE_SHM_ALLOC, &shm_name);
	if (rc)
		return AVB_IO_RESULT_ERROR_OOM;

	rc = tee_shm_alloc(tee, buffer_size,
			   TEE_SHM_ALLOC, &shm_buf);
	if (rc) {
		rc = AVB_IO_RESULT_ERROR_OOM;
		goto free_name;
	}

	memcpy(shm_name->addr, name, name_size);

	memset(param, 0, sizeof(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.size = name_size;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.size = buffer_size;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_PERSIST_VALUE,
			 2, param);
	if (rc)
		goto out;

	if (param[1].u.memref.size > buffer_size) {
		rc = AVB_IO_RESULT_ERROR_NO_SUCH_VALUE;
		goto out;
	}

	*out_num_bytes_read = param[1].u.memref.size;

	memcpy(out_buffer, shm_buf->addr, *out_num_bytes_read);

out:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	return rc;
}

static AvbIOResult write_persistent_value(AvbOps *ops,
					  const char *name,
					  size_t value_size,
					  const u8 *value)
{
	AvbIOResult rc;
	struct tee_shm *shm_name;
	struct tee_shm *shm_buf;
	struct tee_param param[2];
	struct udevice *tee;
	size_t name_size = strlen(name) + 1;

	if (get_open_session(ops->user_data))
		return AVB_IO_RESULT_ERROR_IO;

	tee = ((struct AvbOpsData *)ops->user_data)->tee;

	if (!value_size)
		return AVB_IO_RESULT_ERROR_NO_SUCH_VALUE;

	rc = tee_shm_alloc(tee, name_size,
			   TEE_SHM_ALLOC, &shm_name);
	if (rc)
		return AVB_IO_RESULT_ERROR_OOM;

	rc = tee_shm_alloc(tee, value_size,
			   TEE_SHM_ALLOC, &shm_buf);
	if (rc) {
		rc = AVB_IO_RESULT_ERROR_OOM;
		goto free_name;
	}

	memcpy(shm_name->addr, name, name_size);
	memcpy(shm_buf->addr, value, value_size);

	memset(param, 0, sizeof(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.size = name_size;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.size = value_size;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_WRITE_PERSIST_VALUE,
			 2, param);
	if (rc)
		goto out;

out:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	return rc;
}
#endif

/**
 * ============================================================================
 * AVB2.0 AvbOps alloc/initialisation/free
 * ============================================================================
 */
AvbOps *avb_ops_alloc(int boot_device)
{
	struct AvbOpsData *ops_data;

	ops_data = avb_calloc(sizeof(struct AvbOpsData));
	if (!ops_data)
		return NULL;

	ops_data->ops.user_data = ops_data;

	ops_data->ops.read_from_partition = read_from_partition;
	ops_data->ops.write_to_partition = write_to_partition;
	ops_data->ops.validate_vbmeta_public_key = validate_vbmeta_public_key;
	ops_data->ops.read_rollback_index = read_rollback_index;
	ops_data->ops.write_rollback_index = write_rollback_index;
	ops_data->ops.read_is_device_unlocked = read_is_device_unlocked;
	ops_data->ops.get_unique_guid_for_partition =
		get_unique_guid_for_partition;
#ifdef CONFIG_OPTEE_TA_AVB
	ops_data->ops.write_persistent_value = write_persistent_value;
	ops_data->ops.read_persistent_value = read_persistent_value;
#endif
	ops_data->ops.get_size_of_partition = get_size_of_partition;
	ops_data->mmc_dev = boot_device;

	return &ops_data->ops;
}

void avb_ops_free(AvbOps *ops)
{
	struct AvbOpsData *ops_data;

	if (!ops)
		return;

	ops_data = ops->user_data;

	if (ops_data) {
#ifdef CONFIG_OPTEE_TA_AVB
		if (ops_data->tee)
			tee_close_session(ops_data->tee, ops_data->session);
#endif
		avb_free(ops_data);
	}
}
