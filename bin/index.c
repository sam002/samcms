#include <fcgi_stdio.h>		/* fcgi library; */
#include <stdio.h>		/* printf, fprintf */
#include <stdlib.h>		/* getenv, malloc */
#include <ctype.h>		/* isdigit, tolower*/
#include <sys/types.h>		/* getpwnam, setuid */
#include <pwd.h>		/* getpwnam */
#include <unistd.h>		/* setuid */
#include <libpq-fe.h>		/* PQ-functions */
#include <string.h>		/* strlen */
#include <strings.h>		/* strcasecmp */
#include <syslog.h>		/* syslog */
#include <sys/prctl.h>		/* prctl */
#include <regex.h>
//FIXME
#include <sys/utsname.h>	/* uname */
#include <getopt.h>

#define DEBUG
//#undef DEEBUG

/* Настройки */
#define VERSION			"samcms version 0.1"
#define CONNINFO		"dbname='samcms' user='samcms' password='qwerty' hostaddr='127.0.0.1' port='5432'"	// параметры подключения
#define DEFAULT_SECTION_NAME	NULL			// Имя сигнального элемента
#define DEBUG						// включить отладку


/******************** ГЛОАЛЬНЫЕ ПЕРЕМЕННЫЕ ********************/

/* Общие переменные для postgres */
PGconn *conn;
PGresult *res;

typedef struct commands
{
	const char *name;		/* имя функции */
	const char *cert;		/* сертификат модуля */
}commands; //структура для команд

typedef struct section{
	char *name;			/* Имя секции */
	char *params;		/* Количество параметров */
	char *doctype;		/* Тип документа ("text/xml" по умолчании) */
	char *param_type;	/* Тип параметров из окружения ("QUERY_STRING" по умолчании) */
	char *templt_path;	/* Шаблон пути ("/" - корень по умолчании) */
	struct section *next;		/* Указатель на следующую структуру */
	struct commands *commands;	/* Указатель на массив переменных из секции */
} section; //структура для команд

volatile section *section_get;
volatile section *section_post;

//глобальный сборщик мусора
void **garb = NULL;

/* конец ***************************** ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ */



/******************** ОБЪЯВЛЕНИЕ ФУНКЦИЙ ********************/
void add_garb(void* ptr);
int initialise(const char *conninfo);
void free_garb(void);
char from_hex(char ch);
/* конец ******************************* ОБЪЯВЛЕНИЕ ФУНКЦИЙ */



/******************** ПРЕВЕНТИВНОЕ КЕШИРОВАНИЕ ЗАПРОСОВ ********************/
/*
* Отдельный поток для кеширования запросов. Сохраняет соответствие URI и
* шаблона пути модуля. При повторном обращении по одному и тому же пути
* возвращает, сохранённое при первом обращении, имя вызываемого модуля.
* Для оптимизации составляем дерево, ako индексирование.
*/

//временная структура, обрабатываемая отдельным потоком. Преобразуется в дерево.
typedef struct uricache
{
	const char *name;		/* имя функции */
	const char *path;		/* путь в URI запросе */
}uricache; //структура кеша

char* search_module(const char *uri_path){
	char* module_name = NULL;
	regex_t preg;
	
	char* string = NULL;
	
	if(uri_path == NULL){
		return NULL;
	}
	else{
	
	string = malloc(strlen(uri_path));
	strcat(string, uri_path);
	
	//Если путь корректный, то пробежимся по структуре модулей, Сравнивая с шаблоном
		int *tmp = (int *)(&section_get[0]);
		do
		{
			//Проверка корректности регулярного выражения
			if(NULL == section_get->templt_path || 0 != regcomp (&preg, section_get->templt_path, 0))
			{
				//TODO Выключить некорректный модуль, передать предупреждение.
				return NULL;
			}
			if(regexec(&preg, string, 0, NULL, 0) == 0)
			{
				module_name = section_get->name;
				return module_name;
				break;
			}
			//Сместим указатель на один элемент вперёд, чтобы не вылетать при первой же итерации
			section_get=section_get->next;
		} while((int *)(&section_get[0]) != tmp);
	}
	
	return module_name;
}

/* конец ******************************* ОБЪЯВЛЕНИЕ ФУНКЦИЙ */



