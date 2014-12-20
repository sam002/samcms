/* 
 * Project: samcms
 * File:   index.h
 * Author: Semen Dubina <semen@sam002.net>
 *
 * Created on 19 Dec 2014
 */

#ifndef INDEX_H
#define	INDEX_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <sys/types.h>		/* getpwnam, setuid */
#include <pwd.h>		/* getpwnam */
#include <unistd.h>		/* setuid */
#include <sys/prctl.h>		/* prctl */
    
#include "cache.h"
#include "controllers.h"
#include "router.h"
#include "views.h"
#include "config.h"
    

void add_garb(void* ptr);

#ifdef	__cplusplus
}
#endif

#endif	/* INDEX_H */

