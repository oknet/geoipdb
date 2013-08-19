#include "base.h"

int Mcount=0;

void *Malloc(unsigned int size)
{
        void *p;
        p=(void *)malloc(size);
        if(p==NULL)
        {
                perror("malloc");
                exit(1);
        }
	Mcount++;
	/* printf("Alloc( %d )\n",Mcount); */
        return(p);
}
void *Realloc(void *p,unsigned int size)
{
        p=(void *)realloc(p,size);
        if(p==NULL)
        {
                perror("realloc");
                exit(1);
        }
        return(p);
}
void Free(void *p)
{
	/* printf("Free( %d )\n",Mcount); */
	Mcount--;
	free(p);
}
FILE *Fopen(char *filename, char *mode)
{
        FILE *fp;

        fp=fopen(filename,mode);
        if( fp == NULL )
        {
                printf("Fopen: Open File %s Error\n",filename);
                perror("fopen");
                exit(1);
        }
        return(fp);
}
/* 模仿 Perl split 语句 */
SPLIT * split(char *string,char *substr)
{
        SPLIT *p1=NULL,*p2,*p3;
        char *p4,*p5,*p6;
        int substrlen=Strlen(substr);

	/* printf("Split( %d )\n",Mcount); */

        p4=(char *)Malloc(Strlen(string)+1);
        p5=p4;
        strcpy(p4,string);
        while(1)
        {
                p6=p4;
                if( p4!=NULL )        p4=strstr(p4,substr);
                if( p6 != NULL )
                {
                        if(p4!=NULL)
                        {
                                *p4='\0';
                                p4+=substrlen;
                        }
                        p2=(SPLIT *)Malloc(sizeof(SPLIT));
                        p2->msg=(char *)Malloc(Strlen(p6)+1);
                        strcpy(p2->msg,p6);	p2->msg[Strlen(p6)]='\0';
                        p2->next=NULL;
                        if( p1 == NULL )
                        {
                                p1=p2;
                        }
                        else
                        {
                                p3->next=p2;
                        }
                        p3=p2;
                }
                else        break;
        }
        Free(p5);
	/* printf("_Split( %d )\n",Mcount); */
        return(p1);
}
SPLIT * free_split(SPLIT * link)
{
        if( link != NULL )
        {
                if( link->msg != NULL )
		{
			Free(link->msg);
			link->msg=NULL;
		}
	        link->next=free_split(link->next);
		Free(link);
        }
        return NULL;
}
/*********************************************
 把str中的str1替换为str2
 *********************************************/
char * ExChange(char *str,char *str1,char *str2)
{
	char *p,*p1;
	int size=0,size1,size2;
	char *strbuf;

	if( str==NULL || str1==NULL) return(str);
	size1=Strlen(str1);
	if( str2==NULL ) size2=0;
	else             size2=Strlen(str2);
	p=str;
	while(1)
	{
		if((p1=strstr(p,str1))==NULL) break;
		size+=(p1-p);
		strbuf=(char *)Malloc(Strlen(str)-size);
		p1+=size1;  strcpy(strbuf,p1);
		str=(char *)Realloc(str,Strlen(str)-size1+size2+1);
		sprintf(&str[size],"%s%s",(str2==NULL)?"":str2,strbuf);
		Free(strbuf);
		p=str;
		p+=size;
		p+=size2;
		size+=size2;
	}
	return(str);
}

/* 把string中的ch删除 */
void del_ch(char *string,char ch)
{
        int i;

        i=0;
        while( string[i] == ch )        i++;
        if( i != 0 )        strcpy(string,&string[i]);
        i=Strlen(string);
        while( string[i-1] == ch && i>0 )        i--;
        if( i != Strlen(string) )        string[i]='\0';
}

int StrCmp(const char *s1, const char *s2)
{
	if( s1==NULL && s2==NULL )	return(0);
	if( s1==NULL && s2!=NULL )	return(1);
	if( s1!=NULL && s2==NULL )	return(1);
	return(strcmp(s1,s2));
}
int StrnCmp(const char *s1, const char *s2, unsigned int n)
{
	if( s1==NULL && s2==NULL )	return(0);
	if( s1==NULL && s2!=NULL )	return(1);
	if( s1!=NULL && s2==NULL )	return(1);
	return(strncmp(s1,s2,n));
}
unsigned int Strlen(const char *s)
{
	if( s==NULL )	return(0);
	return(strlen(s));
}

void trim(char * str)
{
        int len, pos = 0;

        if(str == NULL) return;
        len = strlen(str);
        if( len==0 )    return;

        for(len--; str[len] == '\n' || str[len] == ' ' || str[len] == '\t'; len--)
                str[len] = 0;

        while(str[pos] == ' ' || str[pos] == '\t')
                pos++;

        if(str[pos] == '\"' && str[len] == '\"')
        {
                pos++;
                str[len] = 0;
                len--;
        }

        memmove(str, &str[pos], len - pos + 2);
}

int nsplit(char * buf, char ** name, char ** value)
{
        * value = strchr(buf, ':');
        if(* value == 0)        * value = strchr(buf, '=');
        if(* value == 0)        return -1;

        (* value)[0] = 0;
        * value = (* value + 1);
        * name = buf;

        trim(* name);
        trim(* value);

        return 0;
}

char x2c(char *what)
{
    register char digit;

    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
    return(digit);
}

void unescape_url(char *url)
{
    int x,y;

    for (x=0,y=0; url[y]; ++x,++y) {
        if ((url[x] = url[y]) == '%') {
            url[x] = x2c(&url[y+1]);
            y += 2;
        }
    }
    url[x] = '\0';
}

int my_iconv(char *from, char *to, char *inbuf, size_t inbufsize, char *outbuf, size_t outbufsize)
{
        iconv_t cd;
        char **in, **out;

        in=&inbuf;
        out=&outbuf;

        cd=iconv_open(to, from);
        if( cd==0 ) return(-1);
        if( iconv(cd, in, &inbufsize, out, &outbufsize) == -1 ) return(-1);
        iconv_close(cd);
        return(0);
}
