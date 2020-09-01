#pragma once
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<byteswap.h>
#define VECT_MINSIZE 1024

// Struct to hold dynamic memory
typedef struct vect
{
	void*array;
	size_t capacity;
	size_t count;
	size_t size;
} vect;

// Create vector of size
vect*vect_create(size_t size,size_t count);

// Free resources for vector
void vect_delete(vect*v);

// Push value onto end of vector
void vect_append(vect*v,void*value);
#define vect_push(v,val,type) \
	do{ \
		type x=val; \
		vect_append(v,&x); \
	}while(0)

// Internal use: reallocate data to larger size
void vect_realloc(vect*v,size_t newcount);

// Print vector elements
// v: vect pointer
// fmt: printf format specifier for type
// type: e.g., int, float, etc.
#define vect_print(v,fmt,type) \
	do \
	{ \
		for(int i=0;i<v->count;++i) \
		{ \
			printf(fmt,((type*)v->array)[i]); \
			if(i<v->count-1) \
				printf(", "); \
		} \
		puts(""); \
	} while(0)
