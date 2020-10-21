#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <search.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
    assert((elemSize > 0) && (initialAllocation >= 0));
    if (initialAllocation == 0)
        initialAllocation = 4;
    v->logLen = 0;
    v->initialAllocLen = initialAllocation;
    v->allocLen = initialAllocation;
    v->elemSize = elemSize;
    v->freefn = freeFn;
    v->elems = malloc(v->allocLen * v->elemSize);
    assert(v->elems != NULL);
}

void VectorDispose(vector *v)
{
    if (v->freefn != NULL)
    {
        for (int i = 0; i < v->logLen; i++)
        {
            void *currElement = (char *)v->elems + i * v->elemSize;
            v->freefn(currElement);
        }
    }
    free(v->elems);
}

int VectorLength(const vector *v)
{
    return v->logLen;
}

void *VectorNth(const vector *v, int position)
{
    assert((position >= 0) && (position < v->logLen));
    return (char *)v->elems + v->elemSize * position;
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
    assert((position >= 0) && (position < v->logLen));
    void *dest = (char *)v->elems + v->elemSize * position;
    if (v->freefn != NULL)
        v->freefn(dest);
    memcpy(dest, elemAddr, v->elemSize);
}

void VectorGrow(vector *v)
{
    v->allocLen += v->initialAllocLen;
    v->elems = realloc(v->elems, v->allocLen * v->elemSize);
    assert(v->elems != NULL);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
    assert((position >= 0) && (position <= v->logLen));
    if (v->logLen == v->allocLen)
        VectorGrow(v);

    void *src = (char *)v->elems + v->elemSize * position;
    void *dest = (char *)src + v->elemSize;
    int size = (v->logLen - position) * v->elemSize;
    memmove(dest, src, size);
    memcpy(src, elemAddr, v->elemSize);
    v->logLen++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
    if (v->logLen == v->allocLen)
        VectorGrow(v);
    void *dest = (char *)v->elems + v->elemSize * v->logLen;
    memcpy(dest, elemAddr, v->elemSize);
    v->logLen++;
}

void VectorDelete(vector *v, int position)
{
    assert((position >= 0) && (position < v->logLen));
    void *dest = (char *)v->elems + v->elemSize * position;
    void *src = (char *)dest + v->elemSize;
    int size = (v->logLen - position - 1) * v->elemSize;
    if (v->freefn != NULL)
        v->freefn(dest);
    memmove(dest, src, size);
    v->logLen--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
    assert(compare != NULL);
    qsort(v->elems, v->logLen, v->elemSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
    assert(mapFn != NULL);
    for (int i = 0; i < v->logLen; i++)
    {
        void *current = (char *)v->elems + v->elemSize * i;
        mapFn(current, auxData);
    }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
    assert((key != NULL) && (searchFn != NULL) && (startIndex >= 0) && (startIndex <= v->logLen));
    void *base = (char *)v->elems + startIndex * v->elemSize;
    size_t size = v->logLen - startIndex;
    const int elemSize = v->elemSize;
    void *result;

    if (isSorted)
    {
        result = bsearch(key, base, v->logLen - startIndex, v->elemSize, searchFn);
    }
    else
    {
        result = lfind(key, base, &size, elemSize, searchFn);
    }
    if (result == NULL)
        return kNotFound;
    return ((char *)result - (char *)v->elems) / v->elemSize;
}
