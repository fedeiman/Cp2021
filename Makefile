# Compilers
CC = gcc
#CC = clang-11

# Flags
#CFLAGS = $(INC) -std=c11 -Wall -Wextra -O3 -march=native -mavx2 -Rpass=loop-vectorize -Rpass=missed=loopvectorize -Rpass-analysis=loop-vectorize 
CFLAGS = $(INC) -std=c11 -Wall -Wextra -O3 -funroll-loops -ffast-math -flto -fopenmp
LDFLAGS = -lm
EXTRA = 

# Binary file
TARGET = tiny_pcg

# Files
C_SOURCES = tiny_pcg.c wtime.c pcg_basic.c
C_OBJS = $(patsubst %.c, %.o, $(C_SOURCES))

# Rules
all: $(TARGET)

$(TARGET): $(C_OBJS)
	$(CC) $(CFLAGS) $(EXTRA) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

cleanMain:
	rm -f $(TARGET)
