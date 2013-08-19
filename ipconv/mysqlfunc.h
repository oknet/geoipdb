#ifndef __MYSQLFUNC__
#define __MYSQLFUNC__

#include <stdio.h>
#include <unistd.h>
#include <mysql.h>
#include <errmsg.h>
#include "base.h"

#define MAXQBUF 2048
#define GETQM   1
#define SETQM   2

typedef struct mydb
{
	MYSQL *mysql;
	char *host;
	char *user;
	char *passwd;
	char *db;
	unsigned int port;
	char *unix_socket;
	unsigned int client_flag;
        int nqbuf;
        char *qbuf[MAXQBUF];
        int qlength[MAXQBUF];
} MyDB;

MyDB * CreateMyDB(char *, unsigned int, char *, char *, char *, char *);
void OpenMyDB(MyDB *);
void ReOpenMyDB(MyDB *);
void QmodeMyDB(MyDB *, int, int *);
void QPushMyDB(MyDB *);
int QueryMyDB(MyDB *, int, char *, int );
void ErrorMyDB(MyDB *);
MyDB * FreeMyDB(MyDB *);
MYSQL_RES *MySqlStoreResult(MyDB *);

#endif
