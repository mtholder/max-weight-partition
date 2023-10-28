all: max-weight-partition


max-weight-partition: max_weight_partition.cpp error.h
	g++ -g -std=c++20 -O3 -Wall -o max-weight-partition max_weight_partition.cpp
	
clean:
	$(RM) max-weight-partition