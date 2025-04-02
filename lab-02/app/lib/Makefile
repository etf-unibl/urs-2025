CFLAGS := -Wall -I../inc
SRCS_DIR := ../src

STATIC := true

ifeq ($(STATIC), true)
	FPIFLAG :=
else
	FPIFLAG := -fPIC
endif

libtest.a: print.o sum.o
	$(CROSS_COMPILE)ar rcs $@ $^

libtest.so: print.o sum.o
	$(CROSS_COMPILE)gcc -shared -o $@ $^

%.o: $(SRCS_DIR)/%.c
	$(CROSS_COMPILE)gcc $(FPIFLAG) -c $< $(CFLAGS) -o $@
	

clean:
	rm -f *.o libtest*

.PHONY: clean

