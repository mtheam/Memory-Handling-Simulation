// this is the file you need to edit
// -------------------------------------------------------------------------------------
// you don't have to use any of the code below, but you can
using namespace std; 

#include "memsim.h"
#include <iostream>
#include <cassert>
#include <list>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <numeric>
#include <boost/functional/hash.hpp>
#include <chrono>
#include <atomic>

// NOTE 64bit system ; pointers are always going to be faster to passed if less than 8 bytes 

using namespace std::chrono; 


struct Partition{  
  int64_t tag;
  int64_t size; //size in bytes, must be a multiple of pageSize
  int64_t addr; 
};

typedef list<Partition>::iterator PartitionIt; 

void partitionPrinter(Partition & p ){
  std::cout << "tag: " << p.tag  << std::endl;
  std::cout << " size: " << p.size  << std::endl;
  std::cout << " addr: " << p.addr << std::endl; 
  std::cout << "-----------" << std::endl;
}

struct scmp{ //custom comparison (note how the set is declared to keep the set sorted and note the type it contains)
  bool operator()(const PartitionIt & c1, const PartitionIt & c2) const{
    if (c1->size == c2->size){ 
      //std::cout << "trigger if" << std::endl;
      return c1->addr < c2->addr;
    }
     else{
      //std::cout << "trigger else" << std::endl;
      //std::cout << "is " << c1->size << ">" << c2->size << " ?" << std::endl;
      return c1->size > c2->size; 
     }
  }
 };
 
 


class Simulator {
  public:
  int64_t pageSize; //partitions are ALWAYS going to be a multiple of pageSize

  int64_t allPageRequests = 0; 

  //all partitions in a linked list
  list<Partition> allBlocks; 
 
  //has ALL blocks, not just the free ones.. //why am i making a new iterator every time? just change them all to references 
  set<PartitionIt, scmp> freeBlocks; //Partition, size (uses custom comparater on size) 

