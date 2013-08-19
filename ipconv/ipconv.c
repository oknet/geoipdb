#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "mysqlfunc.h"

struct iplink
{
        int id;
        unsigned int ips, ipe;
        char country[3];
        char province[16];
        char city[11];
        int isp;
        struct iplink *next;
};

typedef struct iplink IPLINK;

IPLINK *ipl_head;


unsigned int str2ip(char *ipstr)
{
        struct in_addr caddr;
        inet_pton(AF_INET, ipstr, &caddr.s_addr);
        return(ntohl(caddr.s_addr));
}

void ip2str(long unsigned int ip, char *ipstr, int align)
{
	if( align )
		sprintf(ipstr, "%03lu.%03lu.%03lu.%03lu", (ip&0xff000000)>>24, (ip&0x00ff0000)>>16, (ip&0x0000ff00)>>8, (ip&0x000000ff));
	else
		sprintf(ipstr, "%0lu.%0lu.%0lu.%0lu", (ip&0xff000000)>>24, (ip&0x00ff0000)>>16, (ip&0x0000ff00)>>8, (ip&0x000000ff));
}

IPLINK *ipl_new(int id, unsigned int ips, unsigned int ipe, char *country, char *province, char *city, int isp)
{
        IPLINK *p;

        p = (IPLINK *)malloc(sizeof(IPLINK));
        p->id=id;
        p->ips=ips;
        p->ipe=ipe;
        bzero(p->country, 3);   strncpy(p->country, country, 2);
        bzero(p->province, 11); strncpy(p->province, province, 10);
        bzero(p->city, 11);     strncpy(p->city, city, 10);
        p->isp=isp;
        p->next=NULL;
        return(p);
}

IPLINK *ipl_clone(IPLINK *from)
{
        IPLINK *p;

        p = (IPLINK *)malloc(sizeof(IPLINK));
        memcpy(p, from, sizeof(IPLINK));
        return(p);
}

#define EXISTED 1
int ipl_insert(IPLINK *ipl, IPLINK *new)
{
        IPLINK *p, *p1;
        unsigned int ss,se;

        if( new->ips > new->ipe )       return(-1);

        for(p=ipl; p; p=p->next)
        {
                if( p->ips==new->ips && p->ipe==new->ipe )
                {
                        if( p->id )
                        {
                                if( strcmp(new->country, p->country) ||           
                                    strcmp(new->province, p->province) ||
                                    strcmp(new->city, p->city) )
                                {
                                        p1=p->next;
                                        memcpy(p, new, sizeof(IPLINK));
                                        p->next=p1;
                                        return(0);
                                }
                                else    return(EXISTED);
                        }
                        //printf(" = =\n");
                        p1=p->next;
                        memcpy(p, new, sizeof(IPLINK));
                        p->next=p1;
                        return(0);
                }
                else if( p->ips<=new->ips && new->ipe<=p->ipe )
                {
                        //split
                        if( p->ips==new->ips )
                        {
                                //printf(" = <\n");
                                p1 = ipl_clone(p);
                                memcpy(p, new, sizeof(IPLINK));
                                memcpy(new, p1, sizeof(IPLINK));
                                p->next=new;
                                new->ips=p->ipe+1;
                                free(p1);
                                return(0);
                        }
                        else if( p->ipe==new->ipe )
                        {
                                //printf(" < =\n");
                                new->next=p->next;
                                p->next=new;
                                p->ipe=new->ips-1;
                                return(0);
                        }
                        else
                        {
                                //printf(" < <\n");
                                p1 = ipl_clone(p);
                                p->next=new;
                                new->next=p1;
                                p->ipe=new->ips-1;
                                p1->ips=new->ipe+1;
                                return(0);
                        }
                }
                else if( p->ips<=new->ips && p->ipe<=new->ipe )
                {
                        if( p->ipe<new->ips )   continue;
                        if( p->ips==new->ips )
                        {
                                //printf(" = >\n");
                                if( strcmp(new->country, p->country) || 
                                    strcmp(new->province, p->province) || 
                                    strcmp(new->city, p->city) )
                                {
                                        p1=p->next;
                                        ss=p->ips;
                                        se=p->ipe;
                                        memcpy(p, new, sizeof(IPLINK));
                                        p->next=p1;
                                        p->ips=ss;
                                        p->ipe=se;
                                }
                                new->ips=p->ipe+1;
                        }
                        else if( p->ips<new->ips )
                        {
                                //printf(" > >=\n");
                                if( new->isp != p->isp ||
                                    strcmp(new->country, p->country) || 
                                    strcmp(new->province, p->province) || 
                                    strcmp(new->city, p->city) )
                                {
                                        p1 = ipl_clone(new);
                                        p1->ipe=p->ipe;
                                        p->ipe=p1->ips-1;
                                        p1->next=p->next;
                                        p->next=p1;
                                }
                                if( p->ipe==new->ipe )  return(0);
                                else                    new->ips=p->ipe+1;
                        }
                }
                else
                {
                        printf(" ? ? (ips, ipe)=(%u, %u), (nips, nipe)=(%u, %u)\n", p->ips, p->ipe, new->ips, new->ipe);
                        //ipl_dump(ipl_head);
                }
        }
        return(0);
}
void ipl_aggr(IPLINK *p)
{
        IPLINK *p1;

        while(p)
        {
                while(p->next)
                {
                        p1=p->next;
                        if( p->ipe+1 == p1->ips && p->isp == p1->isp && 
                            p->isp == p1->isp &&
                            strcmp(p->country, p1->country)==0 &&
                            strcmp(p->province, p1->province)==0 &&
                            strcmp(p->city, p1->city)==0 )
                        {
                                p->ipe = p1->ipe;
                                p->next= p1->next;
                                free(p1);
                        }
                        else    break;
                }
                p=p->next;
        }
}

