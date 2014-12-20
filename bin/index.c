/* 
 * File:   router.c
 * Author: Semen Dubina <semen@sam002.net>
 *
 * Created on 11 Jul 2012
 */

#include "index.h"
//FIXME
#include <sys/utsname.h>	/* uname */
#include <getopt.h>

#define DEBUG
//#undef DEEBUG

/* CONFIGURE */
#define VERSION			"samcms version 0.1"
#define CONNINFO		"dbname='samcms' user='samcms' password='qwerty' hostaddr='127.0.0.1' port='5432'"	// connection settings
#define DEFAULT_SECTION_NAME	NULL			// Name signaling element
#define DEBUG						// enable debug


/****************************** GLOBAL VARIABLES ******************************/

/* Common variables for postgres */
PGconn *conn;
PGresult *res;

volatile section *section_get;
volatile section *section_post;

//Glogal GC
void **garb = NULL;

/* end ***************************************************** GLOBAL VARIABLES */



/**************************** FUNCTION DECLARATION ****************************/
int bindFCGI(void);
void add_garb(void* ptr);
int initialise(const char *conninfo);
void free_garb(void);
char from_hex(char ch);
/* end ************************************************ FUNCTION DECLARATION  */


/**************************   MAIN PROGRAM CODE   *****************************/

int bindFCGI() {
    char *query_type = NULL;
    char *query_string = NULL;
    char *request_uri = NULL;
    
    while (FCGI_Accept() >= 0) {

        query_type = getenv("REQUEST_METHOD");
            query_string = getenv("QUERY_STRING");
            request_uri = getenv("DOCUMENT_URI");
        //Processing query
        routeQuery(query_type, request_uri, query_string);

    }
    return 1;
}

//Функция инициализации перед принятием запросов.
int initialise(const char *conninfo)
{
	unsigned char err=0;
	PGresult *init_res=NULL;
	unsigned short num = 0, i;
	char *paramvalues[6];
	char tmp_buf[256];

	syslog(LOG_DEBUG,"start initialize\n");

	/* Соединяемся с базой */
	conn = PQconnectdb(conninfo);
	if(PQstatus(conn) != CONNECTION_OK) {
		syslog(LOG_DEBUG,"error connect\n");
		PQfinish(conn);
		return ++err;
	}
	/* При удачном соединении запрашиваем массив всех возможных функций обработки*/
	else
	{
		{
			prctl(PR_GET_NAME, tmp_buf);
			paramvalues[0] = (char*)malloc(strlen(tmp_buf)+1);
			add_garb(paramvalues[0]);
			memcpy(paramvalues[0], tmp_buf, strlen(tmp_buf)+1);
		}
		{
			pid_t pid;
			pid=getpid();
			sprintf(tmp_buf, "%d", pid);
			paramvalues[1]=(char*)malloc(strlen(tmp_buf)+1);
			add_garb(paramvalues[1]);
			memcpy(paramvalues[1], tmp_buf, strlen(tmp_buf)+1);
		}
		{
			struct utsname buf;
			uname(&buf);
			
			paramvalues[2]=(char*)malloc(strlen(buf.nodename)+1);
			add_garb(paramvalues[2]);
			memcpy(paramvalues[2], buf.nodename, strlen(buf.nodename)+1);
			
			paramvalues[3]=(char*)malloc(sizeof(buf) - strlen(buf.nodename)+1);
			add_garb(paramvalues[3]);
			sprintf(paramvalues[3],"%s %s %s %s", buf.sysname, buf.release, buf.version, buf.machine);
		}
		{
			strcpy(tmp_buf, VERSION);
			paramvalues[4]=(char*)malloc(strlen(tmp_buf)+1);
			add_garb(paramvalues[4]);
			memcpy(paramvalues[4], tmp_buf, strlen(tmp_buf)+1);
		}

		paramvalues[5]="127.0.0.1";
		
		init_res = PQexecParams(conn, "SELECT * FROM initialize($1, $2, $3, $4, $5, $6)", 6, NULL, (const char **)paramvalues, NULL, NULL, 0);
		
		num = PQntuples(init_res);

		if(PQresultStatus(init_res)==PGRES_TUPLES_OK && init_res != NULL)
		{
		/* .next использовать для работы с многими потоками */
			section_get = (section*)malloc(sizeof(struct section)*(num+1));
			add_garb((void *)section_get);
			//Заполнение начальной структуры
			section_get[0].name = malloc(sizeof(VERSION));
			add_garb(section_get[0].name);
			section_get[0].name = VERSION;
			section_get[0].commands = NULL;
			section_get[0].params = NULL;
			section_get[0].doctype = NULL;
			section_get[0].templt_path = "/";
			section_get[0].next = (section*)&(section_get[0]);
			for(i = 1; i <= num; i++)
			{
				// получение имени структуры
				section_get[i].name = PQgetvalue(init_res, i-1, 0);
				// заполнение структур переменных
				section_get[i].params = PQgetvalue(init_res, i-1, 1);
				section_get[i].doctype = PQgetvalue(init_res, i-1, 2);
				section_get[i].param_type = PQgetvalue(init_res, i-1, 3);
				section_get[i].templt_path = PQgetvalue(init_res, i-1, 4);
				//section_commands[i].commands = add_commands(&tmp_buf, &tmp_size);
				// ставим указатель на следующую структуру (в предыдущей структуре) на текущую
				section_get[i-1].next = (section*)&(section_get[i]);
			}
			// если в файле есть описание структур, то последняя структура должна замыкаться на сигнальную
			// получаем кольцо из указателей, что поможет избежать ошибок сегментирования
			if(num > 0)
			{
				section_get[i-1].next = (section*)&(section_get[0]);
				section_get = section_get->next;
			}
			else
			{
				fputs("cann't find structure\n", stderr);
			}
		}
		PQclear(res);
	}
	return err;
}

