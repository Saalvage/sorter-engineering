## Data Structure
The segmented nature of the input data structure is not used, therefore it is simply abstracted away behind an index operator.

## Algorithms
The algorithm consists of three subroutines. If a radix sort bin is sufficiently small (see parameters below), the bin is sorted using Robin Hood sort.

### Radix Sort
Takes the position of a bit and an input range and sorts the range into two bins, one where the bit is unset and one where it is set. It does so by starting an iteration from both ends of the range and swapping elements whenever they are on the incorrect side, stopping when the iterators meet. It recurses over each bin with the next-less significant bit position.

### Robin Hood Sort
Scans the input range and linearly maps all elements into its auxiliary array according to their value. The auxiliary array has a capacity of >=2 times the input range's size. When a collision occurs it linearly probes into the appropriate direction (left if the element is larger, right if it is smaller). If the end is reached or the element must be inserted between two existing elements to preserve the order then either the left or right elements are shifted to make space. This always succeeds because of the at least 2x overallocation. The most significant bit is used to distinguish empty slots and the value 0, assuming at least one iteration of radix sort was executed this bit is the same across the entire range. At the end the auxiliary array is scanned linearly and all values written back into the original range in order.

### Parallelization
A simple job stack is used onto which radix sort iterations push their second bin (the first one they continue themselves to improve cache efficiency). If the job stack is empty and all threads are waiting for a job then the algorithm is done.

## Parameters
- `NUM_BLOCKS` in `container.hpp` controls the number of segments the input data is split into
- `ROBIN_HOOD_LIMIT_AUX_SPACE` in `sorter.hpp` controls whether Robin Hood sort invocations only utilize a multiple of their input range's size or the entire allocated buffer
- `ROBIN_HOOD_RANGE` in `sorter.hpp` controls the parameter `r`, the maximum size of a bin for which the radix sort recursion terminates in Robin Hood sort. Setting it to 0 disables Robin Hood sort
- `ROBIN_HOOD_SPACE_MULT` in `sorter.hpp` controls the parameter `m`. `m` times the size of the input range is the size of the Robin Hood auxiliary array
