#include <stdlib.h> /*realloc, malloc, free*/
#include <string.h> /*memcpy*/
#include <assert.h> /*assert*/

#include "vector.h"

#define GROWTH_RATE 2
#define LAST_ELEMENT_IN_ARRAY (char *)(vector->vector_array) + ((vector->size) * vector->element_size)

typedef enum {SUCCESS, FAILURE} return_status_t; 

struct vector
{
	void *vector_array;
	size_t capacity;
	size_t size;
	size_t element_size;
};

vector_t *VectorCreate(size_t initial_capacity, size_t element_size)
{
	vector_t *vector_ptr = (vector_t *) malloc (1 * sizeof(vector_t));
	vector_ptr -> vector_array = (void *) malloc (initial_capacity * element_size);
	vector_ptr -> capacity = initial_capacity;
	vector_ptr -> element_size = element_size;
	vector_ptr -> size = 0;
	
	return vector_ptr;	
}

void *VectorGetAccessToElement(vector_t *vector, size_t index)
{
	return (char *)vector->vector_array + (index * vector->element_size);	
}

int VectorAppend(vector_t *vector, const void *value)
{
	return_status_t return_status = SUCCESS;
	void *realloc_buffer = NULL;
	assert(LAST_ELEMENT_IN_ARRAY);
	
	if (vector->size == vector->capacity)
	{
		realloc_buffer = realloc (vector->vector_array, (vector->capacity * vector->element_size) * GROWTH_RATE);
		
		if(NULL == realloc_buffer)
		{
			return_status = FAILURE;	
		}
		else
		{
			vector->capacity *= GROWTH_RATE;	
		}
	}
	
	memcpy(LAST_ELEMENT_IN_ARRAY, value, vector->element_size);	
	++vector->size;
	
	return return_status;
}

void VectorRemoveLast(vector_t *vector)
{	
	--vector->size;
}

int VectorIsEmpty(const vector_t *vector)
{
	return (0 == vector->size);
}                          

size_t VectorCapacity(const vector_t *vector)
{
	return (vector->capacity);	
}

size_t VectorSize(const vector_t *vector)
{
	return (vector->size);	
}

void VectorDestroy(vector_t *vector)
{
	assert(vector);
	free(vector->vector_array);
	free(vector);	
}

int VectorReserve(vector_t *vector, size_t size)
{
	return_status_t return_status = FAILURE;
	assert(vector);
	
	if (size > vector->capacity)
	{
		void *realloc_buffer = NULL;
		
		realloc_buffer = realloc (vector->vector_array, size * vector->element_size);

		if (NULL != realloc_buffer)
		{	
			return_status = SUCCESS;	
			vector->vector_array = realloc_buffer;
			vector->capacity = size; 
		}
	}
	return return_status;	
}

int VectorShrink(vector_t *vector)
{	
	return_status_t return_status = FAILURE;
	assert(vector);
	
	if(vector->size < vector->capacity / GROWTH_RATE)
	{
		void *realloc_buffer = NULL;
		realloc_buffer = realloc (vector->vector_array, (vector->size * vector->element_size) * GROWTH_RATE);
		
		if (NULL != realloc_buffer)
		{	
			return_status = SUCCESS;	
			vector->vector_array = realloc_buffer;
			vector->capacity = vector->size * GROWTH_RATE; 
		}
	}
	
	return return_status;		
}

