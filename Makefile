# Compilers
CC = nvcc
#CC = clang-11

# Flags
#CFLAGS = $(INC) -std=c11 -Wall -Wextra -O3 -march=native -mavx2 -Rpass=loop-vectorize -Rpass=missed=loopvectorize -Rpass-analysis=loop-vectorize
CUFLAGS = -O3 --use_fast_math -arch=sm_75 --compiler-options "$(CU_FLAGS_XCOMP)"
CFLAGS = $(INC) -std=c11 -Wall -Wextra -O3 -funroll-loops -ffast-math -flto
LDFLAGS = -lm
EXTRA = 

# Binary file
TARGET = tiny_cuda

# Files
C_SOURCES = tiny_cuda_pro.cu
C_OBJS = $(patsubst %.c, %.o, $(C_SOURCES))

# Rules
all: $(TARGET)

$(TARGET): $(C_OBJS)
	$(CC) $(CUFLAGS) $(EXTRA) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

cleanMain:
	rm -f $(TARGET)
