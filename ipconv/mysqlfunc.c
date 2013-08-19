#include "mysqlfunc.h"

MyDB * CreateMyDB(char *host, unsigned int port, char *socket, char *user, char *passwd, char *database)
{
	MyDB *tmp;

	tmp=(MyDB *)Malloc(sizeof(MyDB));
	tmp->mysql=mysql_init((MYSQL *)NULL);

	if( socket==NULL )
	{
		tmp->host=(char *)Malloc(Strlen(host)+1);
		strcpy(tmp->host,host);
		tmp->port=port;
		tmp->unix_socket=NULL;
	}
	else
	{
		tmp->host=NULL;
		tmp->port=0;
		tmp->unix_socket=(char *)Malloc(Strlen(socket)+1);
		strcpy(tmp->unix_socket,socket);
	}
	tmp->user=(char *)Malloc(Strlen(user)+1);
	strcpy(tmp->user,user);

	tmp->passwd=(char *)Malloc(Strlen(passwd)+1);
	strcpy(tmp->passwd,passwd);

	tmp->db=(char *)Malloc(Strlen(database)+1);
	strcpy(tmp->db,database);

	tmp->client_flag=0;
        tmp->nqbuf=-1;
	return(tmp);
}

void OpenMyDB(MyDB *handle)
{
	if( !mysql_real_connect(handle->mysql,
				handle->host,	
				handle->user,
				handle->passwd,
				handle->db,
				handle->port,
				NULL,0) )
		ErrorMyDB( handle );
}

void ReOpenMyDB(MyDB *handle)
{
	Free(handle->mysql);
	handle->mysql=mysql_init((MYSQL *)NULL);
	if( !mysql_real_connect(handle->mysql,
				handle->host,	
				handle->user,
				handle->passwd,
				handle->db,
				handle->port,
				NULL,0) )
		ErrorMyDB( handle );
}

void QmodeMyDB(MyDB *handle, int op, int *mode)
{
        if( op==GETQM )
        {
                if( handle->nqbuf == -1 )       *mode=0;
                else                            *mode=1;
        }
        else if( op==SETQM )
        {
                if( handle->nqbuf == -1 )
                {
                        if( *mode == 1 )
                                handle->nqbuf=0;
                }
                else
                {
                        if( *mode == 0 )
                        {
                                QPushMyDB(handle);
                                handle->nqbuf=-1;
                        }
                }
        }
}

void QPushMyDB(MyDB *handle)
{
        int i=0, err, n, rn;

        if( handle->nqbuf<0 )   return;
        while(i<handle->nqbuf)
        {
                n=9;
                while(n)
                {
                        rn=mysql_real_query(handle->mysql, handle->qbuf[i], handle->qlength[i]);
                        err=mysql_errno(handle->mysql);

                        if( rn && ( err == CR_SERVER_LOST || err == CR_SERVER_GONE_ERROR || err == CR_CONN_HOST_ERROR ) )
                        {
                                ErrorMyDB(handle);
                                sleep((10-n)*5);
                                ReOpenMyDB(handle);
                                n--;
                        }
                        else if( rn )
                        {
                                ErrorMyDB(handle);
                                printf("Some broken, SQL String Dumping ...\n");
                                printf("===================================\n");
                                for(;i<handle->nqbuf;i++)
                                {
                                        printf("%s;\n", handle->qbuf[i]);
                                }
                                printf("===================================\n");
                                printf("SQL String Dumping Completed.\n");
                                exit(0);
                        }
                        else    break;
                }
                Free(handle->qbuf[i]);
                i++;
        }
        handle->nqbuf=0;
}

int QueryMyDB(MyDB *handle, int flag, char *query, int length)
{
	int rn, err, n=9;

/*
	unsigned int len;
	char *buf;

	buf=(char *)Malloc(length * 2 + 1);
	len=mysql_escape_string(buf, query, length);
	rn=mysql_real_query(mysql, buf, length);
	Free(buf);
*/

        if( flag == 0 && handle->nqbuf >= 0 )
        {
                handle->qbuf[handle->nqbuf] = (char *) Malloc((length+1)*sizeof(char));
                memcpy(handle->qbuf[handle->nqbuf], query, length);
		handle->qbuf[handle->nqbuf][length]='\0';
                handle->qlength[handle->nqbuf]=length;
                handle->nqbuf++;
                if( handle->nqbuf==MAXQBUF )    QPushMyDB(handle);
                return(0);
        }
	while( n )
	{
		rn=mysql_real_query(handle->mysql, query, length);
                err=mysql_errno(handle->mysql);

                if( rn && ( err == CR_SERVER_LOST || err == CR_SERVER_GONE_ERROR || err == CR_CONN_HOST_ERROR ) )
		{
			printf("MyDB ERROR %d : %s\n", mysql_errno(handle->mysql), mysql_error(handle->mysql));
			printf("Reconnect to SQL Server...");
                        sleep((10-n)*5);
			ReOpenMyDB(handle);
                        n--;
		}
		else if( rn )
		{
			printf("Query String : %s\n",query);
			ErrorMyDB(handle);
                        return( rn );
		}
		else	return( rn );
	}
        return(0);
}

void ErrorMyDB(MyDB *handle)
{
	if( handle->host )
		printf("      Host : %s\n",handle->host);
	if( handle->unix_socket )
		printf("    Socket : %s\n",handle->unix_socket);
	printf("      User : %s\n",handle->user);
	printf("  Password : %s\n",handle->passwd);
	printf("  Database : %s\n",handle->db);
	printf("ERROR %4d : %s\n", mysql_errno(handle->mysql), mysql_error(handle->mysql));
	//handle = FreeMyDB(handle);
	//exit(10);
}

MyDB *FreeMyDB(MyDB *handle)
{
        if( handle->nqbuf>=0 )
                QPushMyDB(handle);
	mysql_close( handle->mysql );
	/* free(handle->mysql); */
	Free(handle->host);
	Free(handle->unix_socket);
	Free(handle->user);
	Free(handle->passwd);
	Free(handle->db);
	Free(handle);
	return(NULL);
}


/* 用完后使用mysql_free_result()释放res
   调用mysql_fetch_row()从结果集合中取出行
       mysql_row_seek()从结果集合中获得当前的行位置。
       mysql_row_tell()在设置当前的行位置。

 */
MYSQL_RES *MySqlStoreResult(MyDB *handle)
{
	MYSQL_RES *res;

	res=mysql_store_result(handle->mysql);
	if( mysql_field_count(handle->mysql) && res==NULL )	ErrorMyDB(handle);

	return(res);
}
