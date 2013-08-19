#include "hash.h"

unsigned int primes[]={
                       1,       /* 2^ 0 -  0 */
                       2,       /* 2^ 1 -  0 */
                       3,       /* 2^ 2 -  1 */
                       7,       /* 2^ 3 -  1 */
                      13,       /* 2^ 4 -  3 */
                      31,       /* 2^ 5 -  1 */
                      61,       /* 2^ 6 -  3 */
                     127,       /* 2^ 7 -  1 */
                     251,       /* 2^ 8 -  5 */
                     509,       /* 2^ 9 -  3 */
                    1021,       /* 2^10 -  3 */
                    2039,       /* 2^11 -  9 */
                    4093,       /* 2^12 -  3 */
                    8191,       /* 2^13 -  1 */
                   16381,       /* 2^14 -  3 */
                   32749,       /* 2^15 - 19 */
                   65521,       /* 2^16 - 15 */
                  131071,       /* 2^17 -  1 */
                  262139,       /* 2^18 -  5 */
                  524287,       /* 2^19 -  1 */
                 1048573,       /* 2^20 -  3 */
                 2097143,       /* 2^21 -  9 */
                 4194301,       /* 2^22 -  3 */
                 8388593,       /* 2^23 - 15 */
                16777213};      /* 2^24 -  3 */
/*
                33554393,25 39
                67108859,26 5
                134217689,27 39
                268435399,28 57
                536870909,29 3
                1073741789, 30 35
                2147483647, 31 1
                4294967291, 32 5

*/

/*
void hash_dump(hash_t *h);
*/

/** private func start **/

/* hashfunc from mysql */
/* 2^56 - 5 */
/* 14695981039346656037 */
/*
32 bit FNV_prime = 16777619
64 bit FNV_prime = 1099511628211
128 bit FNV_prime = 309485009821345068724781401
256 bit FNV_prime = 374144419156711147060143317175368453031918731002211 

32 bit offset_basis = 2166136261
64 bit offset_basis = 14695981039346656037
128 bit offset_basis = 275519064689413815358837431229664493455
256 bit offset_basis = 100029257958052580907070968620625704837092796014241193945225284501741471925557 
*/
unsigned int calc_hashnr(const char *key, unsigned int len)
{
        const char *end=key+len;
        unsigned int hash;
        for (hash = 0; key < end; key++)
        {
                hash *= 16777619;
                hash ^= (unsigned int) *(unsigned char*) key;
        }
        return (hash);
}

unsigned int calc_hashnr_caseup(const char *key, unsigned int len)
{
        const char *end=key+len;
        unsigned int hash;
        for (hash = 0; key < end; key++)
        {
                hash *= 16777619;
                hash ^= (unsigned int) (unsigned char) toupper(*key);
        }
        return (hash);
}

/* string compare routine */
int comp_hash_c(const void *key1, const void *key2)
{
        hash_node_t *m1, *m2;

        m1 = ( hash_node_t *) key1;
        m2 = ( hash_node_t *) key2;

        return strcmp(m1->key, m2->key);
}
/* string compare ignoring case routine */
int comp_hash_cc(const void *key1, const void *key2)
{
        hash_node_t *m1, *m2;

        m1 = ( hash_node_t *) key1;
        m2 = ( hash_node_t *) key2;

        return strcasecmp(m1->key, m2->key);
}
/* int compare routine */
int comp_hash_i( const void *key1, const void *key2 )
{
        hash_node_t *m1, *m2;
        int *ui1, *ui2;

        m1 = ( hash_node_t *) key1;
        m2 = ( hash_node_t *) key2;

        ui1 = ( int * ) m1->key;
        ui2 = ( int * ) m2->key;

        if( *ui1 < *ui2 ) return(-1);
        if( *ui1 > *ui2 ) return(1);
        return(0);
}

/* unsigned int compare routine */
int comp_hash_ui( const void *key1, const void *key2 )
{
        hash_node_t *m1, *m2;
        unsigned int *ui1, *ui2;

        m1 = ( hash_node_t *) key1;
        m2 = ( hash_node_t *) key2;

        ui1 = ( unsigned int * ) m1->key;
        ui2 = ( unsigned int * ) m2->key;

        if( *ui1 < *ui2 ) return(-1);
        if( *ui1 > *ui2 ) return(1);
        return(0);
}

