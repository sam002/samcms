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
	$(CC) index.c $(LDFLAGS) $(CFLAGS) $(CFLAGS_INDEX) -o samcms-index

samcms:
	$(CC) samcms.c $(CFLAGS) $(CFLAGS_SAMCMS) -o samcms

clean:
	$(RM) $(PROGRAMS)
# ${TARGET} $(OBJECTS)

.PHONY: all clean
