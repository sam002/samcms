/* 
 * File:   router.c
 * Author: Semen Dubina <semen@sam002.net>
 *
 * Created on 19 Dec 2014
 */

#include "router.h"
#include "controllers.h"

/**************************** FUNCTION DECLARATION ****************************/

char from_hex(char ch);
/* Перекодировка из процентной кодировки */
char *urldecode(char *str);

/* end ************************************************ FUNCTION DECLARATION  */



/********************************** ROUTER CODE *******************************/

int routeQuery(char *type, char *path, char *query, char *cookies){
    char *request_uri = NULL;
    char *query_string = NULL;
    char *query_cookies = NULL;
    
        // Parse GET
       if (strcasecmp(type, "GET") == 0) {
            query_string = getenv("QUERY_STRING");
            request_uri = getenv("DOCUMENT_URI");
            request_uri++;
            //Processing query
            printf("TEST_2, %s and %s\n", request_uri, query_string);
            //contrDebug(request_uri, query_string);
            return 0;
        }
    return 1;
};

int parseGet() {
    char *query_string;
    char *request_uri;
    
    // Parse GET
    if (strcasecmp(getenv("REQUEST_METHOD"), "GET") == 0) {
        query_string = getenv("QUERY_STRING");
        request_uri = getenv("DOCUMENT_URI");
        request_uri++;
        //Processing query
    }

    // If "query string" is present, parse as functions of modules
    if (query_string != NULL && strlen(query_string) > 0) {
        //FIXME Testing new function
        char *return_name = search_module(request_uri);
        if (return_name != NULL) {
            //continue;
        }
        //FIXME End of test
        int *tmp = (int *) (&section_get[0]);
        do {
            if (strncmp(request_uri, section_get->name, strlen(section_get->name)) == 0) {
                printf("Content-Type: %s\n\n", section_get->doctype);
                contrModule(section_get, urldecode(query_string));
                break;
            }
            //Сместим уазатель на один элемент вперёд, чтобы не вылетать при первой же итерации
            section_get = section_get->next;
        } while ((int *) (&section_get[0]) != tmp);
    }        //При пустом q_s делаем обработку запроса по URI 
    else {
        char* cookie_str = cookie_str = getenv("HTTP_COOKIE");
        //Запрос внутренних страниц
        if (strlen(request_uri) > 1) {
            printf("Content-Type: text/html\n\n");
            //dec_str = urldecode(request_uri);
            contrAdmin(request_uri, cookie_str);
        }            //Наполнение "главной".
        else {

            printf("Content-Type: text/html\n\n");
            contrAdmin("get_default", cookie_str);
        }
    }
    return 0;
}


/* HEX2CHR */
char from_hex(char ch) {
	return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Перекодировка из процентной кодировки */
char *urldecode(char *str) {
	char *pstr = str, *buf = malloc(strlen(str) + 1), *pbuf = buf;

	while (*pstr) {
		if (*pstr == '%') {
			if (pstr[1] && pstr[2]) {
				*pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
				pstr += 2;
			}
		} else if (*pstr == '+') {
			*pbuf++ = ' ';
		} else {
			*pbuf++ = *pstr;
		}
		pstr++;
	}
	*pbuf = '\0';

	return buf;
}

keyvalue* parseKeyValueString(char* string, char delimiter_key, char delimiter_value){
    const char delimetr_key_value[]={delimiter_key, delimiter_value, '\0'};
    int max_params = 1024;
    keyvalue *res;
        
    if (string != NULL && strlen(string) > 0) {

        char *pt = strtok(string, delimetr_key_value);

        res = (keyvalue*) malloc(sizeof (struct keyvalue));
        res[0].name = (char *) malloc(strlen(pt) + 1);
        strcpy(res[0].name, pt);
        res[0].value = (char *) malloc(strlen(pt) + 1);
        pt = strtok(NULL, delimetr_key_value);
        strcpy(res[0].value, pt);

         pt = strtok(NULL, delimetr_key_value);
        for (int i = 0; pt != NULL && i < (max_params - 1); i++, pt = strtok(NULL, delimetr_key_value)) {
            res = (keyvalue*) realloc(res, sizeof (struct keyvalue)*(2 + i));
            res[i+1].name = (char *) malloc(strlen(pt) + 1);
            strcpy(res[i+1].name, pt);
            res[i+1].value = (char *) malloc(strlen(pt) + 1);
            pt = strtok(NULL, delimetr_key_value);
            strcpy(res[i+1].value, pt);
        };
     return  res;  
    }
    return NULL;
}

int parsePost() {
    if (strcasecmp(getenv("REQUEST_METHOD"), "POST") == 0);
    {
        return 0;
    }
}

/* end ********************************************************** ROUTER CODE */