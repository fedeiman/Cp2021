# Compilers
# Compilers
CC = gcc

# Flags
CFLAGS = -std=c11 -Wall -Wextra $(EXTRA)
LDFLAGS = -lm
EXTRA =

FILE =

# Binary file
TARGET = tiny_mc

# Files
C_SOURCES = $(TARGET).c wtime.c mtwister.c
C_OBJS = $(patsubst %.c, %.o, $(C_SOURCES))

# Rules
all: $(TARGET)

$(TARGET): $(C_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

cleanMain: 
	rm -f $(TARGET) $(TARGET).o