void ipl_dump(IPLINK *p)
{
        char cips[17], cipe[17];
        unsigned int nips, nipe;

        while(p)
        {
                nips=ntohl(p->ips);     nipe=ntohl(p->ipe);
                inet_ntop(AF_INET, &nips, cips, sizeof(cips));
                inet_ntop(AF_INET, &nipe, cipe, sizeof(cipe));
                if( p->id )     printf("(%s - %s) -> ", cips, cipe);
                else            printf("[%s - %s] -> ", cips, cipe);
                p=p->next;
        }
        printf("NIL\n");
}

void output(long unsigned int ip, int maskbits, int align, int shortmask)
{
	long unsigned int mask=0xffffffff;
	char ipstr[16], maskstr[16];

	mask>>=(32-maskbits);
	mask<<=(32-maskbits);
	ip&=mask;
	ip2str(ip, ipstr, align);
	ip2str(mask, maskstr, align);
	if( shortmask == 1 )
		printf("%s/%d\n", ipstr, maskbits);
	else if( shortmask == 2 )
		printf("%s/%s\n", ipstr, maskstr);
	else
		printf("%s/%s/%d\n", ipstr, maskstr, maskbits);
}

void automask(unsigned long int ips, unsigned long int ipe, int align, int shortmask)
{
	int offset;
	long unsigned int tmp;

	if( ipe < ips )
	{
		tmp=ipe;
		ipe=ips;
		ips=tmp;
	}

	for(offset=0; offset<32; offset++)
	{
		//printf("Offser=%d, ips=%lx, ipe=%lx\n", offset, ips, ipe);
		if( ips == ipe )
		{
			output(ips<<offset, 32-offset, align, shortmask);
			break;
		}
		if( (ips & 0x00000001L)==1 )
		{
			output(ips<<offset, 32-offset, align, shortmask);
			ips++;
		}
		if( (ipe & 0x00000001L)==0 )
		{
			output(ipe<<offset, 32-offset, align, shortmask);
			ipe--;
		}
		if( (ips & 0x00000001L)==0 && (ipe & 0x00000001L)==1 )
		{
			ips>>=1;
			ipe>>=1;
		}
		if( ips > ipe )	break;
	}
}

