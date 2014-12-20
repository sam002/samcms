/* 
 * File:   cache.c
 * Author: Semen Dubina <semen@sam002.net>
 *
 * Created on 19 Dec 2014
 */


/************************ PREVENTIVE CACHING REQUESTS *************************/
/*
* Отдельный поток для кеширования запросов. Сохраняет соответствие URI и
* шаблона пути модуля. При повторном обращении по одному и тому же пути
* возвращает, сохранённое при первом обращении, имя вызываемого модуля.
* Для оптимизации составляем дерево, ako индексирование.
*/

//temp structure, processed by a separate thread. Is transformed into a tree.
typedef struct uricache
{
	const char *name;		/* function name */
	const char *path;		/* path in URI query */
}uricache; // cache structure

/* end ****************************************** PREVENTIVE CACHING REQUESTS */