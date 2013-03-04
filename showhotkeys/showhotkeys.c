//
//  Copyright (c) 2013  Finnbarr P. Murphy.   All rights reserved.
//
//  Examine BootOptionSupport global variable and list boot hotkeys if available
//          Note - does not list contents of hotkeys.
//    

#include <efi.h>
#include <efilib.h>

// from TianoCore .../MdePkg/Include/Base.h
#define BASE_CR(Record, TYPE, Field)  ((TYPE *) ((CHAR8 *) (Record) - (CHAR8 *) &(((TYPE *) 0)->Field)))


#define VarBootOptionSupport      L"BootOptionSupport"

#define EFI_BOOT_OPTION_SUPPORT_KEY   0x00000001
#define EFI_BOOT_OPTION_SUPPORT_APP   0x00000002
#define EFI_BOOT_OPTION_SUPPORT_COUNT 0x00000300

#define EFI_SHELL_INTERFACE_GUID \
   (EFI_GUID) {0x47c7b223, 0xc42a, 0x11d2, {0x8e,0x57,0x00,0xa0,0xc9,0x69,0x72,0x3b}}

#define BUFSIZE 8 

typedef struct {
    LIST_ENTRY Link;
    CHAR16     Name[BUFSIZE];
} HOTKEY_LIST_ENTRY;

STATIC HOTKEY_LIST_ENTRY HotKeyList;

typedef union {
    struct {
        UINT32 Revision : 8;
        UINT32 ShiftPressed : 1;
        UINT32 ControlPressed : 1;
        UINT32 AltPressed : 1;
        UINT32 LogoPressed : 1;
        UINT32 MenuPressed : 1;
        UINT32 SysReqPressed : 1;
        UINT32 Reserved : 16;
        UINT32 InputKeyCount : 2;
    } Options;
    UINT32 PackedValue;
} EFI_BOOT_KEY_DATA;

typedef struct _EFI_KEY_OPTION {
    EFI_BOOT_KEY_DATA KeyData;
    UINT32 BootOptionCrc;
    UINT16 BootOption;
} EFI_KEY_OPTION;

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
    EFI_STATUS Status;
    EFI_SHELL_INTERFACE *shell;
    EFI_GUID gEfiShellInterfaceGuid = EFI_SHELL_INTERFACE_GUID;

    Status = uefi_call_wrapper(BS->OpenProtocol, 6,
                               image, &gEfiShellInterfaceGuid,
                               (VOID **)&shell, image, NULL,
                               EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(Status))
        return Status;

    *argc = shell->Argc;
    *argv = shell->Argv;

    Status = uefi_call_wrapper(BS->CloseProtocol, 4, image,
                               &gEfiShellInterfaceGuid, image, NULL);
    return Status;
}


BOOLEAN
IsHotKeyVariable (CHAR16 *Name, EFI_GUID *Guid)
{
    EFI_GUID GlobalVariableGuid = EFI_GLOBAL_VARIABLE;

    if (CompareGuid (Guid, &GlobalVariableGuid) ||
        (StrSize (Name) != sizeof (L"Key####")) ||
        (StrnCmp (Name, L"Key", 3) != 0)) {
        return FALSE;
    }

    return TRUE;
}


INT16
CompareOptionNumbers(CHAR16 *Name1, CHAR16 *Name2)
{
    UINT16  OptionNumber1 = 0;
    UINT16  OptionNumber2 = 0;
    UINTN   Index;

    for (Index = 3; Index < 7; Index++) {
       if ((Name1[Index] >= L'0') && (Name1[Index] <= L'9')) {
           OptionNumber1 = OptionNumber1 * 10 + Name1[Index] - L'0';
       } else if ((Name1[Index] >= L'A') && (Name1[Index] <= L'F')) {
           OptionNumber1 = OptionNumber1 * 10 + Name1[Index] - L'A';
       }
    }

    for (Index = 3; Index < 7; Index++) {
       if ((Name2[Index] >= L'0') && (Name2[Index] <= L'9')) {
           OptionNumber2 = OptionNumber2 * 10 + Name2[Index] - L'0';
       } else if ((Name2[Index] >= L'A') && (Name2[Index] <= L'F')) {
           OptionNumber2 = OptionNumber2 * 10 + Name2[Index] - L'A';
       }
    }

    if (OptionNumber1 > OptionNumber2)
        return 1;
    if (OptionNumber1 < OptionNumber2)
        return -1;
    return 0;
}


static void
Usage(void)
{
    Print(L"Usage: showhotkeys [-v | --verbose] [-n | --nosort ]\n");
}


EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_GUID Guid = EFI_GLOBAL_VARIABLE;
    EFI_GUID curGuid= NullGuid;
    UINTN Attr = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
    UINTN DataValue = 0;
    UINTN DataSize = 4;
    UINTN argc;
    UINTN Size, CurSize;
    CHAR16 **argv;
    CHAR16 *Name, *val;
    HOTKEY_LIST_ENTRY *HotKeyVar;
    HOTKEY_LIST_ENTRY *Node, *PrevNode;
    int Result, Verbose=0, NoSort=0;

    InitializeLib(image, systab);

    Status = get_args(image, &argc, &argv);
    if (EFI_ERROR(Status)) {
        Print(L"ERROR: Parsing command line arguments: %d\n", Status);
        return Status;
    }

    if (argc == 2) {
        if (!StrCmp(argv[1], L"--help") ||
            !StrCmp(argv[1], L"-h") ||
            !StrCmp(argv[1], L"-?")) {
            Usage();
            return Status;
        } else if (!StrCmp(argv[1], L"--verbose") ||
            !StrCmp(argv[1], L"-v")) {
                Verbose=1;
        } else if (!StrCmp(argv[1], L"--nosort") ||
            !StrCmp(argv[1], L"-n")) {
                NoSort=1;
        } 
    }
    if (argc == 3) {
        if (!StrCmp(argv[1], L"--verbose") ||
            !StrCmp(argv[1], L"-v")        ||
            !StrCmp(argv[2], L"--verbose") ||
            !StrCmp(argv[2], L"-v")) {
                Verbose=1;
        }
        if (!StrCmp(argv[1], L"--nosort") ||
            !StrCmp(argv[1], L"-n")       ||
            !StrCmp(argv[2], L"--nosort") ||
            !StrCmp(argv[2], L"-n")) {
                NoSort=1;
        }
    }

    Status = uefi_call_wrapper(RT->GetVariable, 5,
                  VarBootOptionSupport, &Guid, Attr, &DataSize, &DataValue);
    if (Status != EFI_SUCCESS)  {
         if (Status != EFI_NOT_FOUND) 
             Print(L"ERROR: GetVariable failed: %d\n", Status);
         return Status;
    } else {
         // Print(L"%04x\n", DataValue); 
         if (DataValue && EFI_BOOT_OPTION_SUPPORT_KEY) {
            if (Verbose == 1) 
                Print(L"Boot manager supports application launching using keys\n");
         } else {
            if (Verbose == 1) 
                Print(L"Boot manager does not support application launching using keys\n");
            return Status;
         }
    }

    InitializeListHead(&HotKeyList.Link);
    CurSize = BUFSIZE;
    Name = AllocateZeroPool(CurSize);
 
    // loop through all NVRAM variables and find any hotkeys
    while (TRUE) {
        Size = CurSize;
        Status = uefi_call_wrapper(RT->GetNextVariableName, 3, &Size, Name, &curGuid);
        if (Status ==  EFI_NOT_FOUND)
            break;
        if (Status == EFI_BUFFER_TOO_SMALL) {
            Name = ReallocatePool(Name, CurSize, Size);
            CurSize = Size;
            Status = uefi_call_wrapper(RT->GetNextVariableName, 3, &Size, Name, &curGuid);
        }
        if (Status != EFI_SUCCESS) {
            Print(L"ERROR: GetNextVariableName failed: %d\n", Status);
            return Status;
        }
        if (IsHotKeyVariable (Name, &curGuid)) {
            HotKeyVar = AllocateZeroPool(sizeof(HOTKEY_LIST_ENTRY));
            if (HotKeyVar == NULL) {
                Print(L"ERROR: Out of memory resources\n");
                return EFI_OUT_OF_RESOURCES;
            }
            StrCpy(HotKeyVar->Name, Name);

            // place new hotkey entry into correct position in sorted list if sort enables
            InsertHeadList(&HotKeyList.Link, &HotKeyVar->Link);
            if (NoSort == 0) {
                for (Node = (HOTKEY_LIST_ENTRY *)GetFirstNode (&HotKeyList.Link),
                     PrevNode = (HOTKEY_LIST_ENTRY *)GetFirstNode (&HotKeyList.Link);
                     !IsNull (&HotKeyList.Link, &Node->Link);
                     Node = (HOTKEY_LIST_ENTRY *)GetNextNode (&HotKeyList.Link, &Node->Link)) 
                {
                    Result = CompareOptionNumbers(PrevNode->Name, Node->Name); 
                    if (Result > 0) { 
                        Node = (HOTKEY_LIST_ENTRY *) SwapListEntries (&PrevNode->Link, &Node->Link);
                    } else if (Result < 0) {
                        break;
                    }
                }
            }
        }
    }

    FreePool(Name);

    // output the (sorted) list of hotkeys
    while (!IsListEmpty(&HotKeyList.Link)) {
        Node = (HOTKEY_LIST_ENTRY *)GetFirstNode(&HotKeyList.Link);
        HotKeyVar = BASE_CR(Node, HOTKEY_LIST_ENTRY, Link);
        Print(L"%s\n", HotKeyVar->Name);
        RemoveEntryList(&Node->Link);
        FreePool(Node);
    }

    return Status;
}