  //quick access tagged partitions; contains pointers to linked list nodes 
  unordered_map<int, vector<PartitionIt>> taggedBlocks; //tag, vector<partitions with that tag>

/**
 * @brief Allocates to partition, splits if needed. Updates allBlocks, freeBlocks
 * 
 * @param size 
 * @param tag 
 * @param bigP  
 */
void partitionSplit(int64_t size, int64_t tag, PartitionIt bigP){ //places the tag into the partition and then splits if neccessary 
  //auto start = high_resolution_clock::now(); 
//std::cout << "partitionsplit" << std::endl;
//  auto start = high_resolution_clock::now(); 
  PartitionIt splitPIt; //the new one 
  
/*   if ((*bigP).tag != -1){ 
    result = bigP; 
    return result; 
  }
 */
  auto & partition = *bigP; 
  auto b = partition.size; 

  freeBlocks.erase(bigP);

  int newSize = b-size; 

  if (newSize > 0){ //partition is big enough to split, split and allocate 
    Partition splitP = {-1, (newSize), (partition.addr + size)}; //new free partition "after" the filled one  

    //fill partition 
    partition.size = size; 
    partition.tag = tag; 
    
    //insert new free split partition beside it 
    splitPIt = allBlocks.insert(next(bigP), splitP); 
    freeBlocks.insert(splitPIt); //updateFreeBlocks

    //checks if new free partition needs to be merged 
    PartitionIt adj = next(splitPIt); 
    if ((*adj).tag == -1){ //merge free partitions
      partitionMerge (& adj, & splitPIt); 
    }
    //freeBlocks.insert(result); //update Freeblocks
  }
  else { //fits exactly, allocate and return 
    (*bigP).tag = tag; 
    assert((*bigP).size == size);
  }
}

/**
 * @brief Adds (newly allocated parttiion) to tagged partitions. 
 * 
 * @param tag 
 * @param p 
 */
void addTagged(int64_t tag, PartitionIt p){
  //std::cout << "add tagged" << std::endl;
    vector<PartitionIt> newTaggedVec {p}; //tagged vector 
    auto newTaggedBlock = taggedBlocks.insert({tag, newTaggedVec}); //inserts new only if doesn't exist 

    if (newTaggedBlock.second == false){ //tag already exists, update vector 
      vector<PartitionIt> & taggedVecRef = (*(newTaggedBlock.first)).second; //reference to the vector being updated in taggedBlocks 
      taggedVecRef.push_back(p);
    } 

}


/**
 * @brief Merge partition p2 into p1. Updates linked list allBlocks and set freeBlocks.  
 * 
 * @param p1 
 * @param p2 
 */
void partitionMerge(PartitionIt * p1, PartitionIt * p2){
  //std::cout << "partitionmerge" << std::endl;
 // auto start = high_resolution_clock::now(); 
 
  Partition & firstP = *(*p1); 
  Partition & secondP = *(*p2);  

  if ((firstP.tag != -1) || (secondP.tag != -1)) std::cout << "error, trying to merge non-free partitions" << std::endl;

  freeBlocks.erase(*p1);
  freeBlocks.erase(*p2);

  //std::cout << "merge function" << std::endl;
  secondP.size += firstP.size; //add p1 size to p2
  secondP.addr = firstP.addr;

  //delete p1 from allBlocks
  allBlocks.erase(*p1);
  //update freeBlocks
  freeBlocks.insert(*p2);


  //auto stop = high_resolution_clock::now();
 // partitionMergeTime += (duration_cast<microseconds>(stop-start));

}


/**
 * Makes a new partition and updates allBlocks and FreeBlogs. Merges partitions (multiple times) if needed
 * Only called when new memory is requested, and so will only ever need to merge with free partitions on the left. 
 * @param pages 
 * @return PartitionIt* from allBlocks
 */
PartitionIt  newPartition(int64_t pages){
 // std::cout << "newpartition" << std::endl;
 // auto start = high_resolution_clock::now();

  auto prevIt = --allBlocks.end(); //previous last partition 
  auto prevP = * prevIt; 
  Partition newP = {-1, pages*pageSize, (prevP.addr + prevP.size)};

  //add to allBlocks
  allBlocks.push_back(newP); 
  PartitionIt newPIt = --allBlocks.end(); 
  
  freeBlocks.insert(newPIt);

  //update
  if (prevP.tag == -1){ //last partition was also free, must merge
    partitionMerge(&prevIt, &newPIt);
  }

/*   auto stop = high_resolution_clock::now();
  newPartT += (duration_cast<microseconds>(stop-start)); */

  return newPIt; 
}



  Simulator(int64_t s){ //constructor
    pageSize = s; 
    
    Partition p = {-10, 0, 0};

    allBlocks.push_front(p); 
    PartitionIt pIt = allBlocks.begin(); 
    freeBlocks.insert(pIt);
    
  }

