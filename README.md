# Sorted Bucket

A container written in C++ that keeps elements in sorted order and provides
sub-linear time complexity on functions (insert, erase, find, distance to index).

This is implemented three ways to investigate performance: 
1) RBT: a red-black tree (like std::set) but with weighted nodes to track the sorted idx in logarithmic time, theoretical ```O(log(n))``` all operations.
2) VV: a vector of vectors, theoretical ```O(sqrt(n))``` all operations.
3) LL: a doubly-linked list of doubly-linked lists, theoretical ```O(sqrt(n))``` all operations.

The templated container code is found inside the header files such as ```sortedBucketRBT.h``` 
inside the ```src``` folder.

## Demo

To run the demo, simply go into the ```src``` folder and compile and run
```demo.cpp```. This gives you a general idea of the container shape and how
the implementations differ.

## Benchmarking

### Getting started

The code comes supplied with a ```bench.cpp``` file where I made some tests using Google 
Benchmark as a framework. If you want to use this, create a folder ```benchmark```
in the root directory and clone the framework into there (found at
https://github.com/google/benchmark). If you don't clone to a subdirectory named ```bench```, 
then my CMake file rules will be invalid, and you will need to change the paths
manually.

Now from the root directory, run

```
$ cd benchmark
$ cmake -E make_directory "build"
$ cmake -E chdir "build" cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on -DCMAKE_BUILD_TYPE=Release ../
```

or alternately, if you want address sanitizer and thread sanitizer, you can build
it as I did (replace the second command above with):

```
$ cmake -E chdir "build" cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on -DCMAKE_BUILD_TYPE=Release \ -DCMAKE_C_FLAGS="-g -O2 -fno-omit-frame-pointer -fsanitize=address -fsanitize=thread \ -fno-sanitize-recover=all" -DCMAKE_CXX_FLAGS="-g -O2 -fno-omit-frame-pointer -fsanitize=address \ -fsanitize=thread -fno-sanitize-recover=all "  ../
```

and then actually build it:

```
$ cmake --build "build" --config Release
```

You can then view the build files (it is named ```bench```) inside the ```build```
folder. (If you're using Visual Studio like me, there will be the sortedBucket 
solution file there. Make sure you set "bench" as the startup project and build as
Release.) Make sure at least C++20 flag is set for your compiler or else you'll have
build issues.


## My results

The target was a laptop with:

CPU: AMD Ryzen 9 5900HS (x86-64), 8C/16T, 3300Mhz base clock, boost disabled for consistency

CPU Caches:
  L1 Data 32 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 512 KiB (x8)
  L3 Unified 16384 KiB (x1)

RAM: 16GB LPDDR4X at 3400mHz

OS: Windows 10 Pro 10.0.19044

My compiler was MSVC 19.36.32537.0.

I tested each function ```find```, ```insert```, ```erase```, and ```distance```
between 1,000 and 1,000,000 iterations and compared the results for each implementation.
The raw data for testing uint64_t is below. (You don't have to worry about
the "Iterations" column, it's just a Google Benchmark thing to get a consistent
runtime.)

```
----------------------------------------------------------------------------
Benchmark                                  Time             CPU   Iterations
----------------------------------------------------------------------------
BM_find_RBT<uint64_t>/1000             0.093 ms        0.086 ms        11200
BM_find_RBT<uint64_t>/10000             1.45 ms         1.38 ms          407
BM_find_RBT<uint64_t>/100000            27.8 ms         31.2 ms           22
BM_find_RBT<uint64_t>/1000000            943 ms          938 ms            1
BM_find_LL<uint64_t>/1000              0.964 ms        0.823 ms         1120
BM_find_LL<uint64_t>/10000              16.0 ms         15.6 ms           45
BM_find_LL<uint64_t>/100000              620 ms          625 ms            1
BM_find_LL<uint64_t>/1000000           30937 ms        30938 ms            1
BM_find_VV<uint64_t>/1000              0.056 ms        0.058 ms        13380
BM_find_VV<uint64_t>/10000             0.792 ms         1.02 ms          797
BM_find_VV<uint64_t>/100000             12.1 ms         12.5 ms           90
BM_find_VV<uint64_t>/1000000             193 ms          195 ms            4
BM_distance_RBT<uint64_t>/1000         0.093 ms        0.102 ms        10000
BM_distance_RBT<uint64_t>/10000         1.43 ms         1.36 ms          448
BM_distance_RBT<uint64_t>/100000        27.3 ms         26.8 ms           28
BM_distance_RBT<uint64_t>/1000000        934 ms          938 ms            1
BM_distance_LL<uint64_t>/1000          0.997 ms        0.977 ms          896
BM_distance_LL<uint64_t>/10000          16.2 ms         16.0 ms           37
BM_distance_LL<uint64_t>/100000          621 ms          609 ms            1
BM_distance_LL<uint64_t>/1000000       31021 ms        31016 ms            1
BM_distance_VV<uint64_t>/1000          0.056 ms        0.037 ms        13380
BM_distance_VV<uint64_t>/10000         0.676 ms        0.670 ms         1120
BM_distance_VV<uint64_t>/100000         12.6 ms         13.9 ms          100
BM_distance_VV<uint64_t>/1000000         729 ms          734 ms            1
BM_insert_RBT<uint64_t>/1000           0.138 ms        0.138 ms         4978
BM_insert_RBT<uint64_t>/10000           1.72 ms         1.69 ms          407
BM_insert_RBT<uint64_t>/100000          28.0 ms         27.5 ms           25
BM_insert_RBT<uint64_t>/1000000          787 ms          781 ms            1
BM_insert_LL<uint64_t>/1000            0.492 ms        0.488 ms         1120
BM_insert_LL<uint64_t>/10000            13.1 ms         13.1 ms           56
BM_insert_LL<uint64_t>/100000            434 ms          438 ms            2
BM_insert_LL<uint64_t>/1000000         14928 ms        14938 ms            1
BM_insert_VV<uint64_t>/1000            0.110 ms        0.110 ms         6400
BM_insert_VV<uint64_t>/10000            1.46 ms         1.47 ms          498
BM_insert_VV<uint64_t>/100000           18.5 ms         18.2 ms           37
BM_insert_VV<uint64_t>/1000000           265 ms          258 ms            2
BM_erase_RBT<uint64_t>/1000            0.089 ms        0.081 ms        11200
BM_erase_RBT<uint64_t>/10000            1.41 ms         1.45 ms          560
BM_erase_RBT<uint64_t>/100000           27.1 ms         26.8 ms           28
BM_erase_RBT<uint64_t>/1000000           947 ms          938 ms            1
BM_erase_LL<uint64_t>/1000             0.476 ms        0.531 ms         1000
BM_erase_LL<uint64_t>/10000             10.7 ms         10.0 ms           56
BM_erase_LL<uint64_t>/100000             372 ms          375 ms            2
BM_erase_LL<uint64_t>/1000000          16891 ms        16891 ms            1
BM_erase_VV<uint64_t>/1000             0.083 ms        0.088 ms         7467
BM_erase_VV<uint64_t>/10000             1.03 ms        0.977 ms          640
BM_erase_VV<uint64_t>/100000            14.2 ms         14.2 ms           45
BM_erase_VV<uint64_t>/1000000            210 ms          214 ms            3
```

And likewise, the results for randomly generated strings of length 128:

```
-------------------------------------------------------------------------------
Benchmark                                     Time             CPU   Iterations
-------------------------------------------------------------------------------
BM_find_RBT<std::string>/1000             0.792 ms        0.797 ms         1000
BM_find_RBT<std::string>/10000             9.20 ms         8.96 ms           75
BM_find_RBT<std::string>/100000             139 ms          138 ms            5
BM_find_RBT<std::string>/1000000           2595 ms         2594 ms            1
BM_find_LL<std::string>/1000               2.72 ms         2.78 ms          264
BM_find_LL<std::string>/10000              45.0 ms         45.9 ms           16
BM_find_LL<std::string>/100000             1037 ms         1047 ms            1
BM_find_LL<std::string>/1000000           59277 ms        59281 ms            1
BM_find_VV<std::string>/1000              0.690 ms        0.700 ms         1116
BM_find_VV<std::string>/10000              7.72 ms         7.92 ms           75
BM_find_VV<std::string>/100000             96.8 ms         98.2 ms            7
BM_find_VV<std::string>/1000000            1715 ms         1719 ms            1
BM_distance_RBT<std::string>/1000         0.767 ms        0.891 ms         1000
BM_distance_RBT<std::string>/10000         9.01 ms         8.54 ms           64
BM_distance_RBT<std::string>/100000         134 ms          141 ms            5
BM_distance_RBT<std::string>/1000000       2514 ms         2500 ms            1
BM_distance_LL<std::string>/1000           2.67 ms         2.66 ms          264
BM_distance_LL<std::string>/10000          42.7 ms         44.9 ms           16
BM_distance_LL<std::string>/100000         1124 ms         1125 ms            1
BM_distance_LL<std::string>/1000000       58762 ms        58766 ms            1
BM_distance_VV<std::string>/1000          0.690 ms        0.698 ms          896
BM_distance_VV<std::string>/10000          7.71 ms         7.50 ms           75
BM_distance_VV<std::string>/100000          123 ms          125 ms            6
BM_distance_VV<std::string>/1000000        6678 ms         6672 ms            1
BM_insert_RBT<std::string>/1000           0.848 ms        0.837 ms          747
BM_insert_RBT<std::string>/10000           9.84 ms         9.79 ms           75
BM_insert_RBT<std::string>/100000           155 ms          156 ms            4
BM_insert_RBT<std::string>/1000000         2709 ms         2719 ms            1
BM_insert_LL<std::string>/1000             1.70 ms         1.71 ms          448
BM_insert_LL<std::string>/10000            36.6 ms         36.2 ms           19
BM_insert_LL<std::string>/100000            882 ms          875 ms            1
BM_insert_LL<std::string>/1000000         42618 ms        42625 ms            1
BM_insert_VV<std::string>/1000             1.10 ms         1.10 ms          640
BM_insert_VV<std::string>/10000            15.0 ms         15.0 ms           50
BM_insert_VV<std::string>/100000            195 ms          198 ms            3
BM_insert_VV<std::string>/1000000          3208 ms         3203 ms            1
BM_erase_RBT<std::string>/1000            0.768 ms         1.11 ms         1000
BM_erase_RBT<std::string>/10000            9.53 ms         9.52 ms           64
BM_erase_RBT<std::string>/100000            181 ms          184 ms            4
BM_erase_RBT<std::string>/1000000          3043 ms         3047 ms            1
BM_erase_LL<std::string>/1000              1.70 ms         1.63 ms          498
BM_erase_LL<std::string>/10000             28.7 ms         28.8 ms           25
BM_erase_LL<std::string>/100000             662 ms          641 ms            1
BM_erase_LL<std::string>/1000000          34059 ms        34062 ms            1
BM_erase_VV<std::string>/1000              1.10 ms         1.15 ms          640
BM_erase_VV<std::string>/10000             11.6 ms         11.9 ms           50
BM_erase_VV<std::string>/100000             145 ms          141 ms            4
BM_erase_VV<std::string>/1000000           2361 ms         2359 ms            1
```

### Analysis

Remember that the RBT implementation has a better theoretical complexity ```O(log(n))``` than LL and 
VV ```O(sqrt(n))```. However, we see some very interesting things with large input
sizes. We would expect that the better ```O(log(n))``` complexity would scale better than
```O(sqrt(n))``` in a vacuum, but for instance, look at the insert runtime for large inputs 
of uint64_t:

```
-------------------------------------------------------------------------------
Benchmark                                     Time             CPU   Iterations
-------------------------------------------------------------------------------
BM_insert_RBT<uint64_t>/100000          28.0 ms         27.5 ms           25
BM_insert_RBT<uint64_t>/1000000          787 ms          781 ms            1
...
BM_insert_LL<uint64_t>/100000            434 ms          438 ms            2
BM_insert_LL<uint64_t>/1000000         14928 ms        14938 ms            1
...
BM_insert_VV<uint64_t>/100000           18.5 ms         18.2 ms           37
BM_insert_VV<uint64_t>/1000000           265 ms          258 ms            2
```

The VV beats the RBT by about 3 times, despite having a worse time complexity. The LL
version is not even close to either. Here we see the insane difference that frequent 
memory allocation/freeing and also bad cache performance leads to. 
(Memory alloc'ing performance of course depends on your OS and the cache 
depends on your hardware, but this is a pretty telling overview.)

But wait, that's not the whole story. The red-black tree also uses frequent memory 
allocations for individual insertions, so that can't explain the hundred-fold increase
between VV and LL. Well, firstly, the RBT does have a better theoretical time complexity. 
Secondly, the algorithm for finding something (like an insertion point) in an RBT 
is fundamentally different from the one used in the VV and LL versions. Because we 
start from the root of the tree and travel down at most ```log(n)``` nodes, our path will be a lot shorter
than that of travelling down ```sqrt(n)``` buckets to search the tails of the buckets. 
With a large ```n```, our cache gets trampled significantly more. (This brutish method
used in LL is thankfully replaced with a logarithmic search ```std::upper_bound()``` in the VV
implentation, due to vectors having random-access iterators instead of merely
bidirectional in doubly-linked lists, so VV has the shortest theoretical path.)

Maybe the dispersed memory also causes page faults and thrashing, but it's a similar effect 
to the cache misses and I can't really differentiate. They all fall under the umbrella
of memory hardware realities.

I mainly included the LL implementation out of curiosity to see how much of an
effect it would have on the runtime, but I wasn't expecting it to be this bad.
I'll take away from this that benchmarking is really important because it can
turn around my assumptions about things (such as big-O() being the single most 
important thing for large input sizes). Also, I now consider the placement of objects 
in memory a lot more for code performance.

In the future, I might add an interface for a real iterator class and various
tune-ups, but this project was primarily meant to be a learning experience, not 
something actually meant for production.
