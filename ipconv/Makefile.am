automake_OPTIONS = foreign
INCLUDES= -g -O0 -Wall -I. `/usr/bin/mysql_config --include`
LIBS= -L. `/usr/bin/mysql_config --libs`

bin_PROGRAMS = ipconv automask gentopo

ipconv_SOURCES = ipconv.c mysqlfunc.c mysqlfunc.h hash.c hash.h base.c base.h
ipconv_DEPENDENCIES = $(find /usr -name libmysqlclient.la)

automask_SOURCES = automask.c

gentopo_SOURCES = gentopo.c mysqlfunc.c mysqlfunc.h hash.c hash.h base.c base.h
gentopo_DEPENDENCIES = $(find /usr -name libmysqlclient.la)
