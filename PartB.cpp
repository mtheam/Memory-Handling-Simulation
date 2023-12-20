// -------------------------------------------------------------------------------------
// this is the only file you need to edit
// -------------------------------------------------------------------------------------
//
// (c) 2021, Pavol Federl, pfederl@ucalgary.ca
// Do not distribute this file.

#include "fatsim.h"
#include <cstdio>
#include <numeric>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <stack>
#include <queue>

typedef std::vector<long> adjList; 

int64_t fatSize; //foward declare size of fat 

/* so the index of each block in fatsim is gong to contain a number, which is just the index of a different block  
starting with any blocks labelled -1, or the ending chain, trace backwards using DFS to find the longest chains 
you ONLY have to figure out the chains that end in the NULL/-1 blocks, which means the others are useless 
then, print them out in order of ascendency; 

you dont even need their nodes or the blocks they point to, just the length of the chain they connect with 



bc must be in ascending order, dont even need to iterate through the whole thing; just find the first -1, store the index, and go from there  */


//make graph...and then also an adjacency list? or a vector of pointers to nodes? 
/* struct node{
  long id; 
  node * edge; //thing it points to 
  bool visited; 
}
 */


void graphPrinter(std::vector<adjList> * graph, std::vector<long> * terminate){
  //std::cout << "Adjacency list: " << std::endl;
  int i = 0; 
  for (auto node : *graph){
    std::cout <<   i << " : "; 
    for (auto edge : node) { 
      std::cout << edge << " ";
    }
    std::cout << std::endl;
    i++;
  }

  //std::cout << "And the terminate nodes are: ";
  for (auto t : *terminate){
    std::cout << t << " "; 
  }
  std::cout << std::endl;
}


/**
 * @brief Called on all terminal nodes, and searches through the graph adjacency list using depth first search, keeping track of how long each branch is as it branches off. 
 * Since each terminating node will only have a single longest chain, we can keep searching until all of the branches are visited.
 * This is ITERATIVE, trying to avoid a stack overflow for large files. Approach: push all into stack in reverse order, FIRST NODE HANDLED IS FIRST CHILD 
 * 
 * @param graph  Populated adjacency list 
 * @param end Ending node to start from 
 * @return long
 * Length of longest chain from terminating node 
 */
long dfs(std::vector<adjList> * graph, long end){ 
  struct nodeChain{ //struct to store the chain at a certain node when a split is made 
    long chain; //chain length  
    long n; //node id
  };

  long original = end; 
  long result = 1; // longest chain from the terminate node
  //adjList * edges = & (*graph)[end]; //get adjacency list for first degree connections 

  int64_t fSize = fatSize; //internal version 
  
  std::vector<bool> visited (fatSize, false); //at the node index, true if visited
  std::stack<nodeChain> toVisit; //stack of nodes to visit 

  nodeChain e; 
  e.n = end; 
  e.chain = 1; 

  toVisit.push(e);

  long chain = 1; //current longest chain 
  long cachedSplit; //cache of the last edge for processing on a node where the path is being split 

  while(!toVisit.empty()){
   nodeChain curr = toVisit.top();

    end = curr.n; //rmb that the source node is already in the stack 

    //assert (curr == end);
    toVisit.pop();
    //if you've found a node visited, then it doesn't get added to the stack 
    if (visited[end] == true){
     // std::cout << "already visited" << std::endl;
      continue;
    }  
    else{ //process undiscovered adjacent nodes into stack 
      //std::cout << "processing: " << end << std::endl;
      visited[end] = true; 
      chain = curr.chain; 
    }
   // std::cout << "Processing: " << end << " chain : " << chain;

    adjList * edges = & (*graph)[end]; //adjacencies to current node
    auto nEdges = (*edges).size();  

/*       //if we've split down a path before this, decrement the edges processed for that node 
      if (splits.empty() == false){
        auto & lastFork = splits.back();
        lastFork.numEdges--; 
        //std::cout << " --- Last fork was: " << lastFork.n << " and there are " << lastFork.numEdges << " edges left " << std::endl;
        if (lastFork.numEdges <= 0){ //backtrack to last node 
          chain = lastFork.chain; 
          std::cout << "Backtracking... Setting chain back to: " << chain << std::endl;
          splits.pop(); 
        }
      } */


    if (nEdges == 0){//reached the deepest point of this branch 
      //std::cout << "end of branch? chain is " << chain << std::endl;
      if (end != original){ 
        if (chain > result) result = chain;
      }
      else{ //end == original, so chain length of 1
      chain = 1; 
      } 

    } 
    //else chain ++; //TODO put it here or in fork? 
/*     else if(nEdges == 1){ // single edge 
      chain++; 
    }
    else { // multiple edges leading to node 
      same = true; 
      //std::cout << "fork chain : "  << chain << std::endl;

      nodeChain nChain; //save this splitting path in the queue, and the chain length of the node 
      nChain.chain = chain; 
      nChain.n = end;
      nChain.numEdges = nEdges; //as nodes are processed after a fork, the most recent forked node will have processed nodes numEdges--;

      splits.push(nChain); //save split to queue 

      chain++;
    } */

 //std::cout << std::endl;
    //std::cout << "SEGFAULT?" << std::endl;
      //REVERSE ORDER ITERATORS bc iterative means we put them all in the stack backwards! 
      //the real chain processing is done when we push to stack?
      long forking; 
      for (auto next = (*edges).rbegin(); next != (*edges).rend(); next++){
        if (visited[*next] == false){
          nodeChain toStack; 
          toStack.chain = (chain + 1) ;  
          toStack.n = *next; 

          toVisit.push(toStack); 
         // std::cout << "pushing " << *next << std::endl;
        } 
      } 

      

  } 

   
 // std::cout << "longest chain is: " << result << std::endl;
  return result; 
}





// reimplement this function   
std::vector<long> fat_check(const std::vector<long> & fat) // 0 to 10 000 000 
{
  fatSize = fat.size();  
  int64_t fSize = fatSize; //internal version 
  
  
  adjList edges;

  //adjacency list where each index n to the node contains a list of all edges pointing immediately towards it 
  //termination nodes will be empty, and will instead be kept in a separate vector 
  std::vector<adjList> graph (fSize, edges); //of size fatSize, filled with empty edge vectors 
  //std::vector<long> empty = {}; 

  //seperate adjacency list that keeps track of all termination nodes, avoids a segfault from accessing index -1, and avoids having to keep track of it as a substitute index in graph 
  std::vector<long> terminate; 

  for (long i = 0; i < fSize ; i++){
    long n = fat[i];
    if (n != -1){
      graph[n].push_back(i);
    }
    else{ //is a terminate node, -1 
      terminate.push_back(i); 
    }
  }

  //graphPrinter(& graph,  & terminate); 

  std::vector<long> result;  
  for (auto t : terminate){
    long clength = dfs(&graph, t);
    result.push_back(clength);
  }
  

  sort(result.begin(), result.end()); 
  return result;
}


//fat[2] = 6 means that the node 2 points towards node 6 
//and fat[6] = -1 means that the node 6 points towards nothing, so the chain ends at 6 
//we are filling the list at node 6 
//so trying to populate an adj list, for each list adjist[i] , the node [i] will ahve stuff poitoing to it 
//and so everything that is fat[x] = i, means that the adjlistd[i].push_bac\