# Compilers
CC = clang

# Flags
CFLAGS = -std=c11 -Wall -Wextra -O3 $(EXTRA)
LDFLAGS = -lm
EXTRA =

# Binary file
TARGET = tiny_mc3

# Files
C_SOURCES = tiny_mc.c wtime.c tiny_twis.c
C_OBJS = $(patsubst %.c, %.o, $(C_SOURCES))

# Rules
all: $(TARGET)

$(TARGET): $(C_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

