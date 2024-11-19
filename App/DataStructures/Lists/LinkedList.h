#ifndef LinkedList_h
#define LinkedList_h

#include "../Common/Node.h"

// MARK: DATA TYPES

// LinkedLists are used to move between and manipulate related nodes in an organized fashion.
struct LinkedList
{
    /* PUBLIC MEMBER VARIABLES */
    // Head points to the first node in the chain.
    struct Node *head;
    // Length refers to the number of nodes in the chain.
    int length;
    
    /* PUBLIC MEMBER METHODS */
    // Insert adds new items to the chain at a specified location - this function creates the new nodes.
    void (*insert)(struct LinkedList *linked_list, int index, void *data, unsigned long size);
    // Remove deletes an item from the chain and handles the deallocation of memory.
    void (*remove)(struct LinkedList *linked_list, int index);
    // Retrieve allows data in the chain to be accessed
    void * (*retrieve)(struct LinkedList *linked_list, int index);
    // Sorting and searching the list (bubble sort).
    void (*sort)(struct LinkedList *linked_list, int (*compare)(void *a, void *b));
    // Binary search (requires the list be sorted).
    short (*search)(struct LinkedList *linked_list, void *query, int (*compare)(void *a, void *b));
};



// MARK: CONSTRUCTORS 
struct LinkedList linked_list_constructor(void);
void linked_list_destructor(struct LinkedList *linked_list);

#endif /* LinkedList_h */