void hash_node_swap( hash_t *h, int ti, int tj, int si, int sj )
{
        hash_link_t *tp, *sp;
        hash_node_t bk;

        tp = &h->data[ti];
        sp = &h->data[si];

        memcpy( &bk,             &tp->bucket[tj], sizeof(hash_node_t) );
        memcpy( &tp->bucket[tj], &sp->bucket[sj], sizeof(hash_node_t) );
        memcpy( &sp->bucket[sj], &bk,             sizeof(hash_node_t) );
}

int hash_bucket_expand( hash_t *h, unsigned int i )
{
        hash_link_t *p;
        hash_node_t *bk;
        int incsize=0;

#ifdef DEBUG
        printf("*hash_bucket_expand\n");
#endif
        p = &h->data[i];

        if( p->bucket==NULL )
        {
                incsize=h->buckets;
        }
        else
        {
                if( p->buckets>4 )      incsize=h->buckets;
                else                    incsize=p->buckets;
        }
        // printf("hash_bucket_expand %u --> %u\n", p->buckets, p->buckets+incsize);
        bk = (hash_node_t *) realloc( p->bucket, (p->buckets+incsize)*sizeof(hash_node_t) );
        if( bk==NULL )
        {
                printf("%s[%d] :", __FILE__, __LINE__);
                perror("realloc");
                return(-1);
        }
#ifdef HASH_DEBUG
        printf("+%d\n", incsize*sizeof(hash_node_t));
#endif
        p->bucket = bk;
        if( p->buckets==0 )     h->usize++;

        bzero( &p->bucket[p->buckets], incsize*sizeof(hash_node_t) );
        h->msize+=incsize;
        p->buckets+=incsize;
        
        return(0);
}

/* clear a unit hash->data[i]->bucket[j] , and move last unit to here */
int hash_clear( hash_t *h, unsigned int i, unsigned int j )
{
        hash_link_t *p;

#ifdef DEBUG
        printf("*hash_clear\n");
#endif
        p = &h->data[i];
        if( p->used == 0 )      return(0);
        if( j > p->used-1 )     return(0);

        p->used--;
        if( p->used != j )      memcpy( &p->bucket[j], &p->bucket[p->used], sizeof(hash_node_t) );
        bzero( &p->bucket[p->used], sizeof(hash_node_t) );

        if( p->used == 0 )      h->usize--;
        h->umsize--;
        return(0);
}

int hash_clear_node( hash_t *h, hash_node_t *hn)
{
        hash_link_t *p;
        unsigned int i, v, ov;

        if( hn==NULL )  return(-1);

        ov = v = hn->v;
        v %= h->size;
        p = &h->data[v];
        for( i=0; i<p->used; i++ )
        {
                if( p->bucket[i].v == ov )
                {
                        hash_clear(h, v, i);
                        break;
                }
        }
        return(0);
}

/* move h->data[i].bucket[j] to h->data[ti].bucket[<last>] */
int hash_move( hash_t *h, unsigned int i, unsigned int j, unsigned ti )
{
        hash_link_t *p, *tp;
        
#ifdef DEBUG
        printf("*hash_move\n");
#endif
        p = &h->data[i];
        tp = &h->data[ti];

        if( tp->used==0 && tp->buckets>0 )      h->usize++;
        else if( tp->used >= tp->buckets && hash_bucket_expand( h, ti )<0 )     return(-1);

        memcpy( &tp->bucket[tp->used], &p->bucket[j], sizeof(hash_node_t) );
        tp->used++;
        h->umsize++;    /* hash_clear will do umsize-- */
        
        hash_clear( h, i, j );
        return(0);
}

