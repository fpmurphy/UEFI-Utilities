//
//   Copyright (c) 2012  Finnbarr P. Murphy.  All rights reserved.
//
//   List UEFI Secure Boot certificates.
//

#include <efi.h>
#include <efilib.h>

#include "oid_registry.h"
#include "efiauth.h"
#include "efishell.h"
#include "x509.h"
#include "asn1_ber_decoder.h"

#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))
#define UTCDATE_LEN 23

/* temporary store for output text */
CHAR16 tmpbuf[1000];
int    wrapno = 1;


EFI_STATUS
get_args(EFI_HANDLE image, UINTN *argc, CHAR16 ***argv)
{
    EFI_STATUS status;
    EFI_SHELL_INTERFACE *shell;
    EFI_GUID gEfiShellInterfaceGuid = EFI_SHELL_INTERFACE_GUID;

    status = uefi_call_wrapper(BS->OpenProtocol, 6,
                               image, &gEfiShellInterfaceGuid,
                               (VOID **)&shell, image, NULL,
                               EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(status))
        return status;

    *argc = shell->Argc;
    *argv = shell->Argv;

    status = uefi_call_wrapper(BS->CloseProtocol, 4, image,
                               &gEfiShellInterfaceGuid, image, NULL);
    return status;
}


CHAR16 *
ASCII_to_UCS2(const char *s, int len)
{
    CHAR16 *ret = NULL;
    int i;

    ret = AllocateZeroPool(len*2 + 2);
    if (!ret)
        return NULL;

    for (i = 0; i < len; i++)
        ret[i] = s[i];

    return ret;
}


int 
do_version(void *context, long state_index,
           unsigned char tag,
           const void *value, long vlen)
{
    int version = *(const char *)value;

    Print(L"  Version: %d (0x%02x)\n", version + 1, version);

    return 0;
}


int
do_signature(void *context, long state_index,
             unsigned char tag,
             const void *value, long vlen)
{
    Print(L"  Signature Algorithm: %s\n", tmpbuf);
    tmpbuf[0] = '\0';

    return 0;
}


int
do_algorithm(void *context, long state_index,
             unsigned char tag,
             const void *value, long vlen)
{
    enum OID oid; 
    CHAR16 buffer[100];

    oid = Lookup_OID(value, vlen);
    Sprint_OID(value, vlen, buffer, sizeof(buffer));
    if (oid == OID_id_dsa_with_sha1)
        StrCpy(tmpbuf, L"id_dsa_with_sha1");
    else if (oid == OID_id_dsa)
        StrCpy(tmpbuf, L"id_dsa");
    else if (oid == OID_id_ecdsa_with_sha1)
        StrCpy(tmpbuf, L"id_ecdsa_with_sha1");
    else if (oid == OID_id_ecPublicKey)
        StrCpy(tmpbuf, L"id_ecPublicKey");
    else if (oid == OID_rsaEncryption)
        StrCpy(tmpbuf, L"rsaEncryption");
    else if (oid == OID_md2WithRSAEncryption)
        StrCpy(tmpbuf, L"md2WithRSAEncryption");
    else if (oid == OID_md3WithRSAEncryption)
        StrCpy(tmpbuf, L"md3WithRSAEncryption");
    else if (oid == OID_md4WithRSAEncryption)
        StrCpy(tmpbuf, L"md4WithRSAEncryption");
    else if (oid == OID_sha1WithRSAEncryption)
        StrCpy(tmpbuf, L"sha1WithRSAEncryption");
    else if (oid == OID_sha256WithRSAEncryption)
        StrCpy(tmpbuf, L"sha256WithRSAEncryption");
    else if (oid == OID_sha384WithRSAEncryption)
        StrCpy(tmpbuf, L"sha384WithRSAEncryption");
    else if (oid == OID_sha512WithRSAEncryption)
        StrCpy(tmpbuf, L"sha512WithRSAEncryption");
    else if (oid == OID_sha224WithRSAEncryption)
        StrCpy(tmpbuf, L"sha224WithRSAEncryption");
    else {
        StrCat(tmpbuf, L" (");
        StrCat(tmpbuf, buffer);
        StrCat(tmpbuf, L")");
    }

    return 0;
}

