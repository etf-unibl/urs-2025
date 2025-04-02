CFLAGS := -Wall -I./inc
LDFLAGS := -ltest -L./lib
STATIC ?= yes
DESTDIR := .

ifeq ($(STATIC), yes)
	LDFLAGS += -static
	LIBS := lib/libtest.a
else
	LIBS := lib/libtest.so
endif

app: src/app.o $(LIBS)
	mkdir $(DESTDIR)
	$(CROSS_COMPILE)gcc src/app.o -o $(DESTDIR)/$@ $(LDFLAGS) 

src/app.o: src/app.c
	$(CROSS_COMPILE)gcc -c $< $(CFLAGS) -o $@

lib/libtest.a:
	$(MAKE) -C lib libtest.a

lib/libtest.so:
	$(MAKE) -C lib libtest.so

clean:
	rm -f app src/*.o
	$(MAKE) -C lib clean

.PHONY: clean