/* expand hash size to size*2 */
int hash_expand( hash_t *h, unsigned int newsize )
{
        hash_link_t *p;
        int i, j, m, n;
        int d=0;

#ifdef DEBUG
        printf("*hash_expand\n");
#endif
        //if( newsize==0 )                newsize = primes[h->nprime];
        if( newsize > h->maxsize )      newsize=h->maxsize;
        if( h->size >= newsize )	return(1);
        //printf("hash_expand %u --> %u\n", h->size, newsize);
	if( newsize % h->size == 0 )	d = newsize/h->size;
        p = (hash_link_t *) realloc( h->data, newsize*sizeof(hash_link_t) );
        if( p==NULL )
        {
                printf("%s[%d] :", __FILE__, __LINE__);
                perror("realloc");
                return(-1);
        }
#ifdef HASH_DEBUG
        printf("+%d\n", (newsize-h->size)*sizeof(hash_link_t) );
#endif
        h->data=p;
        for( i=h->size; i<newsize; i++ )
        {
                p=&h->data[i];
/*
                p->bucket= (hash_node_t *) malloc( h->buckets*sizeof(hash_node_t) );
                if( p->bucket==NULL )
                {
                        printf("%s[%d] :", __FILE__, __LINE__);
                        perror("malloc");
                        return(-2);
                }
                p->buckets=h->buckets;
*/
                p->bucket=NULL;
                p->buckets=0;
                p->used=0;
                //bzero(p->bucket, h->buckets*sizeof(hash_node_t));
        }
        for( i=0; i<h->size; i++)
        {
                p=&h->data[i];

                for(j=0; j<p->used; j++)
                {
                	if( d )
                	{
                		/* special move */
                        	m = p->bucket[j].v/h->size;
                        	n = m%d;
                        	if( n )
                        	{
                                	/* move p->bucket[j] to h->data[j+n*h->size] */
                                	if( hash_move( h, i, j, i+n*h->size ) <0 )
                                	{
                                       		hash_clear( h, i, j );
                                        	printf("hash_move fault, unit lost!\n"); 
                                	}
                                	j--;
                        	}
                        }
                        else
                        {
                        	/* normal move */
                        	m = p->bucket[j].v%newsize;
                        	if( m == i )	continue;
                        	if( hash_move( h, i, j, m ) <0 )
                                {
                                	hash_clear( h, i, j );
                                       	printf("hash_move fault, unit lost!\n"); 
                                }
                                j--;
                        }
                }
        }
        h->size=newsize;
        //hash_dump(h);
        return(0);
}

int hash_shrink( hash_t *h, unsigned int newsize )
{
	return(0);
}

int hash_escheck( hash_t *h, unsigned int v )
{
	int i;
        hash_link_t *p;
        
#ifdef DEBUG
        printf("*hash_escheck\n");
#endif

        p = &h->data[v];

        if( h->usize*10 >= h->size*8 )
        {
                //hash_dump(h);
                i=h->expand(h, primes[h->nprime+1]);
		switch( i )
                {
			case -1: return(-1);  // memory not enough 277
			case -2: return(-1);  // memory not enough 292
			case  1: return(0);   // did not need enlarge 269
			case  0: h->nprime++; // success enlarged
                		 return(0);
                }
        }
        return(1);
}
/** private func end **/

/** public func start **/

/* hash_init will pre alloc size*buckets*sizeof(hash_t) bytes memory for data */
int hash_init( hash_t *h, int size, int buckets, int flag, unsigned int (*hashfunc)(const char *key, unsigned int len), int (*compare)(const void *key1, const void *key2), int (*destroy)(hash_node_t *hn))
{
        int i;
        hash_link_t *p;

        if( h == NULL )         return(-1);

        if( size<=0 )           size=flag|0x00000002;
        if( buckets<=0 )        buckets=2;

        h->size=size;
        h->maxsize=8388593;     // primes[23];
        h->usize=0;
        h->msize=0;
        h->umsize=0;
        h->flag=flag;
        h->buckets=buckets;
        pthread_mutex_init(&h->mutex, NULL);
        if( flag & HASH_PRIME_MASK )
        {
		if( size<=24 )
		{
			h->nprime=size;
			h->size=primes[h->nprime];
		}
		else
		{
        		h->nprime=1;
        		h->size=primes[h->nprime];
		}
        }
        else	h->nprime=0;

        if( hashfunc == NULL )
        {
                if( flag & HASH_CCASE_MASK )    h->calc_hashnr=calc_hashnr_caseup;
                else	                        h->calc_hashnr=calc_hashnr;
        }
        else    h->calc_hashnr=hashfunc;

	h->destroy=destroy;

        if( compare == NULL )
        {
                if( (flag & HASH_COMP_MASK) == HASH_COMP_C )
                {
                        if( flag & HASH_CCASE_MASK )    h->comp=comp_hash_cc;
                        else                            h->comp=comp_hash_c;
                }
                else if( (flag & HASH_COMP_MASK) == HASH_COMP_I )      h->comp=comp_hash_i;
                else if( (flag & HASH_COMP_MASK) == HASH_COMP_UI )     h->comp=comp_hash_ui;
                else /*if( (flag & HASH_COMP_MASK) == HASH_COMP_? )*/  h->comp=comp_hash_c;
        }
        else    h->comp=compare;

        h->escheck=hash_escheck;
        h->expand=hash_expand;
        h->shrink=hash_shrink;
        
        h->data = (hash_link_t *) malloc( h->size*sizeof(hash_link_t) );
        if( h->data==NULL )
        {
                printf("%s[%d] :", __FILE__, __LINE__);
                perror("malloc");
                return(-1);
        }
#ifdef HASH_DEBUG
        printf("+%d\n", h->size*sizeof(hash_link_t));
#endif
        for( i=0; i<h->size; i++ )
        {
                p=&h->data[i];
/*
                p->bucket= (hash_node_t *) malloc( buckets*sizeof(hash_node_t) );
                if( p->bucket==NULL )
                {
                        printf("%s[%d] :", __FILE__, __LINE__);
                        perror("malloc");
                        return(-2);
                }
                p->buckets=buckets;
*/
                p->bucket=NULL;
                p->buckets=0;
                p->used=0;
/*
                bzero(p->bucket, buckets*sizeof(hash_node_t));
*/
        }
        return(0);
}