int 
do_serialnumber(void *context, long state_index,
                unsigned char tag,
                const void *value, long vlen)
{
    int i;
    char *p = (char *)value;

    Print(L"  Serial Number: ");
    if (vlen > 4) {
        for (i = 0; i < vlen; i++, p++) {
            Print(L"%02x%c", (UINT8)*p, ((i+1 == vlen)?'\n':':'));
        }
    } else 
        Print(L"\n");

    return 0;
}


int
do_issuer(void *context, long  state_index,
          unsigned char tag,
          const void *value, long vlen)
{
    Print(L"  Issuer:%s\n", tmpbuf);
    tmpbuf[0] = '\0';

    return 0;
}

int
do_subject(void *context, long state_index,
           unsigned char tag,
           const void *value, long vlen)
{
    Print(L"  Subject:%s\n", tmpbuf);
    tmpbuf[0] = '\0';

    return 0;
}


int
do_attribute_type(void *context, long state_index,
                  unsigned char tag,
                  const void *value, long vlen)
{
    enum OID oid; 
    CHAR16 buffer[60];

    oid = Lookup_OID(value, vlen);
    Sprint_OID(value, vlen, buffer, sizeof(buffer));
    if (oid == OID_countryName) {
        StrCat(tmpbuf, L" CN=");
    } else if (oid == OID_stateOrProvinceName) {
        StrCat(tmpbuf, L" ST=");
    } else if (oid == OID_locality) {
        StrCat(tmpbuf, L" L=");
    } else if (oid == OID_organizationName) {
        StrCat(tmpbuf, L" O=");
    } else if (oid == OID_commonName) {
        StrCat(tmpbuf, L" CN=");
    } else {
        StrCat(tmpbuf, L" (");
        StrCat(tmpbuf, buffer);
        StrCat(tmpbuf, L")");
    }

    return 0;
}


int
do_attribute_value(void *context, long state_index,
                   unsigned char tag,
                   const void *value, long vlen)
{
    CHAR16 *ptr;

    ptr = ASCII_to_UCS2(value, (int)vlen);
    StrCat(tmpbuf, ptr);
    FreePool(ptr);

    return 0;
}


int
do_extensions(void *context, long state_index,
              unsigned char tag,
              const void *value, long vlen)
{
    Print(L"  Extensions:%s\n", tmpbuf);
    tmpbuf[0] = '\0';
    wrapno = 1;

    return 0;
}


int
do_extension_id(void *context, long state_index,
                unsigned char tag,
                const void *value, long vlen)
{
    enum OID oid; 
    CHAR16 buffer[60];
    int len = StrLen(tmpbuf);

    if (len > (90*wrapno)) {
        StrCat(tmpbuf, L"\n             ");
        wrapno++;
    }

    oid = Lookup_OID(value, vlen);
    Sprint_OID(value, vlen, buffer, sizeof(buffer));
    if (oid == OID_subjectKeyIdentifier)
        StrCat(tmpbuf, L" SubjectKeyIdentifier");
    else if (oid == OID_keyUsage)
        StrCat(tmpbuf, L" KeyUsage");
    else if (oid == OID_subjectAltName)
        StrCat(tmpbuf, L" SubjectAltName");
    else if (oid == OID_issuerAltName)
        StrCat(tmpbuf, L" IssuerAltName");
    else if (oid == OID_basicConstraints)
        StrCat(tmpbuf, L" BasicConstraints");
    else if (oid == OID_crlDistributionPoints)
        StrCat(tmpbuf, L" CrlDistributionPoints");
    else if (oid == OID_certAuthInfoAccess) 
        StrCat(tmpbuf, L" CertAuthInfoAccess");
    else if (oid == OID_certPolicies)
        StrCat(tmpbuf, L" CertPolicies");
    else if (oid == OID_authorityKeyIdentifier)
        StrCat(tmpbuf, L" AuthorityKeyIdentifier");
    else if (oid == OID_extKeyUsage)
        StrCat(tmpbuf, L" ExtKeyUsage");
    else if (oid == OID_msEnrollCerttypeExtension)
        StrCat(tmpbuf, L" msEnrollCertTypeExtension");
    else if (oid == OID_msCertsrvCAVersion)
        StrCat(tmpbuf, L" msCertsrvCAVersion");
    else if (oid == OID_msCertsrvPreviousCertHash)
        StrCat(tmpbuf, L" msCertsrvPreviousCertHash");
    else {
        StrCat(tmpbuf, L" (");
        StrCat(tmpbuf, buffer);
        StrCat(tmpbuf, L")");
    }

    return 0;
}


