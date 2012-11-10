/*
 *  Copyright (c) 2012  Finnbarr P. Murphy.  All rights reserved.
 *
 *  efiauth.h 
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public Licence
 *  as published by the Free Software Foundation; either version
 *  2 of the Licence, or (at your option) any later version.
 */

#ifndef _EFIAUTH_H
#define _EFIAUTH_H

#include <efi.h>
#include <efilib.h>

#define WIN_CERT_TYPE_PKCS_SIGNED_DATA 0x0002
#define WIN_CERT_TYPE_EFI_PKCS115      0x0EF0
#define WIN_CERT_TYPE_EFI_GUID         0x0EF1

#define OFFSET_OF(TYPE, Field) ((UINTN) &(((TYPE *)0)->Field))

#define EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS              0x00000010
#define EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS   0x00000020
#define EFI_VARIABLE_APPEND_WRITE                            0x00000040

#define EFI_CERT_X509_GUID \
  (EFI_GUID) {0xa5c059a1, 0x94e4, 0x4aa7, {0x87,0xb5,0xab,0x15,0x5c,0x2b,0xf0,0x72}}

#define EFI_CERT_PKCS7_GUID \
  (EFI_GUID) {0x4aafd29d, 0x68df, 0x49ee, {0x8a,0xa9,0x34,0x7d,0x37,0x56,0x65,0xa7}}

#define EFI_CERT_RSA2048_GUID \
   (EFI_GUID) {0x3c5766e8, 0x269c, 0x4e34, {0xaa,0x14,0xed,0x77,0x6e,0x85,0xb3,0xb6}}

#define EFI_CERT_TYPE_RSA2048_SHA256_GUID \
   (EFI_GUID) {0xa7717414, 0xc616, 0x4977, {0x94,0x20,0x84,0x47,0x12,0xa7,0x35,0xbf}}

#define EFI_SIG_DB_GUID \
   (EFI_GUID) {0xd719b2cb, 0x3d3a, 0x4596, {0xa3,0xbc,0xda,0xd0,0xe,0x67,0x65,0x6f}};


#pragma pack(1)
typedef struct {
    EFI_GUID  SignatureOwner;
    UINT8     SignatureData[1];
} EFI_SIGNATURE_DATA;

typedef struct {
    EFI_GUID  SignatureType;
    UINT32    SignatureListSize;
    UINT32    SignatureHeaderSize;
    UINT32    SignatureSize; 
} EFI_SIGNATURE_LIST;
#pragma pack()

typedef struct {
    UINT32    dwLength;
    UINT16    wRevision;
    UINT16    wCertificateType;
} WIN_CERTIFICATE;

typedef struct {
    EFI_GUID  HashType;
    UINT8     PublicKey[256];
    UINT8     Signature[256];
} EFI_CERT_BLOCK_RSA_2048_SHA256;

typedef struct {
    WIN_CERTIFICATE Hdr;
    EFI_GUID  CertType;
    UINT8     CertData[1];
} WIN_CERTIFICATE_UEFI_GUID;

typedef struct {     
    WIN_CERTIFICATE Hdr;
    EFI_GUID                    HashAlgorithm;
} WIN_CERTIFICATE_EFI_PKCS1_15;

typedef struct {
    UINT64                      MonotonicCount;
    WIN_CERTIFICATE_UEFI_GUID   AuthInfo;
} EFI_VARIABLE_AUTHENTICATION;

typedef struct {
    EFI_TIME                    TimeStamp;
    WIN_CERTIFICATE_UEFI_GUID   AuthInfo;
} EFI_VARIABLE_AUTHENTICATION_2;

#endif  /* _EFIAUTH_H */
