//
//  Copyright (c) 2012  Finnbarr P. Murphy.   All rights reserved.
//
//  Display  Firmware Information Vendor, Version and Release Date via SMBIOS 
//
//  Any source code included from EDK2 is copyright Intel Corporation
//  
//  Licence: BSD License
//


#include <efi.h>          // Install GNU_EFI package to get these headers
#define GUID EFI_GUID     // hack for SmBios.h
#include "SmBios.h"       // from EDK2. GNU_EFI libsmbios.h is defective 
#include <efilib.h>

#define BUFSIZE 64

#define INVALID_HANDLE  (UINT16) (-1)
#define DMI_INVALID_HANDLE  0x83
#define DMI_SUCCESS 0x00

STATIC SMBIOS_TABLE_ENTRY_POINT *mSmbiosTable = NULL;
STATIC SMBIOS_STRUCTURE_POINTER m_SmbiosStruct;
STATIC SMBIOS_STRUCTURE_POINTER *mSmbiosStruct = &m_SmbiosStruct;


int 
Strlen(const char *str)
{
    const char *s;

    for (s = str; *s; ++s)
        ;
    return (s - str);
}


//
// Modified from EDK2 source code.  Copyright Intel Corporation.
//
CHAR8*
LibGetSmbiosString ( SMBIOS_STRUCTURE_POINTER *Smbios,
                     UINT16 StringNumber)
{
    UINT16  Index;
    CHAR8   *String;

    ASSERT (Smbios != NULL);

    String = (CHAR8 *) (Smbios->Raw + Smbios->Hdr->Length);

    for (Index = 1; Index <= StringNumber; Index++) {
        if (StringNumber == Index) {
            return String;
        }
        for (; *String != 0; String++)
             ;
        String++;

        if (*String == 0) {
            Smbios->Raw = (UINT8 *)++String;
            return NULL;
        }
    }

    return NULL;
}


//
// Modified from EDK2 source code. Copyright Intel Corporation.
//
EFI_STATUS
LibGetSmbiosStructure ( UINT16 *Handle,
                        UINT8  **Buffer,
                        UINT16 *Length)
{
     SMBIOS_STRUCTURE_POINTER  Smbios;
     SMBIOS_STRUCTURE_POINTER  SmbiosEnd;
     UINT8 *Raw;

     if (*Handle == INVALID_HANDLE) {
          *Handle =  mSmbiosStruct->Hdr->Handle;
          return DMI_INVALID_HANDLE;
     }

     if ((Buffer == NULL) || (Length == NULL)) {
          Print(L"Invalid handle\n");
          return DMI_INVALID_HANDLE;
     }

     *Length = 0;
     Smbios.Hdr = mSmbiosStruct->Hdr;
     SmbiosEnd.Raw = Smbios.Raw + mSmbiosTable->TableLength;
     while (Smbios.Raw < SmbiosEnd.Raw) {
          if (Smbios.Hdr->Handle == *Handle) {
             Raw = Smbios.Raw;
             LibGetSmbiosString (&Smbios, (UINT16) (-1));
             *Length = (UINT16) (Smbios.Raw - Raw);
             *Buffer = Raw;
             if (Smbios.Raw < SmbiosEnd.Raw) {
                  *Handle = Smbios.Hdr->Handle;
             } else {
                  *Handle = INVALID_HANDLE;
             }
             return DMI_SUCCESS;
         }
         LibGetSmbiosString (&Smbios, (UINT16) (-1));
    }

    *Handle = INVALID_HANDLE;

    return DMI_INVALID_HANDLE;
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



EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_GUID Guid = EFI_GLOBAL_VARIABLE;
    CHAR16 *ptr;
    CHAR8  *str;
    UINT16  Handle;
    UINTN   Index;
    UINT16  Length;
    UINT8   *Buffer;
    SMBIOS_STRUCTURE_POINTER SmbiosStruct;
    SMBIOS_TABLE_TYPE0 *SmbiosType0;

    InitializeLib(image, systab);

    status = LibGetSystemConfigurationTable(&SMBIOSTableGuid, (VOID**)&mSmbiosTable);
    if ((status != EFI_SUCCESS || mSmbiosTable == NULL) || 
        (CompareMem (mSmbiosTable->AnchorString, "_SM_", 4) != 0)) {
        Print(L"ERROR: SMBIOS table not found.\n");
        return status;
    }

    mSmbiosStruct->Raw  = (UINT8 *) (UINTN) (mSmbiosTable->TableAddress);

    Print(L"SMBIOS Ver: %x.%x  Rev: %x  Table Count: %d\n", 
            mSmbiosTable->MajorVersion, 
            mSmbiosTable->MinorVersion, 
            mSmbiosTable->EntryPointRevision,
            mSmbiosTable->NumberOfSmbiosStructures); 

    Handle  = INVALID_HANDLE;
    LibGetSmbiosStructure (&Handle, NULL, NULL);

    // loop though the tables looking for a type 0 table.
    for (Index = 0; Index < mSmbiosTable->NumberOfSmbiosStructures; Index++) {
        if (Handle == INVALID_HANDLE) {
            break;
        }
        if (LibGetSmbiosStructure (&Handle, &Buffer, &Length) != DMI_SUCCESS) {
            break;
        }
        SmbiosStruct.Raw = Buffer;
        if (SmbiosStruct.Hdr->Type == 0) {     // Type 0 - BIOS

             /* vendor string */
             str = LibGetSmbiosString(&SmbiosStruct, 1);
             ptr = ASCII_to_UCS2(str, Strlen(str));
             Print(L"Firmware Vendor: %s\n", ptr);    
             FreePool(ptr);
                       
             /* version string */
             str = LibGetSmbiosString(&SmbiosStruct, 2);
             ptr = ASCII_to_UCS2(str, Strlen(str));
             Print(L"Firmware Version: %s\n", ptr);
             FreePool(ptr);

             /* release string */
             str = LibGetSmbiosString(&SmbiosStruct, 3);
             ptr = ASCII_to_UCS2(str, Strlen(str));
             Print(L"Firmware Release: %s\n", ptr);
             FreePool(ptr);

             break;
        }
    }

    return status;
}
