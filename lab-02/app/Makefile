CFLAGS := -Wall -I./inc
LDFLAGS_STATIC := -static -ltest -L./lib
LDFLAGS_DYNAMIC := -ltest -L./lib
STATIC ?= 1
DESTDIR ?= bin

ifeq ($(STATIC), 1)
    LDFLAGS := $(LDFLAGS_STATIC)
    LIB_SUFFIX := a
else
    LDFLAGS := $(LDFLAGS_DYNAMIC)
    LIB_SUFFIX := so
endif

app: src/app.o lib/libtest.$(LIB_SUFFIX)
	$(CROSS_COMPILE)gcc src/app.o -o $@ $(LDFLAGS)

src/app.o: src/app.c
	$(CROSS_COMPILE)gcc -c $< $(CFLAGS) -o $@

lib/libtest.$(LIB_SUFFIX):
	$(MAKE) -C lib libtest.$(LIB_SUFFIX) STATIC=$(STATIC)

install: app
	mkdir $(DESTDIR)
	mv $< $(DESTDIR)/	
clean:
	rm -rf app src/*.o $(DESTDIR)
	$(MAKE) -C lib clean

.PHONY: clean 

