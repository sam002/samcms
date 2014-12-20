CC = gcc
#TARGET=$(shell basename `pwd`)
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:%.c=%.o)
PROGRAMS=samcms-index samcms
#TARGET+=$(PROGRAMS)

CFLAGS += -O0
LDFLAGS += -lpq -lfcgi

CFLAGS_INDEX = -I/usr/include/postgresql -L/usr/lib/postgresql/9.1/lib -Wall -std=c99
CFLAGS_SAMCMS = 

all: $(PROGRAMS) #${TARGET}

#$(OBJECTS): $(SOURCES)

#$(TARGET): $(OBJECTS)

samcms-index:
	$(CC) index.c router.c controllers.c views.c $(LDFLAGS) $(CFLAGS) $(CFLAGS_INDEX) -o samcms-index

#samcms-index: index.o router.o controllers.o views.o
#	$(CC) index.o router.o controllers.o views.o $(LDFLAGS) $(CFLAGS) $(CFLAGS_INDEX) -o samcms-index
#
#index.o:
#	$(CC) index.c $(LDFLAGS) $(CFLAGS) $(CFLAGS_INDEX) -o index.o
#	
#router.o:
#	$(CC) router.c $(CFLAGS) $(CFLAGS_INDEX) -o router.o
#	
#controllers.o:
#	$(CC) controllers.c $(CFLAGS) -o controllers.o
#	
#views.o:
#	$(CC) views.c $(CFLAGS) -o views.o

samcms:
	$(CC) samcms.c $(CFLAGS) $(CFLAGS_SAMCMS) -o samcms



clean:
	$(RM) $(PROGRAMS)
# ${TARGET} $(OBJECTS)

.PHONY: all clean