//
//  Yes, a hack but it works!  No support for generalizedDate
//  Otherwise we could have to deal with tm structures, etc.
//
char *
make_utc_date_string(char *s)
{
    static char buffer[50];
    char  *d;

    d = buffer;
    *d++ = '2';      /* year */
    *d++ = '0';
    *d++ = *s++;
    *d++ = *s++;
    *d++ = '-';
    *d++ = *s++;     /* month */
    *d++ = *s++;
    *d++ = '-';
    *d++ = *s++;     /* day */
    *d++ = *s++;
    *d++ = ' ';
    *d++ = *s++;     /* hour */
    *d++ = *s++;
    *d++ = ':';
    *d++ = *s++;     /* minute */
    *d++ = *s++;
    *d++ = ':';
    *d++ = *s++;     /* second */
    *d++ = *s;
    *d++ = ' ';
    *d++ = 'U';
    *d++ = 'T';
    *d++ = 'C';
     *d = '\0';

    return buffer;
}


int
do_validity_not_before(void *context, long state_index,
                       unsigned char tag,
                       const void *value, long vlen)
{
    CHAR16 *ptr;
    char *p;

    p = make_utc_date_string((char *)value);
    ptr = ASCII_to_UCS2(p, UTCDATE_LEN);
    Print(L"  Validity - Not Before: %s", ptr);
    FreePool(ptr);

    return 0;
}


int
do_validity_not_after(void *context, long state_index,
                      unsigned char tag,
                      const void *value, long vlen)
{
    CHAR16 *ptr;
    char *p;

    p = make_utc_date_string((char *)value);
    ptr = ASCII_to_UCS2(p, UTCDATE_LEN);
    Print(L"   Not After: %s\n", ptr);
    FreePool(ptr);

    return 0;
}


int
do_subject_public_key_info(void *context, long state_index,
             unsigned char tag,
             const void *value, long vlen)
{
    Print(L"  Subject Public Key Algorithm: %s\n", tmpbuf);
    tmpbuf[0] = '\0';

    return 0;
}


int
PrintCerts(UINT8 *data, UINTN len, EFI_HANDLE image, CHAR16 *name)
{
    EFI_SIGNATURE_LIST  *CertList = (EFI_SIGNATURE_LIST *)data;
    EFI_SIGNATURE_DATA  *Cert;
    EFI_GUID gX509 = EFI_CERT_X509_GUID;
    EFI_GUID gPKCS7 = EFI_CERT_PKCS7_GUID;
    EFI_GUID gRSA2048 = EFI_CERT_RSA2048_GUID;
    UINTN Index, count = 0, DataSize = len, CertCount;
    UINTN  buflen;
    CHAR16 *ext;
    int status = 0;

    while ((DataSize > 0) && (DataSize >= CertList->SignatureListSize)) {
        count++;
        CertCount = (CertList->SignatureListSize - CertList->SignatureHeaderSize) / CertList->SignatureSize;
        Cert = (EFI_SIGNATURE_DATA *) ((UINT8 *) CertList + sizeof (EFI_SIGNATURE_LIST) + CertList->SignatureHeaderSize);

        // should all be X509 but just in case ...
        if (CompareGuid(&CertList->SignatureType, &gX509) == 0)
            ext = L"X509";
        else if (CompareGuid(&CertList->SignatureType, &gPKCS7) == 0)
            ext = L"PKCS7";
        else if (CompareGuid(&CertList->SignatureType, &gRSA2048) == 0)
            ext = L"RSA2048";
        else 
            ext = L"Unknown";

        for (Index = 0; Index < CertCount; Index++) {
            if ( CertList->SignatureSize > 100 ) {
                Print(L"\nType: %s  (GUID: %g)\n", ext, &Cert->SignatureOwner);
                buflen  = CertList->SignatureSize-sizeof(EFI_GUID);
                status = asn1_ber_decoder(&x509_decoder, NULL, Cert->SignatureData, buflen);
            }
            Cert = (EFI_SIGNATURE_DATA *) ((UINT8 *) Cert + CertList->SignatureSize);
        }
        DataSize -= CertList->SignatureListSize;
        CertList = (EFI_SIGNATURE_LIST *) ((UINT8 *) CertList + CertList->SignatureListSize);
    }

    return status;
}


