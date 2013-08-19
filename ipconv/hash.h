#ifndef __HASH__
#define __HASH__

//#define HASH_DEBUG

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <pthread.h>

#include "base.h"

#define HASH_CCASE_MASK 0x00000001      /* caseup */

#define HASH_PRIME_MASK 0x00000002      /* prime expand */

#define HASH_COMP_MASK  0x000000F0
#define HASH_COMP_C     0x00000000      /* strcmp */
#define HASH_COMP_I     0x00000010      /* int */
#define HASH_COMP_UI    0x00000020      /* unsigned int */
#define HASH_COMP_F     0x00000030      /* float */
#define HASH_COMP_D     0x00000040      /* double */
#define HASH_COMP_UK1   0x00000050      /* reserve */
#define HASH_COMP_UK2   0x00000060      /* reserve */
#define HASH_COMP_UK3   0x00000070      /* reserve */

/*
 * Fowler/Noll/Vo hash
 *
 * The basis of the hash algorithm was taken from an idea sent by email to the
 * IEEE Posix P1003.2 mailing list from Phong Vo (kpv@research.att.com) and
 * Glenn Fowler (gsf@research.att.com).  Landon Curt Noll (chongo@toad.com)
 * later improved on their algorithm.
 *
 * The magic is in the interesting relationship between the special prime
 * 16777619 (2^24 + 403) and 2^32 and 2^8.
 *
 * This hash produces the fewest collisions of any function that we've seen so
 * far, and works well on both numbers and strings.
 */

extern unsigned int primes[];

typedef union hash_data
{
        void *ptr;
        int i;
        uint32_t u32;
        uint64_t u64;
} hash_data_t;

typedef struct hash_node                /* sizeof = 32 */
{                      
        int select;                     /* select times, used by buckets sorting */
        unsigned int v;			/* hash value */
        char *key;   			/* key */
        unsigned int length;            /* length for key */
        hash_data_t data;		/* data area */
} hash_node_t;

typedef struct hash_link                /* sizeof = 16 */
{
        unsigned int buckets, used;
        hash_node_t *bucket;
} hash_link_t;

typedef struct hash                     /* sizeof = 128 */
{
        unsigned int size, usize, maxsize;      /* hash array size , used and maxsize */
        unsigned int msize, umsize;  	        /* the number of units,  */
        unsigned int buckets;           	/* default bucket size */
        int flag;                       	/* flag[bit0]==0 ignore case, =1 not ignore */
        					/* flag[bit1]==0 size*2, =1 by prime table */
        int nprime;
        int (*destroy)(hash_node_t *);
        int (*escheck)(struct hash *, unsigned int );
        					/* check hash table and try to expand or shrink it */
        int (*expand)(struct hash *, unsigned int);
        int (*shrink)(struct hash *, unsigned int);
        unsigned int (*calc_hashnr)(const char *key,unsigned int length);
        int (*comp)(const void *key1, const void *key2);

        pthread_mutex_t mutex;
        hash_link_t *data;
} hash_t;

/** private func start **/

unsigned int calc_hashnr(const char *key, unsigned int len);
unsigned int calc_hashnr_caseup(const char *key, unsigned int len);
void hash_node_swap( hash_t *h, int ti, int tj, int si, int sj );
int hash_bucket_expand( hash_t *h, unsigned int i );

/* clear a unit , and move last unit to here */
int hash_clear( hash_t *h, unsigned int i, unsigned int j );

/* move h->data[i].bucket[j] to h->data[ti].bucket[<last>] */
int hash_move( hash_t *h, unsigned int i, unsigned int j, unsigned ti );

/* expand hash size to size*2 */
int hash_expand( hash_t *h, unsigned int newsize );

int hash_shrink( hash_t *h, unsigned int newsize );

int hash_escheck( hash_t *h, unsigned int v );

/** private func end **/

/** public func start **/

/* hash_init will pre alloc size*buckets*sizeof(hash_t) bytes memory for data */
int hash_init( hash_t *h, int size, int buckets, int flag, unsigned int (*hashfunc)(const char *key, unsigned int len), int (*compare)(const void *key1, const void *key2), int (*destroy)(hash_node_t *hn));

int hash_setmaxsize(hash_t *h, int newsize, int prime);

int hash_destroy( hash_t *h );

int hash_clear_node( hash_t *h, hash_node_t *hn );

hash_node_t * hash_search( hash_t *h, char *key, unsigned int length, void (*op)(void *arg) );

hash_node_t * hash_search_v( hash_t *h, unsigned int v, unsigned int ov, char *key, unsigned int length, void (*op)(void *arg) );

/* length should include '\0' of key */
hash_node_t * hash_insert( hash_t *h, char *key, unsigned int length, void (*op)(void *arg) );

/* hash sort and dump into a hash_link_t struct */
hash_link_t *hash_nosort_dump(hash_t *h, void *unused);
hash_link_t *hash_sort_dump(hash_t *h, int (*comp)(const void *key1, const void *key2));

/* count memory usage */
void hash_memcount(hash_t *h, unsigned long int *total, unsigned long int *used);

/* lock & unlock hash data for pthread safe */
void hash_lock(hash_t *h);
void hash_unlock(hash_t *h); 
#endif
