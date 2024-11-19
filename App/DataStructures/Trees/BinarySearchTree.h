
#ifndef BinarySearchTree_h
#define BinarySearchTree_h

#include "../Common/Node.h"

// MARK: DATA TYPES

struct BinarySearchTree
{
    /* PUBLIC MEMBER VARIABLES */
    // Head points to the top-most node in the tree.
    struct Node *head;
    
    /*PUBLIC MEMBER METHODS */
    // The compare function is specified by the user and is used to compare the data of two nodes.
    // It must take two void pointers as arguments and return 1, -1, or 0.
    int (*compare)(void *data_one, void *data_two);
    // The search function finds a node in the tree, returning its data or NULL if not found.
    void * (*search)(struct BinarySearchTree *tree, void *data);
    // The insert function adds a new node to the tree.  Since memory allocation is handled by the node constructor, the size of this node's data must be specified.
    void (*insert)(struct BinarySearchTree *tree, void *data, unsigned long size);
};

// MARK: CONSTRUCTORS
// The constructor takes the desired compare function as its only argument.
struct BinarySearchTree binary_search_tree_constructor(int (*compare)(void *data_one, void *data_two));
void binary_search_tree_destructor(struct BinarySearchTree tree);

// MARK: PUBLIC HELPER FUNCTIONS
int binary_search_tree_int_compare(void *data_one, void *data_two);
int binary_search_tree_float_compare(void *data_one, void *data_two);
int binary_search_tree_char_compare(void *data_one, void *data_two);
int binary_search_tree_str_compare(void *data_one, void *data_two);


#endif /* BinarySearchTree_h */