CFLAGS := -Wall -I../inc
SRCS_DIR := ../src

ifeq ($(TYPE), d)
    CFLAGS += -fPIC
endif

libtest.a: print.o sum.o
	$(CROSS_COMPILE)ar rcs $@ $^
	
libtest.so: print.o sum.o
	$(CROSS_COMPILE)gcc -shared $^ -o $@

print.o: $(SRCS_DIR)/print.c 
	$(CROSS_COMPILE)gcc -c $< $(CFLAGS) -o $@

sum.o: $(SRCS_DIR)/sum.c
	$(CROSS_COMPILE)gcc -c $< $(CFLAGS) -o $@

clean:
	rm -f *.o libtest.*

.PHONY: clean