int hash_setmaxsize(hash_t *h, int newsize, int prime)
{
        if( h==NULL )   return(1);
        if( prime>=0 )  newsize=primes[prime];
        if( newsize<= h->size )  return(2);
        h->maxsize=newsize;

        return(0);
}

int hash_destroy( hash_t *h )
{
	int i,j;
	
        for( i=0; i<h->size; i++ )
        {
        	if( h->data[i].bucket )
        	{
        		for(j=0; j<h->data[i].used; j++)
        		{
                                //printf("[%d,%d]\n", i, j);
        			if( h->destroy!=NULL )
        				h->destroy(&h->data[i].bucket[j]);
                                //printf("= %s\n", h->data[i].bucket[j].key);
        			free(h->data[i].bucket[j].key);
#ifdef HASH_DEBUG
                                printf("-%d\n", h->data[i].bucket[j].length);
#endif
        		}
			free(h->data[i].bucket);
#ifdef HASH_DEBUG
                        printf("-%d\n", h->data[i].buckets*sizeof(hash_node_t));
#endif
		}
        }
        pthread_mutex_destroy(&h->mutex);
        free(h->data);
#ifdef HASH_DEBUG
        printf("-%d\n", h->size*sizeof(hash_link_t));
#endif
	bzero(h, sizeof( hash_t ));
        return(0);
}

hash_node_t * hash_search( hash_t *h, char *key, unsigned int length, void (*op)(void *arg) )
{
        unsigned int i, v, ov;
        hash_link_t *p;
        int swaped=0;

        v=ov=h->calc_hashnr(key, length);

        v %= h->size;
        p = &h->data[v];

        for(i=0; i<p->used; i++)
        {
                if( ov == p->bucket[i].v && p->bucket[i].length==length && memcmp(p->bucket[i].key, key, length)==0 )
                {
                        p->bucket[i].select++;
                        if( i &&  p->bucket[i].select > p->bucket[i-1].select )
                        {
                                hash_node_swap( h, v, i, v, i-1 );
                                swaped=1;
                        }
                        if( op )        op(&p->bucket[i-swaped]);
                        return( &p->bucket[i-swaped] );
                }
        }
        return( NULL );
}


hash_node_t * hash_search_v( hash_t *h, unsigned int v, unsigned int ov, char *key, unsigned int length, void (*op)(void *arg) )
{
        unsigned int i;
        hash_link_t *p;
        int swaped=0;

        p = &h->data[v];
        for(i=0; i<p->used; i++)
        {
                if( ov == p->bucket[i].v && p->bucket[i].length==length && memcmp(p->bucket[i].key, key, length)==0 )
                {
                        p->bucket[i].select++;
                        if( i &&  p->bucket[i].select > p->bucket[i-1].select )
                        {
                                hash_node_swap( h, v, i, v, i-1 );
                                swaped=1;
                        }
                        if( op )        op(&p->bucket[i-swaped]);
                        return( &p->bucket[i-swaped] );
                }
        }
        return( NULL );
}

