/* 
 * File:   controllers.h
 * Author: Semen Dubina <semen@sam002.net>
 *
 * Created on 19 Dec 2014
 */


#include <regex.h>
#include "controllers.h"

/**********************   CONTROLLERS PROGRAM CODE   **************************/


char* search_module(const char *uri_path){
	char* module_name = NULL;
	regex_t preg;
	
	char* string = NULL;
	
	if(uri_path == NULL){
		return NULL;
	}
	else{
	
	string = malloc(strlen(uri_path));
	strcat(string, uri_path);
	
	// If path correct search in structure array, compare with template
		int *tmp = (int *)(&section_get[0]);
		do
		{
			//Check regular expression
			if(NULL == section_get->templt_path || 0 != regcomp (&preg, section_get->templt_path, 0))
			{
				//TODO Выключить некорректный модуль, передать предупреждение.
				return NULL;
			}
			if(regexec(&preg, string, 0, NULL, 0) == 0)
			{
				module_name = section_get->name;
				return module_name;
				break;
			}
			// Shift the pointer to one element forward so as not to depart at the first iteration
			section_get=section_get->next;
		} while((int *)(&section_get[0]) != tmp);
	}
	
	return module_name;
}

// Print debug info
void contrDebug(const char* title, const char* data) {
    printf("<!DOCTYPE html>\
			<html>\
			<head><title>DEBUG INFO</title></head>\
			<body bgcolor=\"white\">\
			<center><h1>%s</h1></center>\
			<hr><center>%s</center>\
                        <hr><footer>%s</footerr></body>\
			</html>", title, data, VERSION);
}

// Content of admin panel
void contrAdmin(char* uri_path, char* cookie_str) {
	char *paramvalues[2];
	paramvalues[0] = uri_path;

	if(NULL != cookie_str)
	{
		paramvalues[1]=cookie_str;
	}
	//Продумать перенесение кода из БД, непосредственно, сюда.
	res = PQexecParams(conn, "SELECT * FROM contr_admin($1, $2);", 2, NULL, (const char **) paramvalues, NULL, NULL, 1);
	if(PQresultStatus(res)==PGRES_TUPLES_OK)
	{
		printf("<!DOCTYPE html>\
		<html><head><meta http-equiv=\"Content-type\" content=\"text/html\" charset=\"UTF-8\"/>");
		printf("%s\
		</body></html>", PQgetvalue(res, 0, 0), uri_path, uri_path, uri_path);
	}
	else
	{
		printf("<!DOCTYPE html>\
			<html>\
			<head><title>404 Not found</title></head>\
			<body bgcolor=\"white\">\
			<center><h1>404 Not found</h1></center>\
			<hr><center>%s</center></body>\
			</html>", VERSION);
	}

	PQclear(res);
}


// Content of modules
void contrModule (volatile struct section *section, char *query_string) {
	char *paramvalues[2];
	char *command;
	paramvalues[0] = query_string;

	command = malloc(strlen("SELECT * FROM ($1);") + strlen(section->name) + 1);
	sprintf(command, "SELECT * FROM %s($1);", section->name);
	//add_garb(command);
	
	syslog(LOG_DEBUG, "call:\"%s\"", command);
	//Делать вызовы обработки
	res = PQexecParams(conn, command, 1, NULL, (const char **)paramvalues, NULL, NULL, 1);
	if(PQresultStatus(res)==PGRES_TUPLES_OK)
	{
		printf("%s", PQgetvalue(res, 0, 0));
	}
	else
	{
		printf("<font color=#ff4511 size=\"+2\">Error2: Failed to retrieve data.</font>");
		syslog(LOG_DEBUG,"<font color=#ff4511 size=\"+2\">Error: Failed to retrieve data.</font>");
	}
	PQclear(res);
}

/* end *****************************************   CONTROLLERS PROGRAM CODE   */