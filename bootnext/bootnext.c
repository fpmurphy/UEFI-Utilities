//
//  Copyright (c) 2012  Finnbarr P. Murphy.   All rights reserved.
//
//  Manipulate BootNext global variable
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


// 
// check that boot argument is of the form hexadecimal #### 
//
int 
check_arg(CHAR16 *str)
{
    int count = 0;
    CHAR16 *p = str;
    char c;

    while (*p) {
       c = (char)*p;
       if  (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
           return 1;
       }
       count++;
       p++;
    } 

    if (count != 4) 
       return 1;

    return 0;
}


static void
usage(void)
{
    Print(L"Usage: bootnext [-d | --delete | ####]\n");
}


EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_GUID Guid = EFI_GLOBAL_VARIABLE;
    UINTN Attr = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
    UINTN BootValue;
    UINTN DataSize = 0;
    UINTN argc;
    CHAR16 **argv;

    InitializeLib(image, systab);

    status = get_args(image, &argc, &argv);
    if (EFI_ERROR(status)) {
        Print(L"ERROR: Parsing command line arguments: %d\n", status);
        return status;
    }

    if (argc == 1) {
        DataSize = 2;
        status = uefi_call_wrapper(RT->GetVariable, 5,
                      VarBootNext, &Guid, Attr, &DataSize, &BootValue);
        if (status != EFI_SUCCESS)  {
             if (status != EFI_NOT_FOUND) 
                 Print(L"ERROR: GetVariable failed: %d\n", status);
        } else
             Print(L"%04x\n", BootValue);
    } else if (argc == 2) {
        if (!StrCmp(argv[1], L"--help") ||
            !StrCmp(argv[1], L"-h") ||
            !StrCmp(argv[1], L"-?")) {
            usage();
            return EFI_SUCCESS;
        }
        if (!StrCmp(argv[1], L"--delete") ||
            !StrCmp(argv[1], L"-d")) {
            status = uefi_call_wrapper(RT->SetVariable, 5,
                                       VarBootNext, &Guid, Attr, 0, (CHAR16 *)NULL);
            if (status != EFI_SUCCESS && status != EFI_NOT_FOUND) 
                Print(L"ERROR: SetVariable failed; %d\n", status);
            return status;
        } 

        // test argument is of form hexadecimal XXXX
        if (check_arg(argv[1])) {
            Print(L"Invalid argument: %s. Must be hexadecimal #### or -d or --delete.\n", argv[1]);
            return 1;  
        } 

        BootValue = xtoi(argv[1]); 
        DataSize = 2;
        status = uefi_call_wrapper(RT->SetVariable, 5, 
                                   VarBootNext, &Guid, Attr, DataSize, &BootValue);
        if (status != EFI_SUCCESS) 
            Print(L"ERROR: SetVariable failed; %d\n", status);
    }

    return status;
}
