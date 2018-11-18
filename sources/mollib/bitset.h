/*		bitset.h
*/

#ifndef BOOL
	#define	BOOL int
	#define TRUE 1
	#define FALSE 0
#endif


struct bitset *bs_new(int size);
struct bitset *bs_newtrue(int size);
void bs_set(struct bitset *bs, int pos);
void bs_reset(struct bitset *bs, int pos);
BOOL bs_isset(struct bitset *bs, int pos);
struct bitset *bs_and(struct bitset *b1, struct bitset *b2);
struct bitset *bs_or(struct bitset *b1, struct bitset *b2);
int bs_count(struct bitset *bs);
struct bitset *bs_free(struct bitset *bs);

typedef struct bitset *bitsetp;
