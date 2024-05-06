# max-weight-partition

## input csv format with no escaping of internal commas supported.

The first line should be an integer indicating the number of subsets. 

That should be followed by 1 line per subset encoded as:

`subset-weight,label-for-element-1,label-for-element-2,...,label-for-last-element,`

The sum of weights over all lines should be the same for each element.

The program will try to find the partitions that maximum score for each size of
subset. 

Solutions will be emitted as JSON

## Configuration Checks the working directory for a configuration file:
   `.max_weight_partition.cfg` If found it should contain only comment lines
   (those starting with `#`) or lines of the form: `key = value` where the keys
   can be one of the following:

  * `cache_size` - size of cache of solutions for connected components. Default
    2 million. 
  * `output_exact_level` - negative number to silence, positive number to enable
    status outputs only at a specific level of recursion.
  * `output_less_than_level` - omit to disable this behavior. If included, it
    should be a positive number and status output for all levels of recursion
    less than this threshold will emit status updates.


# Acknowledgements The LRUCache implementation is a lightly modified version of
  cachelib from https://github.com/ksholla20/cachelib by GitHub user ksholla20

Those modified files are: 
  * Cache.cpp
  * Cache.h
  * DoublyLinkedList.h
  * LRUCache.cpp
  * LRUCache.h
