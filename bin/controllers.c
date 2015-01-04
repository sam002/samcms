/* 
 * File:   controllers.h
 * Author: Semen Dubina <semen@sam002.net>
 *
 * Created on 19 Dec 2014
 */


#include <regex.h>
#include "controllers.h"



/**********************   CONTROLLERS PROGRAM CODE   **************************/


char* search_module(const char *uri_path) {
	char* module_name = NULL;
	regex_t preg;
	
	char* string = NULL;
	
	if(uri_path == NULL){
		return NULL;
	} else {
	string = malloc(strlen(uri_path));
	strcat(string, uri_path);

	// If path correct search in structure array, compare with template
		int *tmp = (int *)(&section_get[0]);
		do {
			//Check regular expression
			if(NULL == section_get->templt_path || 0 != regcomp (&preg, section_get->templt_path, 0)) {
				//TODO Выключить некорректный модуль, передать предупреждение.
				return NULL;
			}
			if(regexec(&preg, string, 0, NULL, 0) == 0) {
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

void contrDebug(const char* title, keyvalue *query_string, keyvalue *cookies) {
    printf("<!DOCTYPE html>\
			<html>\
			<head><title>DEBUG INFO</title>\
                        <style type=\"text/css\">\
body {background-color: #ffffff; color: #000000;}\
body, td, th, h1, h2 {font-family: sans-serif;}\
pre {margin: 0px; font-family: monospace;}\
a:link {color: #000099; text-decoration: none; background-color: #ffffff;}\
a:hover {text-decoration: underline;}\
table {border-collapse: collapse;}\
.center {text-align: center;}\
.center table { margin-left: auto; margin-right: auto; text-align: left;}\
.center th { text-align: center !important; }\
td, th { border: 1px solid #000000; font-size: 75%%; vertical-align: baseline;}\
h1 {font-size: 150%%;}\
h2 {font-size: 125%%;}\
.p {text-align: left;}\
.e {background-color: #ccccff; font-weight: bold; color: #000000;}\
.h {background-color: #9999cc; font-weight: bold; color: #000000;}\
.v {background-color: #cccccc; color: #000000;}\
.vr {background-color: #cccccc; text-align: right; color: #000000;}\
img {float: right; border: 0px;}\
hr {width: 600px; background-color: #cccccc; border: 0px; height: 1px; color: #000000;}\
</style></head>\n\
<body bgcolor=\"white\">\
<div class=\"center\">\
<table border=\"0\" cellpadding=\"3\" width=\"600\">\
<tr class=\"h\"><td>\
<a href=\"https://github.com/sam002/samcms\">\
<img style=\"position: absolute; top: 0; right: 0; border: 0;\" src=\"https://camo.githubusercontent.com/38ef81f8aca64bb9a64448d0d70f1308ef5341ab/68747470733a2f2f73332e616d617a6f6e6177732e636f6d2f6769746875622f726962626f6e732f666f726b6d655f72696768745f6461726b626c75655f3132313632312e706e67\" alt=\"Fork me on GitHub\" data-canonical-src=\"https://s3.amazonaws.com/github/ribbons/forkme_right_darkblue_121621.png\"></a>\
<a href=\"https://github.com/sam002/samcms\">%s</a><h1 class=\"p\">%s</h1>\
</td></tr>\
</table><br />\n", VERSION, title);

    printf("<h2><a name='env'>Environment</a></h2>\n");
    extern char **environ;
    char *s = *environ;
    for (int i = 0; s; i++) {
        printf("<hr><center>%s</center>\n", s);
        s = *(environ + i);
    }

    printf("<h2>Query String</h2>");
    
    for (int i = 0; NOT_NULL_KEYVALUE(query_string[i]); i++) {
        printf("<hr><center><b>[%s]</b>=>\"%s\"</center>\n", query_string[i].name, query_string[i].value);
    }

    /*printf("<h2>COOKIES</h2>");
    for (int i = 0; NOT_NULL_KEYVALUE(cookies[i]); i++) {
        printf("<hr><center><b>[%s]</b>=>\"%s\"</center>\n", cookies[i].name, cookies[i].value);
    }*/

    printf("<hr><footer>%s</footerr></body>\
			</html>\n\n", VERSION);
}

// Content of admin panel
void contrAdmin(char* uri_path, keyvalue *query_string, keyvalue *cookies) {
	char *paramvalues[2];
	paramvalues[0] = uri_path;

	if(NULL != cookies) {
		paramvalues[1]=cookies;
	}
	//Продумать перенесение кода из БД, непосредственно, сюда.
	res = PQexecParams(conn, "SELECT * FROM contr_admin($1, $2);", 2, NULL, (const char **) paramvalues, NULL, NULL, 1);
	if(PQresultStatus(res)==PGRES_TUPLES_OK) {
		printf("<!DOCTYPE html>\
		<html><head><meta http-equiv=\"Content-type\" content=\"text/html\" charset=\"UTF-8\"/>");
		printf("%s\
		</body></html>", PQgetvalue(res, 0, 0), uri_path, uri_path, uri_path);
	} else {
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
void contrModule (volatile struct section *section, keyvalue *query_string, keyvalue *cookies) {
	char *paramvalues[2];
	char *command;
	paramvalues[0] = query_string[0].value;

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