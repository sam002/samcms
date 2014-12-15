CC=gcc
TARGET=$(shell basename `pwd`)
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:%.c=%.o)
PREFIX=./develop

CFLAGS += -O3 -std=c99 -I/usr/include/postgresql -L/usr/lib/postgresql/9.[2,3,4]/lib -Wall
LDFLAGS += -lpq -lfcgi

install:
	cd ./bin ; make clean ; make all
	test -d $(PREFIX)/usr/bin || install -d $(PREFIX)/usr/bin
	install -m 0755 ./bin/samcms ./bin/samcms-index ./tools/samcms-create $(PREFIX)/usr/bin
	
	test -d $(PREFIX)/etc/default || install -d $(PREFIX)/etc/default
	install -m 0644 ./tools/samcms-default $(PREFIX)/etc/default/samcms
	
	test -d $(PREFIX)/etc/init.d || install -d $(PREFIX)/etc/init.d
	install -m 0755 ./tools/samcms-run $(PREFIX)/etc/init.d/samcms
	
	test -d $(PREFIX)/usr/share/samcms || install -d $(PREFIX)/usr/share/samcms
	cp -r ./modules/ $(PREFIX)/usr/share/samcms/
	chmod -R 755 $(PREFIX)/usr/share/samcms/
	
	$(PREFIX)/usr/bin/samcms-create $(PREFIX)/
	sudo $(PREFIX)/etc/init.d/samcms start

uninstall:
	$(PREFIX)/etc/init.d/samcms stop || killall samcms-index
	rm -rf $(PREFIX)/usr/bin/samcms $(PREFIX)/usr/bin/index $(PREFIX)/usr/bin/samcms-create $(PREFIX)/etc/default/samcms $(PREFIX)/etc/init.d/samcms $(PREFIX)/usr/share/samcms

#all: $(TARGET) index

$(OBJECTS): $(SOURCES)

$(TARGET): $(OBJECTS)

#index: index.o
#	$(CC) index.o $(LDFLAGS) $(CFLAGS) -o index

clean:
	$(RM) -r $(OBJECTS) $(TARGET)

.PHONY: all clean
