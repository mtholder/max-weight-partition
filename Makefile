all: max-weight-partition

parse.o: max_weight_partition.h error.h parse.cpp
	g++ -g -std=c++20 -O3 -Wall -o parse.o -c parse.cpp

LRUCache.o: LRUCache.h LRUCache.cpp Cache.h DoublyLinkedList.h custom_priority_queue.h
	g++ -g -std=c++20 -O3 -Wall -o LRUCache.o -c LRUCache.cpp

Cache.o: Cache.h Cache.cpp DoublyLinkedList.h custom_priority_queue.h
	g++ -g -std=c++20 -O3 -Wall -o Cache.o -c Cache.cpp

max-weight-partition: max_weight_partition.cpp error.h parse.o LRUCache.h LRUCache.o Cache.h Cache.o DoublyLinkedList.h custom_priority_queue.h
	g++ -g -std=c++20 -O3 -Wall -o max-weight-partition max_weight_partition.cpp parse.o LRUCache.o Cache.o
	
clean:
	$(RM) max-weight-partition
	$(RM) parse.o
