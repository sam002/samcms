/* 
 * File:   config.h
 * Author: Semen Dubina <semen@sam002.net>
 *
 * Created on 21 Dec 2014
 */

#ifndef CONFIG_H
#define	CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif


/* CONFIGURE */
#define VERSION			"samcms version 0.1"
#define CONNINFO		"dbname='samcms' user='samcms' password='qwerty' hostaddr='127.0.0.1' port='5432'"	// connection settings
#define DEFAULT_SECTION_NAME	NULL			// Name signaling element
#define DEBUG						// enable debug


#ifdef	__cplusplus
}
#endif

#endif	/* CONFIG_H */

