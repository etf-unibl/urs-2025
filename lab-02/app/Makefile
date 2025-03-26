CFLAGS := -Wall -I./inc
STATIC ?= 1
LDFLAGS_STATIC := -static -ltest -L./lib
LDFLAGS_DYNAMIC := -ltest -L./lib
LDFLAGS :=  $(LDFLAGS_STATIC)

ifeq ($(STATIC),0)
	LDFLAGS := $(LDFLAGS_DYNAMIC)
endif

app: src/app.o lib/libtest.a
	$(CROSS_COMPILE)gcc src/app.o -o $@ $(LDFLAGS)

src/app.o: src/app.c
	$(CROSS_COMPILE)gcc -c $< $(CFLAGS) -o $@

lib/libtest.a:
	$(MAKE) -C lib libtest.a

intall: app
	install -d $(DESTDIR)/usr/bin
	install -m 755 app $(DESTDIR)/usr/bin
clean:
	rm -f app src/*.o
	$(MAKE) -C lib clean

.PHONY: clean

