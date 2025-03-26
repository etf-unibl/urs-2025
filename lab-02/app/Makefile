CFLAGS := -Wall -I./inc
LDFLAGS := -static -ltest -L./lib
STATIC ?= 1
LIB := lib/
DESTDIR ?= bin
ifeq ($(STATIC), 1)
	LIB_NAME += libtest.a
	LIB += /$(LIB_NAME)
else
	LIB_NAME += libtest.so
	LIB += $(LIB_NAME)
	LDFLAGS := -ltest -L./lib
endif
app: src/app.o  $(LIB)
	$(CROSS_COMPILE)gcc src/app.o -o $@ $(LDFLAGS)

src/app.o: src/app.c
	$(CROSS_COMPILE)gcc -c $< $(CFLAGS) -o $@

$(LIB):
	$(MAKE) -C lib $(LIB_NAME) STATIC=$(STATIC)

install: app
	mkdir $(DESTDIR)
	cp app $(DESTDIR) 
clean:
	rm -rf bin
	rm -f app src/*.o
	$(MAKE) -C lib clean

.PHONY: clean

