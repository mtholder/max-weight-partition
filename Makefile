all: max-weight-partition

parse.o: max_weight_partition.h error.h parse.cpp
	g++ -g -std=c++20 -O3 -Wall -o parse.o -c parse.cpp

max-weight-partition: max_weight_partition.cpp error.h parse.o
	g++ -g -std=c++20 -O3 -Wall -o max-weight-partition max_weight_partition.cpp parse.o
	
clean:
	$(RM) max-weight-partition
	$(RM) parse.o
