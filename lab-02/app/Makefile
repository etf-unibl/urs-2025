CFLAGS  := -Wall -I./inc
STATIC  ?= 1
DESTDIR ?= .
LIBS    := -L./lib -ltest

APP_SRC := src/app.c
APP_OBJ := src/app.o


ifeq ($(STATIC),1) # Static by default
	LIB       := libtest.a
    LDFLAGS   := -static
else
	LIB       := libtest.so
    LDFLAGS   := 
endif

LIB_TARG := lib/$(LIB)

app: $(APP_OBJ) $(LIB_TARG)
	$(CROSS_COMPILE)gcc $(APP_OBJ) -o $@ $(LDFLAGS) $(LIBS)

$(APP_OBJ): $(APP_SRC)
	$(CROSS_COMPILE)gcc -c $< $(CFLAGS) -o $@

lib/$(LIB):
	$(MAKE) -C lib $(LIB) STATIC=$(STATIC)

install:
	mkdir -p $(DESTDIR)
	cp app $(DESTDIR)/
ifeq ($(STATIC),0)
	cp $(LIB_TARG) $(DESTDIR)/
endif

clean:
	rm -rf app src/*.o
	$(MAKE) -C lib clean


.PHONY: clean
