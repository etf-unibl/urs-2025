CFLAGS := -Wall -I./inc
LDFLAGS := -ltest -L./lib
LINK ?= static
LIB := libtest.a
DESTDIR = bin

ifeq ($(LINK),dynamic)
	LINK := dynamic
	LIB := libtest.so
else
	LDFLAGS += -static
endif

app: src/app.o lib/$(LIB)
	$(CROSS_COMPILE)gcc src/app.o -o $@ $(LDFLAGS)

src/app.o: src/app.c
	$(CROSS_COMPILE)gcc -c $< $(CFLAGS) -o $@

lib/$(LIB):
	$(MAKE) -C lib $(LIB) LINK=$(LINK)

install : app
	mkdir $(DESTDIR)
	mv $< $(DESTDIR)

clean:
	rm -f app src/*.o $(DESTDIR)
	$(MAKE) -C lib clean

.PHONY: clean

