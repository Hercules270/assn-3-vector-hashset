#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const int initAllocLen = 10;
void initializeVectors(vector *elems, int numBuckets, int elemSize, HashSetFreeFunction freefn)
{
	for (int i = 0; i < numBuckets; i++)
	{
		VectorNew(&elems[i], elemSize, freefn, initAllocLen);
	}
}

void HashSetNew(hashset *h, int elemSize, int numBuckets,
				HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
	assert((elemSize > 0) && (numBuckets > 0) && (hashfn != NULL) && (comparefn != NULL));
	h->elemSize = elemSize;
	h->numBuckets = numBuckets;
	h->logLen = 0;
	h->hashfn = hashfn;
	h->cmpfn = comparefn;
	h->freefn = h->freefn;
	h->elems = malloc(numBuckets * sizeof(vector));
	assert(h->elems != NULL);
	initializeVectors(h->elems, numBuckets, elemSize, freefn);
}

void HashSetDispose(hashset *h)
{
	for (int i = 0; i < h->numBuckets; i++)
	{
		VectorDispose(&h->elems[i]);
	}
	free(h->elems);
}

int HashSetCount(const hashset *h)
{
	return h->logLen;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
	assert(mapfn != NULL);
	for (int i = 0; i < h->numBuckets; i++)
	{
		VectorMap(&(h->elems[i]), mapfn, auxData);
	}
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
	int bucket = h->hashfn(elemAddr, h->numBuckets);
	assert((bucket >= 0) && (bucket < h->numBuckets) && (elemAddr != NULL));
	vector *current = &(h->elems[bucket]);
	int position = VectorSearch(current, elemAddr, h->cmpfn, 0, 0);
	if (position == -1)
	{
		VectorAppend(current, elemAddr);
		h->logLen++;
	}
	else
	{
		VectorReplace(current, elemAddr, position);
	}
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{
	int bucket = h->hashfn(elemAddr, h->numBuckets);
	assert((bucket >= 0) && (bucket < h->numBuckets) && (elemAddr != NULL));
	vector *current = &(h->elems[bucket]);
	int position = VectorSearch(current, elemAddr, h->cmpfn, 0, false);
	if (position == -1)
		return NULL;
	return VectorNth(current, position);
}