//Функция добавления в массив сборщика мусора
void add_garb(void *ptr)
{
	//проверяем начальное значение, выделяем память под массив
	if (garb==NULL)
	{
		garb = malloc(sizeof(void*));
		garb[0] = malloc(sizeof(unsigned int));
		//нулевой элемент массива играет роль счётчика длины
		//устанавливаем в единицу, инициализируем
		*((unsigned int*)garb[0]) = 1;
	}
	if(ptr != NULL)
	{
		syslog(LOG_DEBUG, "ptr is %p", ptr);
		//увеличиваем счётчик элементов;
		*((unsigned int*)garb[0]) += 1;

		//перевыделяем память под принятый указатель
		garb = realloc((void*)garb, sizeof(void*) * *((unsigned int*)garb[0]) );
		//проверка выделения памяти
		if(garb == NULL) 
		{
			syslog(LOG_DEBUG,"Memory error in samcms (add_garb)");
		}
		
		//добавление указателя в конец массива
		garb[*((unsigned int*)garb[0]) - 1] = ptr;
	}
	else
	{
		syslog(LOG_DEBUG, "!!!ptr is NULL (add_garb)");
	}
}

void free_garb(void)
{
	//перебираем массив указателей по счётчику
	while(*((unsigned int*)garb[0]) != 1)
	{
		//уменьшаем счётик
		--*((unsigned int*)garb[0]);
		//освобождаем память
		free((void*)garb[*((unsigned int*)garb[0])]);
	}

	//освобождаем память массива указателей
	free(garb);
	//поможет избежать ошибок сегментации при повторных вызовах
	garb = NULL;
}

int main(int argc, char **argv)
{
	char *conninfo;
	char *username = NULL, *addr = NULL, *database = NULL, *password = NULL;
	char *endptr = NULL;
	unsigned short port = 0;
	int o;

	// register the function performed by the exit.
	if(atexit(free_garb) != 0 )
	{
		fputs ("Reading error",stderr);
		free_garb();
		return (1);
	}

	conninfo = malloc(1);
	while (-1 != (o = getopt(argc, argv, "a:p:d:u:w:"))) {
		switch(o) {
		case 'a': addr = optarg; /* hostaddr */
			conninfo=realloc(conninfo, strlen(conninfo)+strlen(addr)+12);
			sprintf(conninfo, "%shostaddr='%s' ", conninfo, addr);
			break;
		case 'p': port = strtol(optarg, &endptr, 10); /* port */
			if (*endptr) {
				fprintf(stderr, "index: invalid port: %u\n", (unsigned int) port);
				return -1;
			}
			else {
				conninfo=realloc(conninfo, strlen(conninfo)+13);
				sprintf(conninfo, "%sport='%i' ", conninfo, port);
			}
			break;
		case 'd': database = optarg; /* database */
			conninfo=realloc(conninfo, strlen(conninfo)+strlen(database)+10);
			sprintf(conninfo, "%sdbname='%s' ", conninfo, database);
			break;
		case 'u': username = optarg; /* username */
			conninfo=realloc(conninfo, strlen(conninfo)+strlen(username)+8);
			sprintf(conninfo, "%suser='%s' ", conninfo, username);
			break;
		case 'w': password = optarg; /* password */
			conninfo=realloc(conninfo, strlen(conninfo)+strlen(password)+12);
			sprintf(conninfo, "%spassword='%s' ", conninfo, password);
			break;
		default:
			conninfo=CONNINFO;
		}
	}
	if(0 != initialise(conninfo))
	{
		return -1;
	}
        
	/* Binding. It's main part of the program*/
	bindFCGI();

	PQfinish(conn);
#ifdef DEBUG
	syslog(LOG_DEBUG,"cancel fcgi_accept and exit");
#endif
	free_garb();
	return 0;
}

/* end ************************************************   MAIN PROGRAM CODE   */
