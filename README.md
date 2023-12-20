# Memory-Handling-Simulation
Part A is a worst-fit dynamic partition memory allocator, approximating some of the C library functions malloc() and free(). 
Given page size, allocation/deallocaion requestss, and starting memory size, simulation will show statisics, partition states, and addresses at each stage. Allocation follows worst-fit and deallocation will also merge any adjacent free partitions. 

Part B is a function that checks the contents of a file allocaiton table (FAT) that finds the longest possible chain of pointers for FAT tables within the range [1... 10 000 000], processing any valid input under 10s. 
