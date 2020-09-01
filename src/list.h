#pragma once
#include<stdlib.h>

typedef struct node
{
	int value;
	struct node*next;
} node;

// Create node
#define list_create(x) node_create(NULL,x)
node*node_create(node*head,int value);

// Append to end of list
node*list_append(node*head,int value);

// Print all nodes to stdout
void list_print(node*head);

// Print corresponding element in notes from node values
void list_print_note(node*head,char**notes);

// Traverse list, return length in nodes
int list_length(node*head);

// Delete/free list nodes
void list_delete(node*head);
