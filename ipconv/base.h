#ifndef __BASE__
#define __BASE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>

/* Split 结构: 用于实现Perl split功能 */
typedef struct splitlink
{
	char *msg;
	struct splitlink *next;
} SPLIT;

void *Malloc(unsigned int);
void *Realloc(void *, unsigned int);
void Free(void *);
FILE *Fopen(char *, char *);
SPLIT * split(char *, char *);
SPLIT * free_split(SPLIT *);
char * ExChange(char *, char *, char *);
void del_ch(char *, char);
int StrCmp(const char *, const char *);
int StrnCmp(const char *, const char *, unsigned int );
unsigned int Strlen(const char *s);
void trim(char * str);
int nsplit(char * buf, char ** name, char ** value);
char x2c(char *what);
void unescape_url(char *url);
int my_iconv(char *from, char *to, char *inbuf, size_t inbufsize, char *outbuf, size_t outbufsize);

#endif