/* length should be include '\0' of key */
hash_node_t * hash_insert( hash_t *h, char *key, unsigned int length, void (*op)(void *arg) )
{
        unsigned int v, ov;
        hash_link_t *p;
        hash_node_t *hn;
        int ret;
        
#ifdef DEBUG
        printf("*hash_insert\n");
#endif
/*
        if( h->umsize >= h->msize/2 )
                if( hash_expand(h) < 0 )
                        perror("hash_expand fault!\n");
*/

        v=ov=h->calc_hashnr(key, length);
        
        /* call escheck to expand hash table */
        v %= h->size;
        ret = h->escheck(h,v);
        if( ret<0 )		return(NULL);
        else if( ret==0 )	v = ov%h->size;

        /* the key existed already ? */
        hn=hash_search_v(h, v, ov, key, length, op);
        if( hn!=NULL )     return(hn);

        p = &h->data[v];

        /* expand buckets if it full */
        if( p->used == 0 && p->buckets>0 )      h->usize++;
        else if( p->used >= p->buckets && hash_bucket_expand( h, v )<0 )        return(NULL);

        p->bucket[p->used].key=(char *) malloc(length*sizeof(char));
        if( p->bucket[p->used].key == NULL )
        {
                printf("%s[%d] :", __FILE__, __LINE__);
                perror("malloc");
                return(NULL);
        }
#ifdef HASH_DEBUG
        printf("+%d\n", length*sizeof(char));
#endif
        memcpy(p->bucket[p->used].key, key, length);
        p->bucket[p->used].length=length;
        p->bucket[p->used].v=ov;

        if( op )        op(&p->bucket[p->used]);

        p->used++;
        h->umsize++;

        return(&p->bucket[p->used-1]);
}

hash_link_t *hash_nosort_dump(hash_t *h, void *unused)
{
        hash_link_t *hl, *p;
        int i, j;

        hl = (hash_link_t *) malloc(sizeof(hash_link_t));
        hl->bucket = (hash_node_t *) malloc(h->umsize*sizeof(hash_node_t));
        if( hl->bucket==NULL )
        {
                perror("malloc");
                return(NULL);
        }
        hl->buckets=h->umsize;
        hl->used=0;

        for(j=0; j<h->size; j++)
        {
                p=&h->data[j];
                if( p->used )
                {
                        memcpy(&hl->bucket[hl->used], p->bucket, p->used*sizeof(hash_node_t));
                        hl->used+=p->used;
                }
        }
        return( hl );
}

hash_link_t *hash_sort_dump(hash_t *h, int (*comp)(const void *key1, const void *key2))
{
        hash_link_t *hl, *p;
        int i, j;

        hl = (hash_link_t *) malloc(sizeof(hash_link_t));
        hl->bucket = (hash_node_t *) malloc(h->umsize*sizeof(hash_node_t));
        if( hl->bucket==NULL )
        {
                perror("malloc");
                return(NULL);
        }
        hl->buckets=h->umsize;
        hl->used=0;

        /********* why this code lead a memory not free?
        p=h->data;

        for(j=0; j<h->size; j++)
        {
                if(p[j].bucket)
                {
                        for(i=0; i<p[j].used; i++)
                        {
                                memcpy(&hl->bucket[hl->used], &p[j].bucket[i], sizeof(hash_node_t));
                                hl->used++;
                        }
                }
        }
        ************************************************/

        for(j=0; j<h->size; j++)
        {
                p=&h->data[j];
                if( p->used )
                {
                        memcpy(&hl->bucket[hl->used], p->bucket, p->used*sizeof(hash_node_t));
                        hl->used+=p->used;
                }
        }
        if( comp==NULL )	qsort(hl->bucket, hl->used, sizeof(hash_node_t), h->comp);
        else			qsort(hl->bucket, hl->used, sizeof(hash_node_t), comp);
        return( hl );
}

