//
//  Copyright (c) 2012  Finnbarr P. Murphy.   All rights reserved.
//
//  Display Secure Boot variables
//

#include <efi.h>
#include <efilib.h>

#define EFI_SHELL_INTERFACE_GUID \
   (EFI_GUID) {0x47c7b223, 0xc42a, 0x11d2, {0x8e,0x57,0x00,0xa0,0xc9,0x69,0x72,0x3b}}

typedef enum {
    ARG_NO_ATTRIB         = 0x0,
    ARG_IS_QUOTED         = 0x1,
    ARG_PARTIALLY_QUOTED  = 0x2,
    ARG_FIRST_HALF_QUOTED = 0x4,
    ARG_FIRST_CHAR_IS_ESC = 0x8
} EFI_SHELL_ARG_INFO_TYPES;

struct _EFI_SHELL_ARG_INFO {
    UINT32 Attributes;
} __attribute__((packed)) __attribute__((aligned (1)));
typedef struct _EFI_SHELL_ARG_INFO EFI_SHELL_ARG_INFO;

struct _EFI_SHELL_INTERFACE {
    EFI_HANDLE           ImageHandle;
    EFI_LOADED_IMAGE    *Info;
    CHAR16             **Argv;
    UINTN                Argc;
    CHAR16             **RedirArgv;
    UINTN                RedirArgc;
    EFI_FILE            *StdIn;
    EFI_FILE            *StdOut;
    EFI_FILE            *StdErr;
    EFI_SHELL_ARG_INFO  *ArgInfo;
    BOOLEAN              EchoOn;
} __attribute__((packed)) __attribute__((aligned (1)));
typedef struct _EFI_SHELL_INTERFACE EFI_SHELL_INTERFACE;



static EFI_STATUS
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


static void
usage(void)
{
    Print(L"Usage: showsb [-v|--verbose]\n");
}


EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_GUID Guid = EFI_GLOBAL_VARIABLE;
    UINT8 Data;
    UINTN DataSize = sizeof(Data);
    BOOLEAN Verbose = FALSE;
    UINTN argc;
    CHAR16 **argv;

    InitializeLib(image, systab);

    status = get_args(image, &argc, &argv);
    if (EFI_ERROR(status)) {
        Print(L"ERROR: Parsing command line arguments: %d\n", status);
       return status;
    }

    if (argc > 2) {
        usage();
        return EFI_INVALID_PARAMETER;
    } else if (argc == 2 ) {
        if (!StrCmp(argv[1], L"--help") ||
            !StrCmp(argv[1], L"-h") ||
            !StrCmp(argv[1], L"-?")) {
            usage();
            return EFI_SUCCESS;
        } else if (!StrCmp(argv[1], L"--verbose") ||
                   !StrCmp(argv[1], L"-v")) {
             Verbose = TRUE; 
        } else {
            usage();
            return EFI_INVALID_PARAMETER;
        }
    }

    status = uefi_call_wrapper(RT->GetVariable, 5, 
                               L"SecureBoot", &Guid, NULL, &DataSize, &Data);
    if (status != EFI_SUCCESS) {
        Print(L"ERROR: SecureBoot variable not found.\n");
        return status;
    }
    if (Verbose) {
       if (Data) {
           Print(L"Secure Boot: Enabled\n");
       } else {
           Print(L"Secure Boot: Disabled\n");
       }
    } else {
        Print(L"SecureBoot: %d\n", Data);
    }


    status = uefi_call_wrapper(RT->GetVariable, 5, 
                               L"SetupMode", &Guid, NULL, &DataSize, &Data);
    if (status != EFI_SUCCESS) {
        Print(L"ERROR: SetupMode variable not found.\n");
        return status;
    }
    if (Verbose) {
        if (Data) {
            Print(L"Secure Boot Setup Mode: Enabled\n");
        } else {
            Print(L"Secure Boot Setup Mode: Disabled\n");
        }
    } else {
        Print(L"SetupMode: %d\n", Data);
    }

    return status;
}
