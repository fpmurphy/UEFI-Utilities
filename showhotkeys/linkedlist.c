#include <efi.h>
#include <efilib.h>

LIST_ENTRY *
InitializeListHead (
   LIST_ENTRY                *ListHead
  )
{
  ListHead->ForwardLink = ListHead;
  ListHead->BackLink = ListHead;
  return ListHead;
}

LIST_ENTRY *
InsertHeadList (
  LIST_ENTRY                *ListHead,
  LIST_ENTRY                *Entry
  )
{
  Entry->ForwardLink = ListHead->ForwardLink;
  Entry->BackLink = ListHead;
  Entry->ForwardLink->BackLink = Entry;
  ListHead->ForwardLink = Entry;
  return ListHead;
}

LIST_ENTRY *
InsertTailList (
  LIST_ENTRY                *ListHead,
  LIST_ENTRY                *Entry
  )
{
  Entry->ForwardLink = ListHead;
  Entry->BackLink = ListHead->BackLink;
  Entry->BackLink->ForwardLink = Entry;
  ListHead->BackLink = Entry;
  return ListHead;
}

LIST_ENTRY *
GetFirstNode (
  LIST_ENTRY          *List
  )
{
  return List->ForwardLink;
}

LIST_ENTRY *
GetNextNode (
   LIST_ENTRY          *List,
   LIST_ENTRY          *Node
  )
{
  return Node->ForwardLink;
}

LIST_ENTRY *
GetPreviousNode (
   LIST_ENTRY          *List,
   LIST_ENTRY          *Node
  )
{
  return Node->BackLink;
}

BOOLEAN
IsListEmpty (
   LIST_ENTRY          *ListHead
  )
{
  return (BOOLEAN)(ListHead->ForwardLink == ListHead);
}

BOOLEAN
IsNull (
   LIST_ENTRY          *List,
   LIST_ENTRY          *Node
  )
{
  return (BOOLEAN)(Node == List);
}

BOOLEAN
IsNodeAtEnd (
  LIST_ENTRY          *List,
  LIST_ENTRY          *Node
  )
{
  return (BOOLEAN)(!IsNull (List, Node) && List->BackLink == Node);
}

LIST_ENTRY *
SwapListEntries (
  LIST_ENTRY                *FirstEntry,
  LIST_ENTRY                *SecondEntry
  )
{
  LIST_ENTRY                    *Ptr;

  if (FirstEntry == SecondEntry) {
    return SecondEntry;
  }

  //
  // Ptr is the node pointed to by FirstEntry->ForwardLink
  //
  Ptr = RemoveEntryList (FirstEntry);

  //
  // If FirstEntry immediately follows SecondEntry, FirstEntry will be placed
  // immediately in front of SecondEntry
  //
  if (Ptr->BackLink == SecondEntry) {
    return InsertTailList (SecondEntry, FirstEntry);
  }

  //
  // Ptr == SecondEntry means SecondEntry immediately follows FirstEntry,
  // then there are no further steps necessary
  //
  if (Ptr == InsertHeadList (SecondEntry, FirstEntry)) {
    return Ptr;
  }

  //
  // Move SecondEntry to the front of Ptr
  //
  RemoveEntryList (SecondEntry);
  InsertTailList (Ptr, SecondEntry);
  return SecondEntry;
}

LIST_ENTRY *
RemoveEntryList (
  LIST_ENTRY          *Entry
  )
{
  
  Entry->ForwardLink->BackLink = Entry->BackLink;
  Entry->BackLink->ForwardLink = Entry->ForwardLink;
  return Entry->ForwardLink;
}
