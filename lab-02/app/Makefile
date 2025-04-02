CFLAGS := -Wall -I./inc
LDFLAGS_STATIC := -static -ltest -L./lib
LDFLAGS_DYNAMIC := -ltest -L./lib
STATIC := true

ifeq ($(STATIC),true)
	LDFLAGS := $(LDFLAGS_STATIC)
	LIBS := lib/libtest.a
else
	LDFLAGS := $(LDFLAGS_DYNAMIC)
	LIBS := lib/libtest.so
endif

app: src/app.o $(LIBS)
	$(CROSS_COMPILE)gcc src/app.o -o $@ $(LDFLAGS)

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

