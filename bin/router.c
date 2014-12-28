/* 
 * File:   router.c
 * Author: Semen Dubina <semen@sam002.net>
 *
 * Created on 19 Dec 2014
 */


#include "controllers.h"
#include "router.h"

#define TYPE_UNDEFINED 'U'
#define TYPE_GET 'G'
#define TYPE_HEAD 'H'
#define TYPE_POST 'P'

/**************************** FUNCTION DECLARATION ****************************/

char from_hex(char ch);
/* Recoding of interest encoding */
char *urldecode(char *str);
int getTypeCode(char *type);
int parseGet(const char* path, keyvalue *query_string, keyvalue *cookies);

/* end ************************************************ FUNCTION DECLARATION  */


/********************************** ROUTER CODE *******************************/

int routeQuery(char *type, char *path, char *query, char *cookies) {
    char *request_uri = path;
    keyvalue *query_string = parseKeyValueString(query, '&', '=');
    keyvalue *query_cookies = parseKeyValueString(cookies, '&', '=');

    switch (getTypeCode(type)) {
        case TYPE_GET:
            // Parse GET method
#ifdef DEBUG
            //If try path "/debug" and DEBUG enabled..
            if (strcasecmp(request_uri, "/debug") == 0) {
                printf("Echo dbg ->\n\n");
                contrDebug(request_uri, query_string, query_cookies);
                return 0;
            } else
#endif
                //parseGet(request_uri, query_string, query_cookies);
            break;
        case TYPE_HEAD:
            // Parse HEAD mathod (as GET, but only headers answer)
            break;
        case TYPE_POST:
            // Parse POST method
            break;
        default:
            break;
    }
    return 1;
};

int getTypeCode(char *type){
    if(strcasecmp(type, "GET") == 0){
        return TYPE_GET;
    } else if(strcasecmp(type, "HEAD") == 0){
        return TYPE_HEAD;
    } else if(strcasecmp(type, "POST") == 0){
        return TYPE_POST;
    }
    return TYPE_UNDEFINED;
}

int parseGet(const char* path, keyvalue *query_string, keyvalue *cookies) {
    
    path++;

    // If "query string" is present, parse as functions of modules
    if (query_string != NULL) {
        //FIXME Testing new function
        char *return_name = search_module(path);
        if (return_name != NULL) {
            //continue;
        }
        //FIXME End of test
        int *tmp = (int *) (&section_get[0]);
        do {
            if (strncmp(path, section_get->name, strlen(section_get->name)) == 0) {
                printf("Content-Type: %s\n\n", section_get->doctype);
                contrModule(section_get, query_string, cookies);
                break;
            }
            //Сместим уазатель на один элемент вперёд, чтобы не вылетать при первой же итерации
            section_get = section_get->next;
        } while ((int *) (&section_get[0]) != tmp);
    }        //При пустом q_s делаем обработку запроса по URI 
    else {
        //Запрос внутренних страниц
        if (strlen(path) > 1) {
            printf("Content-Type: text/html\n\n");
            //dec_str = urldecode(request_uri);
            contrAdmin("admin", query_string, cookies);
        }            //Наполнение "главной".
        else {

            printf("Content-Type: text/html\n\n");
            contrAdmin("get_default", query_string, cookies);
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
    char delimiter_key_value[3];
    int max_params = 1024;
    keyvalue *res;
        
    delimiter_key_value[0] = delimiter_key;
    delimiter_key_value[1] = delimiter_value;
    delimiter_key_value[2] = '\0';
    
    if (string != NULL && strlen(string) > 0) {
        char *pt = strtok(string, delimiter_key_value);

        res = (keyvalue*) malloc(sizeof(struct keyvalue));
        res[0].name = (char *) malloc(strlen(pt) + 1);
        strcpy(res[0].name, pt);
        res[0].value = (char *) malloc(strlen(pt) + 1);
        pt = strtok(NULL, delimiter_key_value);
        strcpy(res[0].value, pt);

        pt = strtok(NULL, delimiter_key_value);
        int i = 0;
        for (i = 0; pt != NULL && i < (max_params - 1); i++, pt = strtok(NULL, delimiter_key_value)) {
            res = (keyvalue*) realloc(res, sizeof (struct keyvalue)*(2 + i));
            res[i+1].name = (char *) malloc(strlen(pt) + 1);
            strcpy(res[i+1].name, pt);
            res[i+1].value = (char *) malloc(strlen(pt) + 1);
            pt = strtok(NULL, delimiter_key_value);
            strcpy(res[i+1].value, pt);
        };
        res = (keyvalue*) realloc(res, sizeof (struct keyvalue)*(2 + i));
        res[i+1].name = NULL;
        res[i+1].value = NULL;
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