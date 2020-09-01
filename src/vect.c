#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include"vect.h"

vect*vect_create(size_t size)
{
	vect*v=(vect*)malloc(sizeof(vect));
	size_t capacity=(size<VECT_MINSIZE)?VECT_MINSIZE:size;

	if(!v)return NULL;
	vect_realloc(v,capacity);
	//v->values=(int32_t*)malloc(capacity*4);
	if(!v->values)
	{
		puts("error: failed to allocate vect->values");
		free(v);
		return NULL;
	}

	v->size=size;
	v->capacity=capacity;

	return v;
}

void vect_delete(vect*v)
{
	if(v->values)
		free(v->values);
	if(v)
		free(v);
}

void vect_realloc(vect*v,size_t capacity)
{
	// Note: capacity should be in # elements, not # bytes!
	capacity=(capacity<VECT_MINSIZE)?VECT_MINSIZE:capacity;

	if(!v)return;
	if(!v->values)
		v->values=(int32_t*)malloc(capacity*4);
	else
		v->values=(int32_t*)realloc(v->values,capacity*4);
	v->capacity=capacity;
}

void vect_append(vect*v,int value)
{
	if(!v)return;
	if(v->size+1>v->capacity)
		vect_realloc(v,v->size+VECT_MINSIZE);
	if(!v->values)return;
	v->values[v->size++]=value;
}

void vect_print(vect*v)
{
	for(int i=0;i<v->size;++i)
	{
		printf("%d",v->values[i]);
		if(i<v->size-1)
			printf(", ");
	}
	puts("");
}
