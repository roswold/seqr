#pragma once
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#define VECT_MINSIZE 1024

// Struct to hold dynamic memory
typedef struct vect
{
	int32_t*values;
	size_t capacity;
	size_t size;
} vect;

// Create vector of size
vect*vect_create(size_t size);

// Free resources for vector
void vect_delete(vect*v);

// Push value onto end of vector
void vect_append(vect*v,int value);

// Internal use: reallocate data to larger size
void vect_realloc(vect*v,size_t newsize);

// Print vector elements
void vect_print(vect*v);
