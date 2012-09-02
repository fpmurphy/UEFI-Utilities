//
//   Copyright (c) 2012  Finnbarr P. Murphy   All rights reserved.
//

#include <efi.h>
#include <efilib.h>

#define PAGE_SIZE 4096

const CHAR16 *memory_types[] = {
    L"EfiReservedMemoryType",
    L"EfiLoaderCode",
    L"EfiLoaderData",
    L"EfiBootServicesCode",
    L"EfiBootServicesData",
    L"EfiRuntimeServicesCode",
    L"EfiRuntimeServicesData",
    L"EfiConventionalMemory",
    L"EfiUnusableMemory",
    L"EfiACPIReclaimMemory",
    L"EfiACPIMemoryNVS",
    L"EfiMemoryMappedIO",
    L"EfiMemoryMappedIOPortSpace",
    L"EfiPalCode",
};


const CHAR16 *
memory_type_to_str(UINT32 type)
{
    if (type > sizeof(memory_types)/sizeof(CHAR16 *))
        return L"Unknown";

    return memory_types[type];
}

                 
EFI_STATUS
memory_map(EFI_MEMORY_DESCRIPTOR **map_buf, UINTN *map_size,
           UINTN *map_key, UINTN *desc_size, UINT32 *desc_version)
{
    EFI_STATUS err = EFI_SUCCESS;

    *map_size = sizeof(**map_buf) * 31;

get_map:
    *map_size += sizeof(**map_buf);

    err = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, *map_size, (void **)map_buf);
    if (err != EFI_SUCCESS) {
        Print(L"ERROR: Failed to allocate pool for memory map");
        return err;
    }

    err = uefi_call_wrapper(BS->GetMemoryMap, 5, map_size, *map_buf, map_key, desc_size, desc_version);
    if (err != EFI_SUCCESS) {
        if (err == EFI_BUFFER_TOO_SMALL) {
            uefi_call_wrapper(BS->FreePool, 1, (void *)*map_buf);
            goto get_map;
        }
        Print(L"ERROR: Failed to get memory map");
    }
    return err;
}


EFI_STATUS 
print_memory_map(void)
{
    EFI_MEMORY_DESCRIPTOR *buf;
    UINTN desc_size;
    UINT32 desc_version;
    UINTN size, map_key, mapping_size;
    EFI_MEMORY_DESCRIPTOR *desc;
    EFI_STATUS err = EFI_SUCCESS;
    int i = 0;

    err = memory_map(&buf, &size, &map_key, &desc_size, &desc_version);
    if (err != EFI_SUCCESS)
        return err;

    Print(L"Memory Map Size: %d\n", size);
    Print(L"Map Key: %d\n", map_key);
    Print(L"Descriptor Version: %d\n", desc_version);
    Print(L"Descriptor Size: %d\n\n", desc_size);

    desc = buf;
    while ((void *)desc < (void *)buf + size) {
        mapping_size = desc->NumberOfPages * PAGE_SIZE;

        Print(L"[#%02d] Type: %s  Attr: 0x%x\n", i, memory_type_to_str(desc->Type), desc->Attribute);
        Print(L"      Phys: %016llx-%016llx\n", desc->PhysicalStart, desc->PhysicalStart + mapping_size);
        Print(L"      Virt: %016llx-%016llx\n\n", desc->VirtualStart, desc->VirtualStart + mapping_size);

        desc = (void *)desc + desc_size;
        i++;
    }

    uefi_call_wrapper(BS->FreePool, 1, buf);
    return err;
}


EFI_STATUS
efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
    InitializeLib(image, systab);

    print_memory_map();

    return EFI_SUCCESS;
}
