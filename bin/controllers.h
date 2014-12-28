/* 
 * File:   controllers.h
 * Author: Semen Dubina <semen@sam002.net>
 *
 * Created on 19 Dec 2014
 */

#ifndef CONTROLLERS_H
#define	CONTROLLERS_H

#ifdef	__cplusplus
extern "C" {
#endif


#include <fcgi_stdio.h>		/* fcgi library; */
#include <stdio.h>		/* printf, fprintf */
#include <stdlib.h>		/* getenv, malloc */
#include <syslog.h>		/* syslog */
#include <libpq-fe.h>		/* PQ-functions */
#include <string.h>		/* strlen */
#include "config.h"
    
typedef struct commands
{
	const char *name;		/* function name */
	const char *cert;		/* certificate module */
} commands; // structure of commands

typedef struct section{
	char *name;			/* Section name */
	char *params;		/* Count of parameters */
	char *doctype;		/* Document type (default is "text/xml") */
	char *param_type;	/* Type of parametrs from envirement (default is "QUERY_STRING") */
	char *templt_path;	/* Path template ("/" - default is root) */
	struct section *next;		/* Pointer on next section */
	struct commands *commands;	/* Pointer on array commands from section */
} section; // structure of commands

// Set structure for all parametrs
typedef struct keyvalue{
    char *name;
    char *value;
} keyvalue;

#define NOT_NULL_KEYVALUE(KV)  ( KV.name!=NULL) ? ( KV.value!=NULL ) : 0

/****************************** GLOBAL VARIABLES ******************************/

volatile section *section_get;
volatile section *section_post;

/* Common variables for postgres */
PGconn *conn;
PGresult *res;

/* end ***************************************************** GLOBAL VARIABLES */



/**************************** FUNCTION DECLARATION ****************************/

char* search_module(const char *uri_path);
void contrDebug(const char* title, keyvalue *query_string, keyvalue *cookies);
void contrAdmin(char* uri_path, keyvalue *query_string, keyvalue *cookies);
void contrModule (volatile struct section *section, keyvalue *query_string, keyvalue *cookies);

/* end ************************************************ FUNCTION DECLARATION  */

#ifdef	__cplusplus
}
#endif

#endif	/* CONTROLLERS_H */

