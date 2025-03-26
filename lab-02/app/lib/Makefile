CFLAGS := -Wall -I../inc
SRCS_DIR := ../src
LINK?=static
LIB:=libtest.a

ifeq ($(LINK),dynamic)
	CFLAGS += -fPIC
	LIB := libtest.so
endif

$(LIB): print.o sum.o
ifeq ($(LINK),static)
	$(CROSS_COMPILE)ar rcs $@ $^
else
	$(CROSS_COMPILE)gcc -shared $^ -o $@
endif

%.o: $(SRCS_DIR)/%.c
	$(CROSS_COMPILE)gcc -c $< $(CFLAGS) -o $@

clean:
	rm -f *.o libtest*

.PHONY: clean

