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

/**************************** FUNCTION DECLARATION ****************************/

char* search_module(const char *uri_path);
void contrDebug(const char* title, const char* data);
void contrAdmin(char* uri_path, char* cookie_str);
void contrModule (volatile struct section *section, char *query_string);

/* end ************************************************ FUNCTION DECLARATION  */

#ifdef	__cplusplus
}
#endif

#endif	/* CONTROLLERS_H */

