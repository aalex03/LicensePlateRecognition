#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);
}

int main(int argc, char *argv[])
{
    MYSQL *con = mysql_init(NULL);
  if(argc<2)
     exit(1);
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
  char query[100];
  sprintf(query,"SELECT * FROM cars WHERE name='%s' ",argv[1]);
  if(mysql_query(con,query))
  {
    finish_with_error(con);
  }

  MYSQL_RES *result = mysql_store_result(con);
  int nrFields = mysql_num_fields(result);
  MYSQL_ROW row;
  while((row = mysql_fetch_row(result)))
  {
    for(int i = 0; i < nrFields; i++)
	printf("%s ",row[i] ? row[i] : "NULL");
  }
  mysql_free_result(result);
  mysql_close(con);
}
