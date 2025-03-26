CFLAGS := -Wall -I../inc
SRCS_DIR := ../src
STATIC=no

ifeq($(STATIC), no)
	FLAG := -fPIC
endif

libtest.a: print.o sum.o
	$(CROSS_COMPILE)ar rcs $@ $^

libtest.so: print.o sum.o
	$(CROSS_COMPILE)gcc -shared print.o sum.o -o $@

%.o: $(SRCS_DIR)/%.c
	$(CROSS_COMPILE)gcc $(FLAG) -c $< $(CFLAGS) -o $@

clean:
	rm -f *.o libtest*

.PHONY: clean

