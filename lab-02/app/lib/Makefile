CFLAGS := -Wall -I../inc
SRCS_DIR := ../src
STATIC := 1
ifeq ($(STATIC), 0)
	CFLAGS += -fPIC
endif

libtest.so: print.o sum.o
	$(CROSS_COMPILE)gcc -shared $^ -o $^ 

libtest.a: print.o sum.o
	$(CROSS_COMPILE)ar rcs $@ $^

%.o: $(SRCS_DIR)/%.c
	$(CROSS_COMPILE)gcc -c $< $(CFLAGS) -o $@

clean:
	rm -f *.o libtest*

.PHONY: clean