 void allocate(int64_t tag, int64_t size) { 
  //std::cout << "allocate" << std::endl;
    PartitionIt newAllocatedP; //iterator to the allocated partition
    PartitionIt biggest; 

    //how much memory (a multiple of pagesize) that would needed for an allocation
/*     if ((size % pageSize) == 0) memoryNeeded = size/pageSize; 
    else memoryNeeded = size/pageSize + 1; 
 */

    if (allBlocks.size() == 0){ //populate empty memory with a dummy partition 
        Partition dummy = {-1, 0, 0}; 
        allBlocks.push_back(dummy); 
        freeBlocks.insert(allBlocks.begin()); 
      }

    //there are free partitions; test if they can accomodate 
    if (! freeBlocks.empty()){
      biggest = *(freeBlocks.begin());
      Partition & biggestP = *biggest; 
      int64_t biggestSize = biggestP.size; 

        if (biggestSize >= size){ //fits, allocate and split if needed, add to tagged and finish allocation 
          newAllocatedP = biggest; 
          partitionSplit(size, tag, biggest);
          addTagged(tag, newAllocatedP);
          return;  
        }
    } 

    //otherwise, must make a new partition
    PartitionIt lastAllPIt = (--allBlocks.end()); 
    Partition  lastAllP = *lastAllPIt; 

    int64_t reqPages = 0; //number of required pages that will be requested
    int64_t needMem = 0; 

  //calculating needed memory 
    if (lastAllP.tag == -1){ //if free, must take into account when calculating page request  ;
      needMem = size - lastAllP.size;
      reqPages = needMem/pageSize; 
      if ((needMem%pageSize) != 0){ //round up page request 
        reqPages = needMem/pageSize + 1; 
      }
    }
    else{ //otherwise, calculate memory with size param 
      reqPages = size/pageSize;
      if ((size%pageSize) != 0){ //round up page request  
          reqPages = size/pageSize + 1; 
      }
    }

    allPageRequests += reqPages; //add to all page requests 

    newPartition(reqPages); //make the new partition 
    newAllocatedP =  --allBlocks.end();  

    partitionSplit(size, tag, newAllocatedP); //allocate and split if needed 
    addTagged(tag, newAllocatedP); 
    //finish memory request 
  
    //partitionPrinter(*newAllocatedP); 
  } //end allocate 




  void deallocate(int64_t tag) { //tags are positive bc absolute value, so free = negative tag 
  //std::cout << "deallocate" << std::endl;
  //auto start = high_resolution_clock::now(); 

    int64_t end = allBlocks.back().addr;
    vector<PartitionIt> allTagged = taggedBlocks[tag];
    for (auto & t : allTagged){  //merge right
    // partitionPrinter(*t);
      (*t).tag = -1; 
      freeBlocks.insert(t); 
      
      //must check both left and right side for free partitions to merge with 
      PartitionIt nextPIt = next(t);
      PartitionIt prevPIt = prev(t);  
      
      //left side
      if (((*t).addr != 0) && ((*prevPIt).tag == -1)){ 
        //partitionPrinter(*prevPIt); 
       // std::cout << "==========" << std::endl; 
        partitionMerge(&prevPIt, &t); 
      }  

      //right side 
      if (((*t).addr != end) && ((*nextPIt).tag == -1)){ 
       // partitionPrinter(*nextPIt); 
       // std::cout << "==========" << std::endl; 
        partitionMerge(&t, &nextPIt); 
      }  
    }


    //EVERYTHING with that tag in allblocks is gone,so replace with empty vector or just erase it (check for speed)
    (taggedBlocks[tag]).clear(); //delete whole vector from taggedBlocks

/*    auto stop = high_resolution_clock::now();
    deallocateT += duration_cast<microseconds>(stop-start); */
  }


  MemSimResult getStats() {

    // let's guess the result... :)
    MemSimResult result;
    result.max_free_partition_size = 0;
    result.max_free_partition_address = 0;
    
    if (!freeBlocks.empty()){
      auto & big = *(*(freeBlocks.begin()));
      result.max_free_partition_size = big.size;
      result.max_free_partition_address = big.addr;

      for (auto f : freeBlocks){ //catch ties for largest free partitions; if same size, report the smaller addr
        if (((*f).size == big.size) && ((*f).addr < result.max_free_partition_address)){ 
            result.max_free_partition_address = (*f).addr; 
        }
        else break; 
      }
    }

    result.n_pages_requested = allPageRequests;
    return result;
  }

}; //end Simulator class

// ===================================
// parameters:
//    page_size: integer in range [1..1,000,000]
//    requests: array of requests
// return:
//    some statistics at the end of simulation
MemSimResult mem_sim(int64_t page_size, const std::vector<Request> & requests){

  Simulator sim(page_size);

  for (const auto & req : requests) {
    if (req.tag < 0) {
      sim.deallocate(-req.tag);
    } else {
      sim.allocate(req.tag, req.size);
    }

  }

  return sim.getStats();
}
 