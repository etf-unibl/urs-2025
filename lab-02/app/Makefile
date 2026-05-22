# Podrazumijevano je statičko linkovanje (STATIC = 1)
STATIC ?= 1

CFLAGS := -Wall -I./inc
LDFLAGS := -L./lib -ltest

# Ako je STATIC 1, radimo statičko linkovanje
ifeq ($(STATIC), 1)
    LDFLAGS += -static
    LIB_TARGET := lib/libtest.a
# Ako je STATIC 0, radimo dinamičko linkovanje
else
    LIB_TARGET := lib/libtest.so
endif

.PHONY: all clean install

all: app

app: src/app.o $(LIB_TARGET)
	$(CROSS_COMPILE)gcc src/app.o -o $@ $(LDFLAGS)

src/app.o: src/app.c
	$(CROSS_COMPILE)gcc -c $< $(CFLAGS) -o $@

lib/libtest.a:
	$(MAKE) -C lib libtest.a

lib/libtest.so:
	$(MAKE) -C lib libtest.so

# Pravilo za instalaciju (Zadatak sa vježbe)
install: app
	mkdir -p $(DESTDIR)
	cp app $(DESTDIR)/
	@if [ "$(STATIC)" = "0" ]; then \
		echo "Kopiram i dinamičku biblioteku u DESTDIR..."; \
		cp lib/libtest.so $(DESTDIR)/; \
	fi

clean:
	rm -f app src/*.o
	$(MAKE) -C lib clean
