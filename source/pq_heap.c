#include <assert.h> /*assert*/
#include <stdlib.h> /*free, malloc*/

#include "pq_heap.h"
#include "heap.h"

struct pq_heap
{
	heap_t *heap;
};

pq_heap_t *PQHeapCreate(int (*cmp_func_t)(const void *param, 
                                          const void *value))
{
	pq_heap_t *pq_heap = NULL;

	assert (cmp_func_t);

	pq_heap = (pq_heap_t *) malloc (sizeof(pq_heap_t));

	if (NULL == pq_heap)
	{
		return NULL;
	}

	pq_heap->heap = HeapCreate(cmp_func_t);
	if (NULL == pq_heap->heap)
	{
		free(pq_heap); pq_heap = NULL;
	}
	
	return (pq_heap);
}

void PQHeapDestroy(pq_heap_t *pq_heap)
{
	assert(pq_heap);
	assert(pq_heap->heap);
	
	HeapDestroy(pq_heap->heap);
	free(pq_heap); pq_heap = NULL;
}

int PQHeapEnqueue(pq_heap_t *pq_heap, void *value)
{
	assert(pq_heap);
	assert(pq_heap->heap);
	
	return (HeapPush(pq_heap->heap, value));
}

void *PQHeapDequeue(pq_heap_t *pq_heap)
{
	assert(pq_heap);
	assert(pq_heap->heap);
	
	return (HeapPop(pq_heap->heap));
}

void *PQHeapPeek(const pq_heap_t *pq_heap)
{
	assert(pq_heap);
	assert(pq_heap->heap);
	
	return (HeapPeek(pq_heap->heap));
}

size_t PQHeapSize(const pq_heap_t *pq_heap)
{
	assert(pq_heap);
	assert(pq_heap->heap);
	
	return (HeapSize(pq_heap->heap));
}

int PQHeapIsEmpty(const pq_heap_t *pq_heap)
{
	assert(pq_heap);
	assert(pq_heap->heap);
	
	return (HeapIsEmpty(pq_heap->heap));
}

void *PQHeapErase(pq_heap_t *pq_heap, 
                  int (*is_match)(const void *param, const void *value), 
                  void *param)
{
	assert(pq_heap);
	assert(is_match);
	assert(param);

	return (HeapRemove(pq_heap->heap, is_match, param));
}

void PQHeapClear(pq_heap_t *pq_heap)
{
	while (!PQHeapIsEmpty(pq_heap))
	{
		PQHeapDequeue(pq_heap);
	}
}
