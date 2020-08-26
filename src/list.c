#include"list.h"
#include<stdlib.h>
#include<stdio.h>

node*node_create(node*head,int value)
{
	node*n=malloc(sizeof(node));
	n->next=NULL;
	n->value=value;

	if(head)
		head->next=n;

	return n;
}

node*list_append(node*head,int value)
{
	while(head->next)head=head->next;
	return head->next=node_create(head,value);
}

void list_print(node*head)
{
	while(head)
	{
		printf("%d",head->value);
		head=head->next;
		if(head)printf(", ");
	}
	puts("");
}

void list_print_note(node*head,char**notes)
{
	while(head)
	{
		printf("%s",notes[head->value]);
		head=head->next;
		if(head)printf(", ");
	}
	puts("");
}

int list_length(node*head)
{
	int len=0;
	while(head)
	{
		++len;
		head=head->next;
	}
	return len;
}