void usage()
{
	printf("ipconv <align> <shortmask> <level> <isp>\n");
	printf("<align> == 0 : 192.168.1.1\n");
	printf("<align> == 1 : 192.168.001.001\n");
	printf("<shortmask> == 1 : <ip>/24\n");
	printf("<shortmask> == 2 : <ip>/255.255.255.0\n");
	printf("<shortmask> == other : <ip>/255.255.255.0/24\n");
	printf("<level> == 0 : create one region\n");
	printf("<level> == 1 : create region group by country,province\n");
	printf("<level> == 2 : create region group by country,province,city\n");
	printf("<isp> == 1 : CNC\n");
	printf("<isp> == 2 : CTC\n");
	printf("<isp> == CN : China\n");
	exit(1);
}

int main(int argc, char *argv[])
{
        MyDB *db;
        MYSQL_RES *res;
        MYSQL_ROW row;
	int align=0, shortmask=0, level=0, isp=0;
        char sqlbuf[4096], country[3]="  ";
	IPLINK *p;
        
	if( argc<5 )	usage();

	align=atoi(argv[1]);
	shortmask=atoi(argv[2]);
	level=atoi(argv[3]);
	isp=atoi(argv[4]);
	strncpy(country, argv[4],2);
        ipl_head=ipl_new(0, 0x00000000, 0xffffffff, "", "", "", 0);
        db=CreateMyDB("localhost",3306,NULL,"root","", "newip");
        OpenMyDB(db);
	if( level==0 )
	{
		if( isp==0 )	snprintf(sqlbuf, sizeof(sqlbuf), "select id, ips, ipe from iprange where country='%s'", country);
		else		snprintf(sqlbuf, sizeof(sqlbuf), "select id, ips, ipe from iprange where isp=%d", isp);
	}
	if( level==1 ) snprintf(sqlbuf, sizeof(sqlbuf), "select a.id, ips, ipe, country, b.eng from iprange a left join dict b on a.province=b.src where isp=%d", isp);
	if( level==2 ) snprintf(sqlbuf, sizeof(sqlbuf), "select id, ips, ipe, country,province,city from iprange where isp=%d", isp);
	// printf("SQL::%s\n", sqlbuf);
        QueryMyDB(db, 1, sqlbuf, strlen(sqlbuf));
        res=MySqlStoreResult(db);
        while ((row = mysql_fetch_row(res)))
        {
                //printf("// [%s - %s]\n", row[1], row[2]);
		if( level==1 )		p=ipl_new(atoi(row[0]), str2ip(row[1]), str2ip(row[2]), row[3], (row[4]==NULL)?"unknown":row[4], "", isp);
		else if( level==2 )	p=ipl_new(atoi(row[0]), str2ip(row[1]), str2ip(row[2]), row[3], row[4], row[5], isp);
		else			p=ipl_new(atoi(row[0]), str2ip(row[1]), str2ip(row[2]), (isp==0)?country:"", "", "", isp);
                ipl_insert(ipl_head, p);
        }
        mysql_free_result(res);
        FreeMyDB(db);

	ipl_aggr(ipl_head);
	p=ipl_head;
	while(p)
	{
		if( ( isp==0 && strncmp(p->country, country, 2)==0 ) || ( isp!=0 && p->isp==isp ) )
		{
                	ip2str(p->ips, sqlbuf, align);
			ip2str(p->ipe, &sqlbuf[1024], align);
			if( level==0 )	printf("// [%s - %s]\n", sqlbuf, &sqlbuf[1024]);
			if( level==1 )	printf("// %s:%s:[%s - %s]\n%s:%s:", p->country,p->province, sqlbuf, &sqlbuf[1024], p->country,p->province);
			if( level==2 )	printf("// %s:%s:%s:[%s - %s]\n%s:%s:%s:", p->country,p->province,p->city, sqlbuf, &sqlbuf[1024], p->country,p->province,p->city);
			automask(p->ips,p->ipe,align,shortmask);
		}
		p=p->next;
	}

        return 0;
}

