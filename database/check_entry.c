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
    sprintf(query,"SELECT * from cars WHERE nr_inmatriculare = '%s';",argv[1]);
    MYSQL_RES *result;
    MYSQL_ROW row;
    if(mysql_query(con,query))
    {
        finish_with_error(con);
    }
    result = mysql_store_result(con);
    if(mysql_num_rows(result) == 0)
    {
        printf("License plate %s not in database.\n",argv[1]);
        mysql_close(con);
        exit(1);
    }
    row = mysql_fetch_row(result);
    printf("Found %s in db, welcome %s.\n",row[2],row[1]);
    mysql_close(con);
    exit(0);
}