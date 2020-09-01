#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<stdint.h>
#include"vect.h"

vect*vect_create(size_t size,size_t count)
{
	vect*v=(vect*)malloc(sizeof(vect));
	size_t capacity=(count<VECT_MINSIZE)?VECT_MINSIZE:count;

	if(!v)return NULL;
	v->size=size;
	vect_realloc(v,capacity);
	//v->values=(int32_t*)malloc(capacity*4);
	if(!v->array)
	{
		puts("error: failed to allocate vect->values");
		free(v);
		return NULL;
	}

	v->count=count;
	v->capacity=capacity;

	return v;
}

void vect_delete(vect*v)
{
	if(v->array)
		free(v->array);
	if(v)
		free(v);
}

void vect_realloc(vect*v,size_t capacity)
{
	// Note: capacity should be in # elements, not # bytes!
	capacity=(capacity<VECT_MINSIZE)?VECT_MINSIZE:capacity;

	if(!v)return;
	if(!v->array)
		v->array=(int32_t*)malloc(capacity*4);
	else
		v->array=(int32_t*)realloc(v->array,capacity*4);
	v->capacity=capacity;
}

void vect_append(vect*v,void*value)
{
	if(!v)return;
	if(v->count+1>v->capacity)
		vect_realloc(v,v->count+VECT_MINSIZE);

	if(!v->array)return;
	//v->array[v->count++]=value;
	memcpy(v->array+v->count*v->size,value,v->size);
	++v->count;
}

//void vect_print(vect*v)
//{
	//for(int i=0;i<v->count;++i)
	//{
		//printf("%d",((int*)v->array)[i]);
		//if(i<v->count-1)
			//printf(", ");
	//}
	//puts("");
//}