void hash_memcount(hash_t *h, unsigned long int *total, unsigned long int *used)
{
        hash_link_t *p;
        int j;

	*total=*used=sizeof(hash_t)+sizeof(hash_link_t)*h->size;

        for(j=0; j<h->size; j++)
        {
                p=&h->data[j];
		*used+=sizeof(hash_node_t)*p->used;
		*total+=sizeof(hash_node_t)*p->buckets;
        }
}

void hash_lock(hash_t *h)
{
        pthread_mutex_lock(&h->mutex);
}

void hash_unlock(hash_t *h)
{
        pthread_mutex_unlock(&h->mutex);
}

#ifdef DEVDEBUG

int my_comp( const void *key1, const void *key2 )
{
        hash_node_t *m1, *m2;
        unsigned int *ui1, *ui2;

        m1 = ( hash_node_t *) key1;
        m2 = ( hash_node_t *) key2;

        ui1 = ( unsigned int * ) m1->key;
        ui2 = ( unsigned int * ) m2->key;

        if( *ui1 < *ui2 ) return(-1);
        if( *ui1 > *ui2 ) return(1);
        return(0);
}

void hash_dump(hash_t *h)
{
        hash_link_t *p;
        unsigned int i,j ;
        unsigned long long int used, buckets;
        unsigned int nbuckets[10240], maxb=0, sumb=0;
        
        used=buckets=0;
        bzero( nbuckets, sizeof(nbuckets) );

        for(i=0; i<h->size; i++)
        {
                p=&h->data[i];
                //printf("Array[%5d] = (%u/%u)\n", i, p->used, p->buckets);
                if( p->used > maxb )    maxb=p->used;
                if( p->used>=sizeof(nbuckets)/sizeof(unsigned int) )
                        nbuckets[sizeof(nbuckets)/sizeof(unsigned int)-1]++;
                else    nbuckets[p->used]++;
                used+=p->used;  buckets+=p->buckets;
/*
                for( j=0; j<p->used; j++ )
                        printf("(%d)%lu [%u]%s[%u]\n", i, (unsigned long int)p->bucket[j].data.u32, p->bucket[j].length, p->bucket[j].key, p->bucket[j].v);
*/
        }
        printf("Max Buckets : %u\n", maxb);
        printf("nbuckets distributing:\n");
        for(i=1; i<sizeof(nbuckets)/sizeof(unsigned int); i++)
        {
                if( nbuckets[i] )
                        sumb+= nbuckets[i];
        }
        for(i=0; i<sizeof(nbuckets)/sizeof(unsigned int); i++)
        {
                if( nbuckets[i] )
                {
                        if( i ) printf(" [%d] --> %6u(%5.2f%%)\n", i, nbuckets[i], 100.0*(float)(nbuckets[i])/(float)(sumb));
                        else    printf(" [%d] --> %6u\n", i, nbuckets[i]);
                }
        }
        printf("hash array size : %u/%u (%5.2f%%)\n", h->size-nbuckets[0], h->size, 100.0*(float)(h->size-nbuckets[0])/(float)(h->size));
        printf("default hash buckets size : %u\n", h->buckets);
        printf("the number of units : %u/%u (%5.2f%%)\n", h->umsize, h->msize, 100.0*(float)(h->umsize)/(float)(h->msize));
        printf("the hash space utility is %6.2f(%llu/%llu)\n", (float)(used)/(float)(buckets), used, buckets);
        printf("usize/size = %u/%u\n", h->usize, h->size);
        printf("umsize/msize = %u/%u\n", h->umsize, h->msize);
}

void op(void *arg)
{
        hash_node_t *n;
        n = (hash_node_t *)arg;
        n->data.u32++;
}

#include "mystr.c"

int main(int argc, char *argv[])
{
        hash_t h;
        hash_link_t *hl;
        char buf[2048];
        int i;

        hash_init( &h, 0, 1, 2, &calc_hashnr, NULL );
        while( !feof(stdin) )
        {
                if( fgets( buf, 2048, stdin)==NULL )    break;
                trim(buf);
                hash_insert(&h, buf, strlen(buf)+1, op);
        }
        printf("---\n");
        hash_dump(&h);
        h.nprime++;
        hash_expand(&h, 0);
        hash_dump(&h);
        hl=hash_sort_dump(&h);
        for(i=0; i<hl->used; i++)
        {
                printf("%s\n", hl->bucket[i].key);
        }
        return(0);
}
#endif
