# max-weight-partition

## input
csv format with no escaping of internal commas supported.

The first line should be an integer indicating the number of subsets. 

That should be followed by 1 line per subset encoded as:

`subset-weight,label-for-element-1,label-for-element-2,...,label-for-last-element,`

The sum of weights over all lines should be the save for each element.

The program will try to find the partitions that maximum score for each size of subset.
