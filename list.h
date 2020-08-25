#pragma once

typedef struct node
{
	int value;
	struct node*next;
} node;

// Create node
node*node_create(node*head,int value);
// Append to end of list
node*node_append(node*head,int value);
// Print all nodes to stdout
void node_print(node*head);
// Print corresponding element in notes from node values
void node_print_note(node*head,char**notes);
// Traverse list, return length in nodes
int node_length(node*head);
