//
//   Copyright (c) 2012   Finnbarr P. Murphy.  All rights reserved
//
//   See http://blog.fpmurphy.com/2012/07/problems-testing-uefi-apis-using-qemu-ovmf-and-gnu-efi.html
//


#include <efi.h>
#include <efilib.h>


EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
    EFI_STATUS rc = EFI_SUCCESS;
    UINT32 Attr;
    UINT64 MaxStoreSize = 0;
    UINT64 RemainStoreSize = 0;
    UINT64 MaxSize = 0;

    InitializeLib(image, systab);

    Attr = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_HARDWARE_ERROR_RECORD;

    rc = uefi_call_wrapper(RT->QueryVariableInfo, 4, 
                           Attr, &MaxStoreSize, &RemainStoreSize, &MaxSize);

    if (rc != EFI_SUCCESS) {
        Print(L"ERROR: Failed to get store sizes: %d\n", rc);
    } else {
        Print(L"Max Storage Size: %ld\n", MaxStoreSize);
        Print(L"Remaining Storage Size: %ld\n", RemainStoreSize);
        Print(L"Max Variable Size: %ld\n", MaxSize);
    }

    return rc;
}
