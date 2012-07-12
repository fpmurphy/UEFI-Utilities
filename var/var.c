//
//  Copyright (c) 2012  Finnbarr P. Murphy.   All rights reserved.
//
//  Manipulate nonauthenticated UEFI NV variables
//

#include <efi.h>
#include <efilib.h>

#define DEMO_GUID \
   (EFI_GUID) {0xd2b2c240, 0xfc2f, 0x4ba5, {0x94,0x2c,0xc8,0x1e,0xe3,0x0c,0x17,0xa6}}

#define EFI_SHELL_INTERFACE_GUID \
   (EFI_GUID) {0x47c7b223, 0xc42a, 0x11d2, {0x8e,0x57,0x00,0xa0,0xc9,0x69,0x72,0x3b}}

#define BUFSIZE 2048

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
    Print(L"Usage: var [-c|--create] name value\n");
    Print(L"       var [-d|--delete] name\n");
    Print(L"       var [-m|--modify] name newvalue\n");
    Print(L"       var [-s|--show] name\n");
}


EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_GUID Guid = DEMO_GUID;
    CHAR16 Data[BUFSIZE];
    UINTN DataSize = 0;
    UINTN argc;
    CHAR16 **argv;
    UINTN Attr = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;

    InitializeLib(image, systab);

    status = get_args(image, &argc, &argv);
    if (EFI_ERROR(status)) {
        Print(L"ERROR: Parsing command line arguments: %d\n", status);
        return status;
    }

    if (argc == 2 ) {
        if (!StrCmp(argv[1], L"--help") ||
            !StrCmp(argv[1], L"-h") ||
            !StrCmp(argv[1], L"-?")) {
            usage();
            return EFI_SUCCESS;
        }
    } else if (argc == 3 ) {
        if (!StrCmp(argv[1], L"--delete") ||
            !StrCmp(argv[1], L"-d")) {
                status = uefi_call_wrapper(RT->SetVariable, 5, 
                                           argv[2], &Guid, Attr, 0, (CHAR16 *)NULL);
                if (status != EFI_SUCCESS) 
                    Print(L"ERROR: SetVariable failed; %d\n", status);
                return status;
        } else if (!StrCmp(argv[1], L"--show") ||
            !StrCmp(argv[1], L"-s")) {
                ZeroMem(Data, BUFSIZE);
                DataSize = BUFSIZE;
                status = uefi_call_wrapper(RT->GetVariable, 5,
                                           argv[2], &Guid, Attr, &DataSize, &Data);
                if (status != EFI_SUCCESS) 
                    Print(L"ERROR: GetVariable failed: %d\n", status);
                else
                    Print(L"Variable: %s, value: %s, size: %d\n", argv[2], Data, DataSize/2);
                return status;
        }
    } else if (argc == 4 ) {
        if (!StrCmp(argv[1], L"--create") ||
            !StrCmp(argv[1], L"-c")) {
                ZeroMem(Data, BUFSIZE);
                StrCpy(Data, argv[3]);
                DataSize = StrLen(Data) * 2;

                status = uefi_call_wrapper(RT->SetVariable, 5, 
                                           argv[2], &Guid, Attr, DataSize, &Data);
                if (status != EFI_SUCCESS) 
                    Print(L"ERROR: SetVariable failed; %d\n", status);
                else 
                    Print(L"Created variable: %s, value: %s\n", argv[2], argv[3]);
                return status;
        } else if (!StrCmp(argv[1], L"--modify") ||
            !StrCmp(argv[1], L"-m")) {
                ZeroMem(Data, BUFSIZE);
                DataSize = BUFSIZE;
                status = uefi_call_wrapper(RT->GetVariable, 5,
                                           argv[2], &Guid, Attr, &DataSize, &Data);
                if (status != EFI_SUCCESS) {
                    Print(L"ERROR: Cannot modify variable: %s. GetVariable failed: %d\n", argv[2], status);
                    return status;
                } 

                ZeroMem(Data, BUFSIZE);
                StrCpy(Data, argv[3]);
                DataSize = StrLen(Data) * 2;
                status = uefi_call_wrapper(RT->SetVariable, 5, 
                                           argv[2], &Guid, Attr, DataSize, &Data);
                if (status != EFI_SUCCESS) 
                    Print(L"ERROR: SetVariable failed; %d\n", status);
                else 
                    Print(L"Modified variable: %s, new value: %s\n", argv[2], argv[3]);
                return status;
        }
    }

    usage();
    return status;
}
