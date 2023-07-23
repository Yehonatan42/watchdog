#include <assert.h>
#include <stdlib.h>

#include "vector.h"
#include "heap.h"

#define HEAPSIZE 10

typedef int (*is_match_t)(const void *param, const void *value);

static void HeapSwap(heap_t *heap, size_t index1, size_t index2);
static void HeapifyUp(heap_t *heap, size_t index, cmp_func_t cmp_func_p);
static void HeapifyDown(heap_t *heap, size_t index, cmp_func_t cmp_func_p);
static size_t HeapParentIndex(size_t length);
static size_t HeapRightChild(size_t index);
static size_t HeapLeftChild(size_t index);
static void *HeapGetElement(const heap_t *heap, size_t index);
static int HeapSearchIndex(heap_t *heap, is_match_t is_match_p, 
                                         const void *param, 
                                         int curr_index);

struct heap
{
    vector_t *vector;
    cmp_func_t cmp_func_p;
};

heap_t *HeapCreate(cmp_func_t cmp_func_p)
{
    heap_t *new_heap = NULL;

    assert(cmp_func_p);

    new_heap = (heap_t *) malloc (sizeof(heap_t));

    if (NULL == new_heap)
    {
        return NULL;
    }
    
    new_heap->vector = VectorCreate(HEAPSIZE, sizeof(void *));

    if (NULL == new_heap->vector)
    {
        free(new_heap); new_heap = NULL;
        return NULL;
    }
    
    new_heap->cmp_func_p = cmp_func_p;

    return new_heap;
}

void HeapDestroy(heap_t *heap)
{
    assert(heap);
    assert(heap->vector);

    VectorDestroy(heap->vector); heap->vector = NULL;
    free(heap); heap = NULL;
}

status_t HeapPush(heap_t *heap, void *value)
{
    assert(heap);
    assert(value);
    assert(heap->vector);

    if (1 == VectorAppend(heap->vector, &value))
    {
        return MEMORY_ALLOCATION_ERROR;
    }

    HeapifyUp(heap, HeapSize(heap) - 1, heap->cmp_func_p);

    return SUCCESS;
}

void *HeapPop(heap_t *heap)
{
    void *removed_value = NULL;

    assert(heap);
    assert(heap->vector);

    removed_value = HeapPeek(heap);
    HeapSwap(heap, 0, HeapSize(heap) - 1);
    VectorRemoveLast(heap->vector);
    
    HeapifyDown(heap, 0, heap->cmp_func_p);

    return removed_value;    
}

void *HeapPeek(const heap_t *heap)
{   
    assert(heap);
    assert(heap->vector);

    return (*(void **)HeapGetElement(heap, 0));
}

size_t HeapSize(const heap_t *heap)
{
    assert(heap);
    assert(heap->vector);

    return (VectorSize(heap->vector));
}

int HeapIsEmpty(const heap_t *heap)
{
    assert(heap);
    assert(heap->vector);

    return (VectorIsEmpty(heap->vector));
}

void *HeapRemove(heap_t *heap, 
				 int (*is_match)(const void *param, const void *value), 
				 const void *param)
{
    int remove_index = 0;
    void *return_data = NULL;

    assert(heap);
    assert(heap->vector);
    assert(is_match);
    assert(param);

    remove_index = HeapSearchIndex(heap, is_match, param, 0);

    if (-1 == remove_index)
    {
        return NULL;
    }

    return_data = *(void **)HeapGetElement(heap, remove_index);
    HeapSwap(heap, remove_index, HeapSize(heap) - 1);
    VectorRemoveLast(heap->vector);
    HeapifyUp(heap, remove_index, heap->cmp_func_p);
    HeapifyDown(heap, remove_index, heap->cmp_func_p);

    return return_data;
}  

static void HeapifyUp(heap_t *heap, size_t index, cmp_func_t cmp_func_p)
{
    int parent_index = 0;

    assert(heap);

    parent_index = HeapParentIndex(index + 1);

    while (0 <= parent_index && 
           0 < cmp_func_p(*(void**)HeapGetElement(heap, index), 
                          *(void**)HeapGetElement(heap, parent_index)))
    {
        HeapSwap(heap, index, parent_index);
        index = parent_index;
        parent_index = HeapParentIndex(parent_index + 1);
    }
}

static void HeapifyDown(heap_t *heap, size_t index, cmp_func_t cmp_func_p)
{
    size_t left_child_index = 0;
    size_t right_child_index = 0;
    size_t heap_size = 0;

    assert(heap);

    heap_size = HeapSize(heap);
    left_child_index = HeapLeftChild(index);
    right_child_index = HeapRightChild(index);

    while (heap_size > left_child_index)
    { 
        if (heap_size > right_child_index) 
        {
            if (0 < cmp_func_p(*(void**)HeapGetElement(heap, right_child_index), 
                               *(void**)HeapGetElement(heap, left_child_index)))
            {
                if (0 < cmp_func_p(*(void**)HeapGetElement(heap, 
                                                           right_child_index), 
                                   *(void**)HeapGetElement(heap, index)))
                {
                    HeapSwap(heap, right_child_index, index);
                    index = right_child_index;
                    right_child_index = HeapRightChild(index);
                    left_child_index = HeapLeftChild(index);
                    continue;
                }
                else
                {
                    break;
                }
            }
        }
        if (0 < cmp_func_p(*(void**)HeapGetElement(heap, left_child_index), 
                           *(void**)HeapGetElement(heap, index)))
        {
            HeapSwap(heap, left_child_index, index);
            index = left_child_index;
            left_child_index = HeapLeftChild(index);
            right_child_index = HeapRightChild(index);
            continue;
        }
        else
        {
            break;
        }
    }
}

static size_t HeapLeftChild(size_t index)
{
    return (index * 2 + 1);
}

static size_t HeapRightChild(size_t index)
{
    return (index * 2 + 2);
}

static size_t HeapParentIndex(size_t length)
{
    return (length / 2 - 1);
}

static int HeapSearchIndex(heap_t *heap, is_match_t is_match_p, 
                                         const void *param, 
                                         int curr_index)
{
    int match_status = is_match_p(param, 
                                  *(void**)HeapGetElement(heap, curr_index));

    if ((size_t)curr_index >= HeapSize(heap))
    {
        return -1;
    }

    else if (1 == match_status)
    {
        return curr_index;
    }
    
    return (HeapSearchIndex(heap, is_match_p, param, curr_index + 1));                                                                    
}

static void HeapSwap(heap_t *heap, size_t index1, size_t index2)
{
	void *temp = NULL;
    void **element1 = NULL;
    void **element2 = NULL;

	assert(heap);
	
    element1 = HeapGetElement(heap, index1);
    element2 = HeapGetElement(heap, index2);

	temp = *element1;
    *element1 = *element2;
    *element2 = temp;
}

static void *HeapGetElement(const heap_t *heap, size_t index)
{
    assert(heap);
    assert(heap->vector);

    return ((void **)VectorGetAccessToElement(heap->vector, index));
}
