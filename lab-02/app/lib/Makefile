STATIC ?= 1

CFLAGS := -Wall -I../inc
SRCS_DIR := ../src

# Ako pravimo dinamičku biblioteku, dodajemo -fPIC fleg
ifeq ($(STATIC), 0)
    CFLAGS += -fPIC
endif

.PHONY: clean

# Pravilo za statičku arhivu
libtest.a: print.o sum.o
	$(CROSS_COMPILE)ar rcs $@ $^

# Pravilo za dinamičku biblioteku
libtest.so: print.o sum.o
	$(CROSS_COMPILE)gcc -shared -o $@ $^

%.o: $(SRCS_DIR)/%.c
	$(CROSS_COMPILE)gcc -c $< $(CFLAGS) -o $@

clean:
	rm -f *.o libtest*
