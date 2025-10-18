CXX = g++
CXXFLAGS = -std=c++20 -g -O3 -ffast-math -march=native -mfpmath=sse -I. \
           -I../../../sim/code/include \
					 ../../../sim/code/src/ds/storage/particleBlock.cc \
           -Wall -Wextra -Wno-unused-parameter

LDFLAGS = -lpthread

BENCH_LIB = /usr/lib/libbenchmark.so
all: simple_bench

OUTPUT_NAME = simple.bench

simple_bench: simple.cc 
	$(CXX) $(CXXFLAGS) simple.cc $(BENCH_LIB) $(LDFLAGS) -o $(OUTPUT_NAME)
	@echo "✅ Built: $(OUTPUT_NAME)"

clean:
	rm -f $(OUTPUT_NAME)