EFI_STATUS
get_variable(CHAR16 *var, UINT8 **data, UINTN *len, EFI_GUID owner)
{
    EFI_STATUS efi_status;

    *len = 0;

    efi_status = uefi_call_wrapper(RT->GetVariable, 5, var, &owner, NULL,
                       len, NULL);
    if (efi_status != EFI_BUFFER_TOO_SMALL)
        return efi_status;

    *data = AllocateZeroPool(*len);
    if (!data)
        return EFI_OUT_OF_RESOURCES;
    
    efi_status = uefi_call_wrapper(RT->GetVariable, 5, var, &owner, NULL,
                       len, *data);

    if (efi_status != EFI_SUCCESS) {
        FreePool(*data);
        *data = NULL;
    }
    return efi_status;
}


EFI_STATUS
OutputVariable(EFI_HANDLE image, CHAR16 *var, EFI_GUID owner) 
{
    EFI_STATUS status = EFI_SUCCESS;
    UINT8 *data;
    UINTN len;

    status = get_variable(var, &data, &len, owner);
    if (status == EFI_SUCCESS) {
        Print(L"\nVARIABLE: %s  (size: %d)\n", var, len);
        PrintCerts(data, len, image, var);
        FreePool(data);
    } else if (status == EFI_NOT_FOUND) {
        Print(L"Variable %s not found\n", var);
    } else 
        Print(L"ERROR: Failed to get variable %s. Code: %d\n", var, status);

    return status;
}

static void
Usage(void)
{
    Print(L"Usage: listcerts [ -pk | -kek | -db | -dbx]\n");
}


EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_GUID gSIGDB = EFI_SIG_DB_GUID;
    CHAR16 *variables[] = { L"PK", L"KEK", L"db", L"dbx" };
    EFI_GUID owners[] = { EFI_GLOBAL_VARIABLE, EFI_GLOBAL_VARIABLE, gSIGDB, gSIGDB };
    UINTN argc;
    CHAR16 **argv;
    int i;

    InitializeLib(image, systab);

    status = get_args(image, &argc, &argv);
    if (EFI_ERROR(status)) {
        Print(L"ERROR: Parsing command line arguments: %d\n", status);
        return status;
    }

    if (argc == 1) {
        for (i = 0; i < ARRAY_SIZE(owners); i++) {
            status = OutputVariable(image, variables[i], owners[i]);
        }
    } else if (argc == 2) {
        if (!StrCmp(argv[1], L"--help") ||
            !StrCmp(argv[1], L"-h") ||
            !StrCmp(argv[1], L"-?")) {
               Usage();
        } else if (!StrCmp(argv[1], L"-pk"))  {
            status = OutputVariable(image, variables[0], owners[0]);
        } else if (!StrCmp(argv[1], L"-kek"))  {
            status = OutputVariable(image, variables[1], owners[1]);
        } else if (!StrCmp(argv[1], L"-db"))  {
            status = OutputVariable(image, variables[2], owners[2]);
        } else if (!StrCmp(argv[1], L"-dbx"))  {
            status = OutputVariable(image, variables[3], owners[3]);
        } else {
            Usage();
        } 
    }

    return status;
}
