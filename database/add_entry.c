#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void finish_with_error(MYSQL *con)
{
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);
}

int main(int argc, char **argv)
{
    MYSQL *con = mysql_init(NULL);
    if(argc==1)
    {
        printf("Usage: %s [params...]\n",argv[0]);
    }
    if (con == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(con));
        exit(1);
    }

    if (mysql_real_connect(con, "localhost", "root", "alex",
                           "testdb", 0, NULL, 0) == NULL)
    {
        finish_with_error(con);
    }

    char query[255];
    sprintf(query,"INSERT INTO cars (`nume_angajat`, `nr_inmatriculare`) VALUES ('%s', '%s')",argv[1],argv[2]);

    if(mysql_query(con,query))
    {
        finish_with_error(con);
    }
    mysql_close(con);
}