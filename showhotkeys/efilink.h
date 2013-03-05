//
//  efilink.h    
//
//  EFI double linked lists
//

#ifndef _EFI_LINK_H
#define _EFI_LINK_H


#ifndef EFI_NT_EMUL

//
// List entry - doubly linked list
//

typedef struct _LIST_ENTRY {
    struct _LIST_ENTRY  *ForwardLink;
    struct _LIST_ENTRY  *BackLink;
} LIST_ENTRY;

#endif 


LIST_ENTRY *
InitializeListHead ( LIST_ENTRY * );

LIST_ENTRY *
InsertHeadList ( LIST_ENTRY *ListHead, LIST_ENTRY *Entry);

LIST_ENTRY *
InsertTailList ( LIST_ENTRY *ListHead, LIST_ENTRY  *Entry); 

LIST_ENTRY *
GetFirstNode ( LIST_ENTRY  *List);

LIST_ENTRY *
GetNextNode ( LIST_ENTRY *List, LIST_ENTRY *Node);

LIST_ENTRY *
GetPreviousNode ( LIST_ENTRY *List, LIST_ENTRY *Node);

BOOLEAN
IsListEmpty ( LIST_ENTRY *ListHead);

BOOLEAN
IsNull ( LIST_ENTRY  *List, LIST_ENTRY *Node);

BOOLEAN
IsNodeAtEnd ( LIST_ENTRY *List, LIST_ENTRY *Node);

LIST_ENTRY *
SwapListEntries ( LIST_ENTRY *FirstEntry, LIST_ENTRY  *SecondEntry);

LIST_ENTRY *
RemoveEntryList ( LIST_ENTRY *Entry);

//
//  EFI_FIELD_OFFSET - returns the byte offset to a field within a structure
//

#define EFI_FIELD_OFFSET(TYPE,Field) ((UINTN)(&(((TYPE *) 0)->Field)))

//
//  CONTAINING_RECORD - returns a pointer to the structure
//      from one of it's elements.
//

#define _CR(Record, TYPE, Field)  \
    ((TYPE *) ( (CHAR8 *)(Record) - (CHAR8 *) &(((TYPE *) 0)->Field)))

#if EFI_DEBUG
    #define CR(Record, TYPE, Field, Sig)     \
        _CR(Record, TYPE, Field)->Signature != Sig ?        \
            (TYPE *) ASSERT_STRUCT(_CR(Record, TYPE, Field), Record) : \
            _CR(Record, TYPE, Field)
#else
    #define CR(Record, TYPE, Field, Signature)   \
        _CR(Record, TYPE, Field)                           
#endif


//
// A lock structure
//

typedef struct _FLOCK {
    EFI_TPL     Tpl;
    EFI_TPL     OwnerTpl;
    UINTN       Lock;
} FLOCK;

#endif

