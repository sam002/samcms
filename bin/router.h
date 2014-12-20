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
    
// Set temporary structure for all pqrqmetrs
typedef struct query_params{
    char *name;
    char *value;
} query_params;

/**************************** FUNCTION DECLARATION ****************************/

int routeQuery(char *type, char *path, query_params *query);
query_params* parseQeryString(char* query_string);

/* end ************************************************ FUNCTION DECLARATION  */

#ifdef	__cplusplus
}
#endif

#endif	/* ROUTER_H */

