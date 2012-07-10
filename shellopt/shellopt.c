//
//   Copyright (c) 2012  Finnbarr P. Murphy.   All rights reserved.
//
//   Set or clear ShellOpt global variable
//

#include <efi.h>
#include <efilib.h>

#define EFI_SHELL_INTERFACE_GUID \
   (EFI_GUID) {0x47c7b223, 0xc42a, 0x11d2, {0x8e,0x57,0x00,0xa0,0xc9,0x69,0x72,0x3b}}

// There is some confusion here - the EDK2 shell source code uses SHELL_VARIABLE_GUID whereas 
// the Shell 2.0 specification mandates EFI_GLOBAL_VARIABLE.  See Section 3.2, Table 2.
#define EFI_GLOBAL_VARIABLE \
   (EFI_GUID) {0x8BE4DF61, 0x93CA, 0x11d2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C }}

#define SHELL_VARIABLE_GUID \
   (EFI_GUID) {0x158def5a, 0xf656, 0x419c, {0xb0,0x27,0x7a,0x31,0x92,0xc0,0x79,0xd2}}

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

static struct {
    CHAR16 *name;
    UINTN len;
} shellopts[] = {
    { L"-nomap", 6 },
    { L"-nostartup", 10 },
    { L"-startup", 8 },
    { L"-noversion", 10 },
    { L"-noconsolein", 12 },
    { L"-noconsoleout", 13 },
    { L"-nointerrupt", 12 },
    { L"-delay", 6 },
    { NULL, 0 }
};


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


static BOOLEAN 
is_number(CHAR16* str)
{
    CHAR16 *s = str;

    while (*s) {
        if (*s  < L'0' || *s > L'9')
            return FALSE;
        s++;
    }

    return TRUE;
}


static void
usage(void)
{
    Print(L"Usage: shellopt -s|--set [-nomap] [-nostartup | -startup] [-noversion] \\\n");
    Print(L"                         [-noconsolein] [-noconsoleout] [-nointerrupt] \\\n");
    Print(L"                         [-delay[:n]]\n");
    Print(L"       shellopt -c|--clear\n");
}


EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
    EFI_STATUS status;
    EFI_GUID gEfiGlobalVariableGuid = SHELL_VARIABLE_GUID;
    CHAR16 buffer[BUFSIZE];
    CHAR16 *bufptr = buffer;
    UINTN bufsize = 0;
    UINTN argc;
    CHAR16 **argv;
    int i, j, Match;

    InitializeLib(image, systab);

    status = get_args(image, &argc, &argv);
    if (EFI_ERROR(status)) {
        Print(L"ERROR: Parsing command line arguments: %d\n", status);
       return status;
    }

    if (argc < 2) {
        usage();
        return EFI_SUCCESS;
    } else if (argc == 2) {
        if (!StrCmp(argv[1], L"--help") ||
            !StrCmp(argv[1], L"/help") ||
            !StrCmp(argv[1], L"-h") ||
            !StrCmp(argv[1], L"-?")) {
            usage();
            return EFI_SUCCESS;
        }
        if (!StrCmp(argv[1], L"-c") ||
            !StrCmp(argv[1], L"--clear")) {
            bufsize = 0;
            bufptr  = (CHAR16 *)NULL;          
        } 
    } else if (argc > 2 && (!StrCmp(argv[1], L"-s") || !StrCmp(argv[1], L"--set"))) {
        ZeroMem(bufptr, BUFSIZE);
        for (i = 2; i < argc; i++) {
            StrLwr(argv[i]);
            Match = 0;
            for (j = 0; shellopts[j].name != NULL; j++) {
                if (!StrCmp(shellopts[j].name, argv[i])) {
                    StrCat(buffer, argv[i]);
                    StrCat(buffer, L" ");
                    Match = 1;
                    // handle delay number if one entered on command line
                    if (!StrCmp(L"-delay", argv[i]) && i < argc && is_number(argv[i+1])) {
                        i++;
                        StrCat(buffer, argv[i]);
                        StrCat(buffer, L" ");
                    }    
                    break;
                }  
             }
             if (!Match) {
                 Print(L"ERROR: Invalid shell startup option: %s\n", argv[i]);
                 return EFI_INVALID_PARAMETER;
             }
         }
         // hack - but it works!
         bufsize = StrLen(buffer) * 2;
     } else {
         usage();
         return EFI_INVALID_PARAMETER;
     }

     status = uefi_call_wrapper(RT->SetVariable, 5, L"ShellOpt", &gEfiGlobalVariableGuid,
                                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                                bufsize, bufptr);
     if (status != EFI_SUCCESS && status != EFI_NOT_FOUND)
         Print(L"ERROR: SetVariable: %d\n", status);

     return status;
}