/********************   КОД ОСНОВНОЙ ПРОГРАММЫ   ********************/
// Наполнение главной
void contr_default(char* uri_path, char* cookie_str) {
	char *paramvalues[2];
	paramvalues[0] = uri_path;

	if(NULL != cookie_str)
	{
		paramvalues[1]=cookie_str;
	}
	//Продумать перенесение кода из БД, непосредственно, сюда.
	res = PQexecParams(conn, "SELECT * FROM contr_default($1, $2);", 2, NULL, (const char **) paramvalues, NULL, NULL, 1);
	if(PQresultStatus(res)==PGRES_TUPLES_OK)
	{
		printf("<!DOCTYPE html>\
		<html><head><meta http-equiv=\"Content-type\" content=\"text/html\" charset=\"UTF-8\"/>");
		printf("%s\
		</body></html>", PQgetvalue(res, 0, 0), uri_path, uri_path, uri_path);
	}
	else
	{
		printf("<!DOCTYPE html>\
			<html>\
			<head><title>404 Not found</title></head>\
			<body bgcolor=\"white\">\
			<center><h1>404 Not found</h1></center>\
			<hr><center>%s</center></body>\
			</html>", VERSION);
	}

	PQclear(res);
}

void contr_mod (volatile struct section *section, char *query_string) {
	char *paramvalues[2];
	char *command;
	paramvalues[0] = query_string;

	command = malloc(strlen("SELECT * FROM ($1);") + strlen(section->name) + 1);
	sprintf(command, "SELECT * FROM %s($1);", section->name);
	add_garb(command);
	
	syslog(LOG_DEBUG, "call:\"%s\"", command);
	//Делать вызовы обработки
	res = PQexecParams(conn, command, 1, NULL, (const char **)paramvalues, NULL, NULL, 1);
	if(PQresultStatus(res)==PGRES_TUPLES_OK)
	{
		printf("%s", PQgetvalue(res, 0, 0));
	}
	else
	{
		printf("<font color=#ff4511 size=\"+2\">Error2: Failed to retrieve data.</font>");
		syslog(LOG_DEBUG,"<font color=#ff4511 size=\"+2\">Error: Failed to retrieve data.</font>");
	}
	PQclear(res);
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
	char *query_string = NULL;
	char *request_uri = NULL;
	char *dec_str = NULL;

	//регистрируем функцию, выполняемую по выходу.
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
		case 'p': port = strtol(optarg, &endptr, 10);/* port */
			if (*endptr) {
				fprintf(stderr, "index: invalid port: %u\n", (unsigned int) port);
				return -1;
			}
			else {
				conninfo=realloc(conninfo, strlen(conninfo)+13);
				sprintf(conninfo, "%sport='%i' ", conninfo, port);
			}
			break;
		case 'd': database = optarg;/* database */
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
	/* Активное ожидание */
	while (FCGI_Accept() >= 0)
	{
		//Обработка запросов

//Разбор GET
		if(strcasecmp(getenv("REQUEST_METHOD"), "GET") == 0)
		{
			query_string = getenv("QUERY_STRING");
			request_uri = getenv("DOCUMENT_URI");
			request_uri++;
			
			//При наличии q_s считаем, что запрос для обработки модулями
			if(query_string != NULL && strlen(query_string) > 0)
			{
				//FIXME Тестирование новой функции!
				char *return_name = search_module(request_uri);
				if (return_name != NULL) {continue;}
				//FIXME Конец тестирования.
				int *tmp = (int *)(&section_get[0]);
				do
				{
					if(strncmp(request_uri, section_get->name, strlen(section_get->name)) == 0)
					{
						printf("Content-Type: %s\n\n", section_get->doctype);
						//printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
						contr_mod(section_get, urldecode(query_string));
						break;
					}
					//Сместим уазатель на один элемент вперёд, чтобы не вылетать при первой же итерации
					section_get=section_get->next;
				} while((int *)(&section_get[0]) != tmp);
			}
			//При пустом q_s делаем обработку запроса по URI 
			else
			{
				char* cookie_str = cookie_str=getenv("HTTP_COOKIE");
				//Запрос внутренних страниц
				if(strlen(request_uri) > 1)
				{
					printf("Content-Type: text/html\n\n");
					//dec_str = urldecode(request_uri);
					contr_default(request_uri, cookie_str);
				}
				//Наполнение "главной".
				else
				{
					
					printf("Content-Type: text/html\n\n");
					contr_default("get_default", cookie_str);
				}
			}
		}
		else
		{
			if(strcasecmp(getenv("REQUEST_METHOD"), "POST") == 0);
			{
				
			}
		}
	}

	PQfinish(conn);
#ifdef DEBUG
	syslog(LOG_DEBUG,"cancel fcgi_accept and exit");
#endif
	free_garb();
	return 0;
}

/* конец *******************************   КОД  ПРОГРАММЫ   */
