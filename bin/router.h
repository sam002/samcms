/* 
 * File:   router.h
 * Author: Semen Dubina <semen@sam002.net>
 *
 * Created on 19 Dec 2014
 */

#ifndef ROUTER_H
#define	ROUTER_H

#ifdef	__cplusplus
extern "C" {
#endif

    
#include <fcgi_stdio.h>		/* fcgi library; */
#include <stdio.h>		/* printf, fprintf */
#include <stdlib.h>		/* getenv, malloc */
#include <strings.h>		/* strcasecmp */
#include <string.h>		/* strlen */
#include <ctype.h>		/* isdigit, tolower*/
    
    
#include <sys/types.h>		/* getpwnam, setuid */
#include <pwd.h>		/* getpwnam */
#include <unistd.h>		/* setuid */
#include <libpq-fe.h>		/* PQ-functions */
#include <syslog.h>		/* syslog */
#include <sys/prctl.h>		/* prctl */
#include "config.h"
    
// Set temporary structure for all pqrqmetrs
typedef struct keyvalue{
    char *name;
    char *value;
} keyvalue;

/**************************** FUNCTION DECLARATION ****************************/

int routeQuery(char *type, char *path, char *query, char *cookies);
keyvalue* parseKeyValueString(char* string, char delimiter_key, char delimiter_value);

/* end ************************************************ FUNCTION DECLARATION  */

#ifdef	__cplusplus
}
#endif

#endif	/* ROUTER_H */