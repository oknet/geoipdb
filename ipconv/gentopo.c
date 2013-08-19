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

char *lend;

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

void ipl_destroy(IPLINK **ipl)
{
	IPLINK *p, *p1;
	
	p=*ipl;
	
	while(p)
	{
		p1=p->next;
		free(p);
		p=p1;
	}
	*ipl=NULL;
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
		printf("      %s/%d%s\n", ipstr, maskbits, lend);
	else if( shortmask == 2 )
		printf("      %s/%s%s\n", ipstr, maskstr, lend);
	else
		printf("      %s/%s/%d%s\n", ipstr, maskstr, maskbits, lend);
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
	printf("ipconv <align> <shortmask> <level> <isp> [<mode>]\n");
	printf("<align> == 0 : 192.168.1.1\n");
/*
	printf("<align> == 1 : 192.168.001.001\n");
*/
	printf("<shortmask> == 1 : <ip>/24\n");
/*
	printf("<shortmask> == 2 : <ip>/255.255.255.0\n");
	printf("<shortmask> == other : <ip>/255.255.255.0/24\n");
*/
	printf("<level> == 0 : create one region\n");
	printf("<level> == 1 : create region group by country,province\n");
	printf("<isp> == 0 : All ISP\n");
	printf("<isp> == 1 : CNC\n");
	printf("<isp> == 2 : CTC\n");
	printf("[<mode>] == 0 : BIND9 ACL format (default).\n");
	printf("[<mode>] == 1 : F5 GTM region.user format.\n");
	exit(1);
}

void gentopo(MyDB *db, int isp, char *province, char *name, int mode)
{
	char sqlbuf[4096];
	MYSQL_RES *res;
	MYSQL_ROW row;
	IPLINK *p;

	if( mode == 0 )
	{
		printf("acl \"%s\" {\n", name);
	}
	else
	{
		printf("   region {\n");
		printf("      name   \"%s\"\n", name);
	}

	if( province==NULL )
		snprintf(sqlbuf, sizeof(sqlbuf), "select id, ips, ipe from iprange where isp=%d", isp);
	else if( strlen(province)==0 )
		snprintf(sqlbuf, sizeof(sqlbuf), "select a.id, ips, ipe, country, b.eng from iprange a left join dict b on a.province=b.src where isp=%d and b.eng is NULL", isp);
	else	snprintf(sqlbuf, sizeof(sqlbuf), "select a.id, ips, ipe, country, b.eng from iprange a left join dict b on a.province=b.src where isp=%d and b.eng='%s'", isp, province);
        QueryMyDB(db, 1, sqlbuf, strlen(sqlbuf));
        res=MySqlStoreResult(db);
        ipl_head=ipl_new(0, 0x00000000, 0xffffffff, "", "", "", 0);
        while ((row = mysql_fetch_row(res)))
        {
                //printf("// [%s - %s]\n", row[1], row[2]);
		if( province==NULL || strlen(province)==0 )
			p=ipl_new(atoi(row[0]), str2ip(row[1]), str2ip(row[2]), "", "", "", isp);
		else	p=ipl_new(atoi(row[0]), str2ip(row[1]), str2ip(row[2]), row[3], (row[4]==NULL)?"unknown":row[4], "", isp);
                ipl_insert(ipl_head, p);
        }
        mysql_free_result(res);
        
        ipl_aggr(ipl_head);
	p=ipl_head;
	while(p)
	{
		if( p->isp==isp )
		{
                	ip2str(p->ips, sqlbuf, 0/*align*/);
			ip2str(p->ipe, &sqlbuf[1024], 0/*align*/);
			printf("// %s:%s:[%s - %s]\n", p->country,p->province, sqlbuf, &sqlbuf[1024]);
			automask(p->ips,p->ipe,0/*align*/,1/*shortmask*/);
		}
		p=p->next;
	}
	ipl_destroy(&ipl_head);
	if( mode == 0 )		printf("   };\n");
	else			printf("   }\n");
}

