CFLAGS := -Wall -I../inc
SRCS_DIR := ../src
STATIC ?= 1

ifeq ($(STATIC), 1)
    TARGET_LIB := libtest.a
    LINK_CMD := $(CROSS_COMPILE)ar rcs $(TARGET_LIB) print.o sum.o
else
    CFLAGS += -fPIC
    TARGET_LIB := libtest.so
    LINK_CMD := $(CROSS_COMPILE)gcc -shared -o $(TARGET_LIB) print.o sum.o
endif

all: $(TARGET_LIB)

$(TARGET_LIB): print.o sum.o
	$(LINK_CMD)

%.o: $(SRCS_DIR)/%.c
	$(CROSS_COMPILE)gcc -c $< $(CFLAGS) -o $@

clean:
	rm -f *.o libtest.*

.PHONY: clean

