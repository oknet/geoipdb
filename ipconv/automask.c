#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void ip2str(long unsigned int ip, char *ipstr, int align)
{
	if( align )
		sprintf(ipstr, "%03lu.%03lu.%03lu.%03lu", (ip&0xff000000)>>24, (ip&0x00ff0000)>>16, (ip&0x0000ff00)>>8, (ip&0x000000ff));
	else
		sprintf(ipstr, "%0lu.%0lu.%0lu.%0lu", (ip&0xff000000)>>24, (ip&0x00ff0000)>>16, (ip&0x0000ff00)>>8, (ip&0x000000ff));
}
void output(long unsigned int ip, int maskbits, int align, int shortmask)
{
	long unsigned int mask=0xffffffff;
	char ipstr[16], maskstr[16];

	mask>>=(32-maskbits);
	mask<<=(32-maskbits);
	ip&=mask;
	ip2str(ip, ipstr, align);
	ip2str(ip, maskstr, align);
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

int main(int argc, char *argv[])
{
	unsigned long int a=0xDACE6000, b=0xDACE9FFF;
	char buf1[16], buf2[16];
	unsigned int ip1,ip2,ip3,ip4;

	if( argc>=3 )
	{
		if( strchr(argv[2],'.')==NULL )
			b=strtoul(argv[2], NULL, 10);
		else
		{
			sscanf(argv[2], "%u.%u.%u.%u", &ip1, &ip2, &ip3, &ip4);
			b=ip1*256*256*256+ip2*256*256+ip3*256+ip4;
		}
	}
	if( argc>=2 )
	{
		if( strchr(argv[1],'.')==NULL )
			a=strtoul(argv[1], NULL, 10);
		else
		{
			sscanf(argv[1], "%u.%u.%u.%u", &ip1, &ip2, &ip3, &ip4);
			a=ip1*256*256*256+ip2*256*256+ip3*256+ip4;
		}
	}

	ip2str(a,buf1,0);
	ip2str(b,buf2,0);
	printf("[%s - %s]\n", buf1, buf2);
	automask(a,b,1,1);

	return(0);
}