int main(int argc, char *argv[])
{
        MyDB *db;
	int align=0, shortmask=0, level=0, isp=0, mode=0;
        
	if( argc<5 )	usage();

	align=atoi(argv[1]);
	shortmask=atoi(argv[2]);
	level=atoi(argv[3]);
	isp=atoi(argv[4]);
	if( argc == 6 )	mode=atoi(argv[5]);

        db=CreateMyDB("localhost",3306,NULL,"root","", "newip");
        OpenMyDB(db);

	// header
	if( mode == 0 )
	{
		lend=";";
		printf("#bind9 acl file, generated by gentopo.\n");
	}
        else
	{
		lend="";
		printf("region_db user {\n");
	}

	// gen CNC
	if( isp==0 || isp==1 )
	{
		gentopo(db, 1, "",              "cnc-unknown",		mode);
	        gentopo(db, 1, "anhui",         "cnc-anhui",		mode);
       		gentopo(db, 1, "beijing",       "cnc-beijing",		mode);
        	gentopo(db, 1, "chongqing",     "cnc-chongqing",	mode);
        	gentopo(db, 1, "fujian",        "cnc-fujian",		mode);
        	gentopo(db, 1, "gansu",         "cnc-gansu",		mode);
        	gentopo(db, 1, "guangdong",     "cnc-guangdong",	mode);
        	gentopo(db, 1, "guangxi",       "cnc-guangxi",		mode);
        	gentopo(db, 1, "guizhou",       "cnc-guizhou",		mode);
        	gentopo(db, 1, "hainan",        "cnc-hainan",		mode);
        	gentopo(db, 1, "hebei",         "cnc-hebei",		mode);
        	gentopo(db, 1, "heilongjiang",  "cnc-heilongjiang",	mode);
        	gentopo(db, 1, "henan",         "cnc-henan",		mode);
        	gentopo(db, 1, "hubei",         "cnc-hubei",		mode);
        	gentopo(db, 1, "hunan",         "cnc-hunan",		mode);
        	gentopo(db, 1, "jiangsu",       "cnc-jiangsu",		mode);
        	gentopo(db, 1, "jiangxi",       "cnc-jiangxi",		mode);
        	gentopo(db, 1, "jilin",         "cnc-jilin",		mode);
        	gentopo(db, 1, "liaoning",      "cnc-liaoning",		mode);
        	gentopo(db, 1, "neimeng",       "cnc-neimeng",		mode);
        	gentopo(db, 1, "ningxia",       "cnc-ningxia",		mode);
        	gentopo(db, 1, "qinghai",       "cnc-qinghai",		mode);
        	gentopo(db, 1, "shandong",      "cnc-shandong",		mode);
        	gentopo(db, 1, "shanghai",      "cnc-shanghai",		mode);
        	gentopo(db, 1, "shannxi",       "cnc-shannxi",		mode);
        	gentopo(db, 1, "shanxi",        "cnc-shanxi",		mode);
        	gentopo(db, 1, "sichuan",       "cnc-sichuan",		mode);
        	gentopo(db, 1, "tianjin",       "cnc-tianjin",		mode);
        	gentopo(db, 1, "xinjiang",      "cnc-xinjiang",		mode);
        	gentopo(db, 1, "xizang",        "cnc-xizang",		mode);
        	gentopo(db, 1, "yunnan",        "cnc-yunnan",		mode);
        	gentopo(db, 1, "zhejiang",      "cnc-zhejiang",		mode);
	}

	// gen CTC
	if( isp==0 || isp==2 )
	{
		gentopo(db, 2, "",              "ctc-unknown",		mode);
        	gentopo(db, 2, "anhui",         "ctc-anhui",		mode);
        	gentopo(db, 2, "beijing",       "ctc-beijing",		mode);
        	gentopo(db, 2, "chongqing",     "ctc-chongqing",	mode);
        	gentopo(db, 2, "fujian",        "ctc-fujian",		mode);
        	gentopo(db, 2, "gansu",         "ctc-gansu",		mode);
        	gentopo(db, 2, "guangdong",     "ctc-guangdong",	mode);
        	gentopo(db, 2, "guangxi",       "ctc-guangxi",		mode);
        	gentopo(db, 2, "guizhou",       "ctc-guizhou",		mode);
        	gentopo(db, 2, "hainan",        "ctc-hainan",		mode);
        	gentopo(db, 2, "hebei",         "ctc-hebei",		mode);
        	gentopo(db, 2, "heilongjiang",  "ctc-heilongjiang",	mode);
        	gentopo(db, 2, "henan",         "ctc-henan",		mode);
        	gentopo(db, 2, "hubei",         "ctc-hubei",		mode);
        	gentopo(db, 2, "hunan",         "ctc-hunan",		mode);
        	gentopo(db, 2, "jiangsu",       "ctc-jiangsu",		mode);
        	gentopo(db, 2, "jiangxi",       "ctc-jiangxi",		mode);
        	gentopo(db, 2, "jilin",         "ctc-jilin",		mode);
        	gentopo(db, 2, "liaoning",      "ctc-liaoning",		mode);
        	gentopo(db, 2, "neimeng",       "ctc-neimeng",		mode);
        	gentopo(db, 2, "ningxia",       "ctc-ningxia",		mode);
        	gentopo(db, 2, "qinghai",       "ctc-qinghai",		mode);
        	gentopo(db, 2, "shandong",      "ctc-shandong",		mode);
        	gentopo(db, 2, "shanghai",      "ctc-shanghai",		mode);
        	gentopo(db, 2, "shannxi",       "ctc-shannxi",		mode);
        	gentopo(db, 2, "shanxi",        "ctc-shanxi",		mode);
        	gentopo(db, 2, "sichuan",       "ctc-sichuan",		mode);
        	gentopo(db, 2, "tianjin",       "ctc-tianjin",		mode);
        	gentopo(db, 2, "xinjiang",      "ctc-xinjiang",		mode);
        	gentopo(db, 2, "xizang",        "ctc-xizang",		mode);
        	gentopo(db, 2, "yunnan",        "ctc-yunnan",		mode);
        	gentopo(db, 2, "zhejiang",      "ctc-zhejiang",		mode);
        }

        // gen Cernet
	if( isp==0 || isp==4 )
	{
        	gentopo(db, 4, NULL,              "cernet",		mode);
        }

        // gen CMNET
	if( isp==0 || isp==5 )
	{
        	gentopo(db, 5, "",              "cmnet-unknown",	mode);
        	gentopo(db, 5, "anhui",         "cmnet-anhui",		mode);
        	gentopo(db, 5, "beijing",       "cmnet-beijing",	mode);
        	gentopo(db, 5, "chongqing",     "cmnet-chongqing",	mode);
        	gentopo(db, 5, "fujian",        "cmnet-fujian",		mode);
        	gentopo(db, 5, "gansu",         "cmnet-gansu",		mode);
        	gentopo(db, 5, "guangdong",     "cmnet-guangdong",	mode);
        	gentopo(db, 5, "guangxi",       "cmnet-guangxi",	mode);
        	gentopo(db, 5, "guizhou",       "cmnet-guizhou",	mode);
        	gentopo(db, 5, "hainan",        "cmnet-hainan",		mode);
        	gentopo(db, 5, "hebei",         "cmnet-hebei",		mode);
        	gentopo(db, 5, "heilongjiang",  "cmnet-heilongjiang",	mode);
        	gentopo(db, 5, "henan",         "cmnet-henan",		mode);
        	gentopo(db, 5, "hubei",         "cmnet-hubei",		mode);
        	gentopo(db, 5, "hunan",         "cmnet-hunan",		mode);
        	gentopo(db, 5, "jiangsu",       "cmnet-jiangsu",	mode);
        	gentopo(db, 5, "jiangxi",       "cmnet-jiangxi",	mode);
        	gentopo(db, 5, "jilin",         "cmnet-jilin",		mode);
        	gentopo(db, 5, "liaoning",      "cmnet-liaoning",	mode);
        	gentopo(db, 5, "neimeng",       "cmnet-neimeng",	mode);
        	gentopo(db, 5, "ningxia",       "cmnet-ningxia",	mode);
        	gentopo(db, 5, "qinghai",       "cmnet-qinghai",	mode);
        	gentopo(db, 5, "shandong",      "cmnet-shandong",	mode);
        	gentopo(db, 5, "shanghai",      "cmnet-shanghai",	mode);
        	gentopo(db, 5, "shannxi",       "cmnet-shannxi",	mode);
        	gentopo(db, 5, "shanxi",        "cmnet-shanxi",		mode);
        	gentopo(db, 5, "sichuan",       "cmnet-sichuan",	mode);
        	gentopo(db, 5, "tianjin",       "cmnet-tianjin",	mode);
        	gentopo(db, 5, "xinjiang",      "cmnet-xinjiang",	mode);
        	gentopo(db, 5, "xizang",        "cmnet-xizang",		mode);
        	gentopo(db, 5, "yunnan",        "cmnet-yunnan",		mode);
        	gentopo(db, 5, "zhejiang",      "cmnet-zhejiang",	mode);
        }

        // gen CRTC
        //gentopo(db, 7, NULL,              "crtc",		mode);

	if( isp==0 || isp==7 )
	{
        	gentopo(db, 7, "",              "crtc-unknown",		mode);
        	gentopo(db, 7, "anhui",         "crtc-anhui",		mode);
        	gentopo(db, 7, "beijing",       "crtc-beijing",		mode);
        	gentopo(db, 7, "chongqing",     "crtc-chongqing",	mode);
        	gentopo(db, 7, "fujian",        "crtc-fujian",		mode);
        	gentopo(db, 7, "gansu",         "crtc-gansu",		mode);
        	gentopo(db, 7, "guangdong",     "crtc-guangdong",	mode);
        	gentopo(db, 7, "guangxi",       "crtc-guangxi",		mode);
        	gentopo(db, 7, "guizhou",       "crtc-guizhou",		mode);
        	gentopo(db, 7, "hainan",        "crtc-hainan",		mode);
        	gentopo(db, 7, "hebei",         "crtc-hebei",		mode);
        	gentopo(db, 7, "heilongjiang",  "crtc-heilongjiang",	mode);
        	gentopo(db, 7, "henan",         "crtc-henan",		mode);
        	gentopo(db, 7, "hubei",         "crtc-hubei",		mode);
        	gentopo(db, 7, "hunan",         "crtc-hunan",		mode);
        	gentopo(db, 7, "jiangsu",       "crtc-jiangsu",		mode);
        	gentopo(db, 7, "jiangxi",       "crtc-jiangxi",		mode);
        	gentopo(db, 7, "jilin",         "crtc-jilin",		mode);
        	gentopo(db, 7, "liaoning",      "crtc-liaoning",	mode);
        	gentopo(db, 7, "neimeng",       "crtc-neimeng",		mode);
        	gentopo(db, 7, "ningxia",       "crtc-ningxia",		mode);
        	gentopo(db, 7, "qinghai",       "crtc-qinghai",		mode);
        	gentopo(db, 7, "shandong",      "crtc-shandong",	mode);
        	gentopo(db, 7, "shanghai",      "crtc-shanghai",	mode);
        	gentopo(db, 7, "shannxi",       "crtc-shannxi",		mode);
        	gentopo(db, 7, "shanxi",        "crtc-shanxi",		mode);
        	gentopo(db, 7, "sichuan",       "crtc-sichuan",		mode);
        	gentopo(db, 7, "tianjin",       "crtc-tianjin",		mode);
        	gentopo(db, 7, "xinjiang",      "crtc-xinjiang",	mode);
        	gentopo(db, 7, "xizang",        "crtc-xizang",		mode);
        	gentopo(db, 7, "yunnan",        "crtc-yunnan",		mode);
        	gentopo(db, 7, "zhejiang",      "crtc-zhejiang",	mode);
        }

        if( mode == 0 )		printf("#end\n");
	else			printf("}\n");
        FreeMyDB(db);
        return 0;
}

