//
//  Copyright (c) 2012  Finnbarr P. Murphy.   All rights reserved.
//
//  List and/or remove UEFI crash variables 
//


#include <efi.h>
#include <efilib.h>
#include <efilink.h>

// from TianoCore .../MdePkg/Include/Base.h
#define BASE_CR(Record, TYPE, Field)  ((TYPE *) ((CHAR8 *) (Record) - (CHAR8 *) &(((TYPE *) 0)->Field)))

#define EFI_SHELL_INTERFACE_GUID \
   (EFI_GUID) {0x47c7b223, 0xc42a, 0x11d2, {0x8e,0x57,0x00,0xa0,0xc9,0x69,0x72,0x3b}}

#define LINUX_EFI_CRASH_GUID \
   (EFI_GUID) { 0xcfc8fc79, 0xbe2e, 0x4ddc, { 0x97, 0xf0, 0x9f, 0x98, 0xbf, 0xe2, 0x98, 0xa0 } }

#define PSTORE_EFI_ATTRIBUTES \
   (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS)

#define BUFSIZE 256

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

typedef struct {
    LIST_ENTRY Link;
    CHAR16     Name[BUFSIZE];
} CRASH_VAR;

STATIC LIST_ENTRY CrashVarList;


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


EFI_STATUS
ClearCrashVars(mode)
{
    EFI_GUID CrashGuid = LINUX_EFI_CRASH_GUID;
    EFI_GUID curGuid= NullGuid;
    EFI_STATUS status = EFI_SUCCESS;
    CHAR16 Name[BUFSIZE], *val;
    LIST_ENTRY *Entry;
    CRASH_VAR *CrashVar;
    UINTN size;

    InitializeListHead(&CrashVarList);
    ZeroMem(Name, BUFSIZE);

    // build linked list of crash variables
    while (1) {
        size = sizeof(Name);
        status = uefi_call_wrapper(RT->GetNextVariableName, 3, &size, Name, &curGuid);
        if (status != EFI_SUCCESS) {
            if (status ==  EFI_NOT_FOUND)
                break; 
            Print(L"ERROR: GetNextVariableName failed: %d\n", status);
            return status; 
        }
        val = LibGetVariable(Name, &curGuid);
        if (!CompareGuid( &curGuid, &CrashGuid)) {
            CrashVar = AllocateZeroPool(sizeof(CRASH_VAR));
            if (CrashVar == NULL) {
                Print(L"ERROR: Out of resources\n");
                return EFI_OUT_OF_RESOURCES;
            }
            StrCpy(CrashVar->Name, Name);
            InsertTailList(&CrashVarList, &CrashVar->Link);
        }
        FreePool(val);
    }
    
    while (!IsListEmpty(&CrashVarList)) {
        Entry = CrashVarList.Flink;
        CrashVar = BASE_CR(Entry, CRASH_VAR, Link);
        if (mode == 1 || mode == 3) { 
            Print(L"%s\n", CrashVar->Name); 
        }
        if (mode == 2 || mode == 3) { 
            status = uefi_call_wrapper(RT->SetVariable, 5, 
                     &(CrashVar->Name), &CrashGuid, PSTORE_EFI_ATTRIBUTES, 0, NULL);
            if (status != EFI_SUCCESS) {
                Print(L"ERROR: SetVariable failed: %d\n", status);
            }
        }
        RemoveEntryList(Entry);
        FreePool(CrashVar);
    }

    return status;
}


static void
Usage(void)
{
    Print(L"Usage: clearcrashvars [-v | --verbose] [-d | --delete]\n");
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
            Usage();
    } else if (argc == 2) {
        if (!StrCmp(argv[1], L"--help") ||
            !StrCmp(argv[1], L"-h") ||
            !StrCmp(argv[1], L"-?")) {
            Usage();
        } else if (!StrCmp(argv[1], L"--verbose") ||
            !StrCmp(argv[1], L"-v")) {
                status = ClearCrashVars(1);
        } else if (!StrCmp(argv[1], L"--delete") ||
            !StrCmp(argv[1], L"-d")) {
                status = ClearCrashVars(2);
        } else
            status = 1; 
    } else if (argc == 3) {
        if ((!StrCmp(argv[1], L"--delete") || !StrCmp(argv[1], L"-d")) && 
            (!StrCmp(argv[2], L"--verbose") || !StrCmp(argv[2], L"-v"))) { 
                status = ClearCrashVars(3);
        } else if ((!StrCmp(argv[2], L"--delete") || !StrCmp(argv[2], L"-d")) && 
            (!StrCmp(argv[1], L"--verbose") || !StrCmp(argv[1], L"-v"))) { 
                status = ClearCrashVars(3);
        } else
            status = 1;
    }

    return status;
}



