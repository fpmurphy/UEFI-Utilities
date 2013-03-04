typedef struct _LIST_ENTRY LIST_ENTRY;

struct _LIST_ENTRY {
    LIST_ENTRY *ForwardLink;
    LIST_ENTRY *BackLink;
};

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
