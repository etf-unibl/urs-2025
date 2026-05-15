SRC = blinky.c

TARGET = hps_button_control

CC = $(CROSS_COMPILE)gcc

CFLAGS = -Wall --sysroot=$(SYSROOT)

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)