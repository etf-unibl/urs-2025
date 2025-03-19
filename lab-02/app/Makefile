CFLAGS := -Wall -I./inc
LDFLAGS := -static -ltest -L./lib
LIB := libtest.a
TYPE ?= s
DESTDIR ?= bin

ifeq ($(TYPE), d)
    LIB := libtest.so
    LDFLAGS := -ltest -L./lib
endif

app: src/app.o lib/$(LIB)
	$(CROSS_COMPILE)gcc src/app.o -o $@ $(LDFLAGS)

src/app.o: src/app.c
	$(CROSS_COMPILE)gcc -c $< $(CFLAGS) -o $@

lib/$(LIB):
	$(MAKE) -C lib $(LIB) TYPE=$(TYPE)

install: app
	mkdir $(DESTDIR)
	mv $< $(DESTDIR)/
    
clean:
	rm -rf app src/*.o $(DESTDIR)
	$(MAKE) -C lib clean

.PHONY: clean

