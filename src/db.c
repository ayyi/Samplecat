#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
#ifdef OLD
  #include <libart_lgpl/libart.h>
#endif

#include "mysql/mysql.h"
#include "dh-link.h"
#include "typedefs.h"
#include <gqview2/typedefs.h>
#include "support.h"
#include "main.h"
#include "db.h"
extern struct _app app;
extern char err [32];
extern char warn[32];
extern unsigned debug;

static gboolean is_connected = FALSE;

/*
	CREATE DATABASE samplelib;

	CREATE TABLE `samples` (
	  `id` int(11) NOT NULL auto_increment,
	  `filename` text NOT NULL,
	  `filedir` text,
	  `keywords` varchar(60) default '',
	  `pixbuf` blob,
	  `length` int(22) default NULL,
	  `sample_rate` int(11) default NULL,
	  `channels` int(4) default NULL,
	  `online` int(1) default NULL,
	  `last_checked` datetime default NULL,
	  `mimetype` tinytext,
	  `notes` mediumtext,
	  `colour` tinyint(4) default NULL,
	  PRIMARY KEY  (`id`)
	)

	BLOB can handle up to 64k.

*/
	/*
	Calls:
	MYSQL *mysql_init(MYSQL *mysql)
	char *mysql_get_server_info(MYSQL *mysql)
	void mysql_close(MYSQL *mysql)
	*/
 
	/*
	MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db,
			unsigned int port, const char *unix_socket, unsigned int client_flag)
	*/

gboolean
db__connect()
{
	MYSQL *mysql = &app.mysql;

	if(!mysql_init(&(app.mysql))){
		printf("Failed to initiate MySQL connection.\n");
		exit(1);
	}
	dbg (1, "MySQL Client Version is %s\n", mysql_get_client_info());

	if(!mysql_real_connect(mysql, app.config.database_host, app.config.database_user, app.config.database_pass, app.config.database_name, 0, NULL, 0)){
		errprintf("cannot connect to database: %s\n", mysql_error(mysql));
		//currently this wont be displayed, as the window is not yet opened.
		statusbar_printf(1, "cannot connect to database: %s\n", mysql_error(mysql));
		return FALSE;
	}
	if(debug) printf("MySQL Server Version is %s\n", mysql_get_server_info(mysql));

	if(!mysql_select_db(mysql, app.config.database_name /*const char *db*/)==0)/*success*/{
		errprintf("Failed to connect to Database: %s\n", mysql_error(mysql));
		return FALSE;
	}

	is_connected = TRUE;
	return TRUE;
}
 

gboolean
db__is_connected()
{
	return is_connected;
}


int
db__insert(char *qry)
{
	MYSQL *mysql = &app.mysql;
	int id = 0;

	if(mysql_exec_sql(mysql, qry)==0){
		if(debug) printf("db_insert(): ok!\n");
		id = mysql_insert_id(mysql);
	}else{
		perr("not ok...\n");
		return 0;
	}
	return id;
}


gboolean
db_delete_row(int id)
{
	char sql[1024];
	snprintf(sql, 1024, "DELETE FROM samples WHERE id=%i", id);
	dbg(2, "row: sql=%s", sql);
	if(mysql_query(&app.mysql, sql)){
		perr("delete failed! sql=%s\n", sql);
		return false;
	}
	return true;
}


int 
mysql_exec_sql(MYSQL *mysql, const char *create_definition)
{
	return mysql_real_query(mysql, create_definition, strlen(create_definition));
}


gboolean
db_update_path(const char* old_path, const char* new_path)
{
	gboolean ok = false;
	MYSQL *mysql = &app.mysql;

	char* filename = NULL; //FIXME
	char* old_dir = NULL; //FIXME

	char query[1024];
	snprintf(query, 1023, "UPDATE samples SET filedir='%s' WHERE filename='%s' AND filedir='%s'", new_path, filename, old_dir);
	dbg(0, "%s", query);

	if(!mysql_exec_sql(mysql, query)){
		ok = TRUE;
	}
	return ok;
}


MYSQL_RES *dir_iter_result = NULL;

void
db__dir_iter_new()
{
	#define DIR_LIST_QRY "SELECT DISTINCT filedir FROM samples ORDER BY filedir"
	MYSQL *mysql = &app.mysql;

	if(dir_iter_result) gwarn("previous query not free'd?");

	//char qry[1024];
	//snprintf(qry, 1024, DIR_LIST_QRY);

	if(mysql_exec_sql(mysql, DIR_LIST_QRY) == 0){
		dir_iter_result = mysql_store_result(mysql);
		/*
		else{  // mysql_store_result() returned nothing
		  if(mysql_field_count(mysql) > 0){
				// mysql_store_result() should have returned data
				printf( "Error getting records: %s\n", mysql_error(mysql));
		  }
		}
		*/
	}
	else{
		dbg("failed to find any records: %s", mysql_error(mysql));
	}
}


char*
db__dir_iter_next()
{
	if(!dir_iter_result) return NULL;

	MYSQL_ROW row;
	row = mysql_fetch_row(dir_iter_result);
	if(!row) return NULL;

	//printf("update_dir_node_list(): dir=%s\n", row[0]);

	/*
	char uri[256];
	snprintf(uri, 256, "%s", row[0]);

	return uri;
	*/
	return row[0];
}


void
db__dir_iter_free()
{
	if(dir_iter_result) mysql_free_result(dir_iter_result);
	dir_iter_result = NULL;
}


