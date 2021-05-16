# Compilers
CC = gcc
#CC = clang-11

INC = -I../ -I../../
# Flags
#CFLAGS = $(INC) -std=c11 -Wall -Wextra -O3 -march=native -mavx2 -Rpass=loop-vectorize -Rpass=missed=loopvectorize -Rpass-analysis=loop-vectorize 
CFLAGS = $(INC) -std=c11 -Wall -Wextra -O3 -march=native -fopt-info-vec-note -mavx2 -ftree-vectorize -fopt-info-vec 
LDFLAGS = -lm
EXTRA = 

# Binary file
TARGET = tiny_mc

# Files
C_SOURCES = tiny_mc.c wtime.c
C_OBJS = $(patsubst %.c, %.o, $(C_SOURCES))

# Rules
all: $(TARGET)

$(TARGET): $(C_OBJS)
	$(CC) $(CFLAGS) $(EXTRA) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

cleanMain:
	rm -f $(TARGET)
