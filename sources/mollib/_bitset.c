#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "bitset.h"


struct bitset {
	int bsize;
	int size;
	char bits[1];
};


struct bitset *bs_new(int size)
{
	struct bitset *ret;

	ret = calloc(sizeof(struct bitset) + (size + 7) / 8, sizeof(char));
	assert(ret);
	ret->bsize = size;
	ret->size = (size + 7) / 8;
	return(ret);
}

struct bitset *bs_newtrue(int size)
{
	struct bitset *ret = bs_new(size);
	memset(ret->bits,-1,ret->size);
	return(ret);
}


char *bs_mask(struct bitset *bs, int pos, int *mask)
{
	int bit,byte;
	int i;
	char *c;

	assert(bs);
	assert(bs->bsize > pos);

	byte = pos / 8;
	bit  = pos % 8;

	for (c = bs->bits, i = 0; i < byte; ++i)
		++c;

	for (*mask = 1, i = 0; i < bit; ++i)
		*mask <<= 1;
		
	return(c);
}


void bs_set(struct bitset *bs, int pos)
{
	int mask;
	char *c;

	c = bs_mask(bs, pos, &mask);

	*c |= mask;
}


void bs_reset(struct bitset *bs, int pos)
{
	int mask;
	char *c;

	c = bs_mask(bs, pos, &mask);

	*c &= ~mask;
}


BOOL bs_isset(struct bitset *bs, int pos)
{
	int mask;
	char *c;

	c = bs_mask(bs, pos, &mask);

	return(*c & mask);
}


struct bitset *bs_and(struct bitset *b1, struct bitset *b2)
{
	int i;
	struct bitset *ret;

	assert(b1);
	assert(b2);
	assert(b1->size == b2->size);

	ret = bs_new(b1->bsize);

	for (i = 0; i < b1->size; ++i)
		ret->bits[i] = (char)(b1->bits[i] & b2->bits[i]);

	return(ret);
}

struct bitset *bs_or(struct bitset *b1, struct bitset *b2)
{
	int i;
	struct bitset *ret;

	assert(b1);
	assert(b2);
	assert(b1->size == b2->size);

	ret = bs_new(b1->bsize);

	for (i = 0; i < b1->size; ++i)
		ret->bits[i] = (char)(b1->bits[i] | b2->bits[i]);

	return(ret);
}


int bs_count(struct bitset *bs)
{
	int ret = 0;
	int i;
	char *c;
	char temp;

	assert(bs);

	c = bs->bits;
	temp = *c;
	for (i = 0; i < bs->bsize; ++i) {
		if (i && (i%8 == 0)) {
			++c;
			temp = *c;
		}

		if (temp & 1)
			++ret;

		temp >>= 1;
	}

	return(ret);
}

struct bitset *bs_free(struct bitset *bs)
{
	free(bs);
	return(NULL);
}
