/* 
    File: my_allocator.c

    Author: <your name>
            Department of Computer Science
            Texas A&M University
    Date  : <date>

    Modified: 

    This file contains the implementation of the module "MY_ALLOCATOR".

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <stdexcept>
#include <stdlib.h>
#include <iostream>
#include "my_allocator.h"
#include <cmath>
#include "math.h"

#define DEBUG 0
#define USE_MY_MALLOC 0


using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

struct Header{
  Header* next;
  int size;
  bool filled;

  Header(int s) : size(s){}

  bool operator ==(Header b){
    return (next == b.next) && (size == b.size) && (filled == b.filled);
  }
};

Header** freeList;

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FUNCTIONS FOR MODULE MY_ALLOCATOR */
/*--------------------------------------------------------------------------*/

/* Don't forget to implement "init_allocator" and "release_allocator"! */

unsigned int _bbs;
unsigned int _l;
int freeListSize;
Addr startOfMemoryBlock;

extern unsigned int init_allocator(unsigned int _basic_block_size, 
			    unsigned int _length) {
  //Needs to:
  //Initialize free list array
  //Init local vars of length and basic block size
  //Init headers to fill given length

  if(_basic_block_size <= sizeof(Header)){
    throw runtime_error("Basic block size cannot be less than or equal to the size of the Header.");
  }

  if(modf(log2(_basic_block_size), new double ) >= .001){
    throw runtime_error("Unable to use a block size that is not a power of 2.");
  }
  if(modf((double)_length/(double)_basic_block_size, new double ) >= .001){
    throw runtime_error("The full length of memory is not perfectly divisible by the block size.");
  }

  _bbs = _basic_block_size;
  _l = _length;

  Addr startAddress = malloc(_length);
  startOfMemoryBlock = startAddress;


  freeListSize = (int)(log2(_length / _basic_block_size)) + 1;

  if(DEBUG){
    cout << "The free list has " << freeListSize << " different size slots.\n";
    cout << "Program has memory starting at addr: " << startAddress;
  }
  
  freeList = new Header*[freeListSize];

  int lengthUnallocated = _l;
  int currentSectionAllocatingIn = freeListSize - 1;
  while(lengthUnallocated >= _bbs){
    if(DEBUG){
      cout << "Unallocated length: " << lengthUnallocated << "\n";
    }
    int currentSectionBlockSize = pow(2, currentSectionAllocatingIn) *_bbs;
    if(currentSectionBlockSize <= lengthUnallocated){
      lengthUnallocated -= currentSectionBlockSize;
      Header* startOfHeader = (Header*) startAddress;
      startOfHeader->size = currentSectionBlockSize;
      startAddress = (Addr) startOfHeader + currentSectionBlockSize;
      freeList[currentSectionAllocatingIn] = startOfHeader;
    }
    currentSectionAllocatingIn--;
  }

  if(DEBUG){
    printFreeList();
  }

}

extern int release_allocator(){
  if(DEBUG){
    cout << "Releasing allocator.\n";
  }
  if(USE_MY_MALLOC){
    free(startOfMemoryBlock);
  }
  return 0;
}

void printFreeList(){
  cout << "Current Free List: \n";
  for(int i = 0; i < freeListSize; i++){
    cout << i << " | ";
    int numHeaders = 0;
    Header* firstHeader = freeList[i];
    while(firstHeader != NULL){
      numHeaders++;
      cout << "[" << (Addr) firstHeader << "]";
      cout << "size:" << firstHeader->size;
      firstHeader = firstHeader->next;
    }
    cout << " " << numHeaders << "\n";
  }
}



void splitToGetSomeInSection(int sectionNeeded){
  int sectionToStartSplittingFrom = -1;
  for(int i = sectionNeeded+1; i < freeListSize; i++){
    if(freeList[i] != NULL){
      sectionToStartSplittingFrom = i;
      break;
    }
  }
  if(sectionToStartSplittingFrom == -1){
    throw runtime_error("Out of memory.");
  }
  for(int i = sectionToStartSplittingFrom; i > sectionNeeded; i--){
    splitSection(i);
  }
}

void splitSection(int s){
  if(freeList[s] == NULL){
    throw runtime_error("Tried to split from empty section.");
  }
  if(s == 0){
    throw runtime_error("Tried to split from smallest section.");
  }
  Header* curSectionHeader = freeList[s];
  freeList[s] = curSectionHeader->next;

  if(DEBUG){
    cout << "splitting section of size: " << curSectionHeader->size << "\n";
  }

  curSectionHeader->size = curSectionHeader->size/2;


  Addr putNextHeaderHere = getBuddyAddress((Addr) curSectionHeader, curSectionHeader->size);
  if(DEBUG){
    cout << "Splitting section: " << s << "\n";
    printFreeList();
    cout << "Buddy address of: " << (Addr) curSectionHeader << ", " << putNextHeaderHere << "\n";
  }
  curSectionHeader->next = (Header*) putNextHeaderHere; 

  curSectionHeader->next->size = curSectionHeader->size;

  freeList[s-1] = curSectionHeader;
}


Addr getBuddyAddress(Addr start, int size){
  if(DEBUG){
    cout << "Start of memory block: " << (Addr) startOfMemoryBlock << "\n";
    cout << "Calculating buddy of " << start << ", size " << size << "\n";
  }
  Addr offsetStart = (Addr) (start - (int)startOfMemoryBlock);
  Addr offsetBuddy = (Addr) ( (int)offsetStart ^ size );
  if(DEBUG){
    cout << "The buddy is: " << (Addr)((int)offsetBuddy + (int)startOfMemoryBlock) << "\n";
  }
  return (Addr) ((int)offsetBuddy + (int)startOfMemoryBlock);
}



extern Addr my_malloc(unsigned int _length) {
  if(!USE_MY_MALLOC){
    return malloc((size_t)_length);
  }

  if(DEBUG){
    cout << "Malloc request for: " << _length << "(" << (_length + sizeof(Header)) << ").\n";
    printFreeList();
  }

  int blocksizeNeeded = _length + sizeof(Header);
  int sectionToTakeFrom = -1;
  for(int i = 0; i < freeListSize; i++){
    if(pow(2,i)*_bbs >= blocksizeNeeded){
      sectionToTakeFrom = i;
      break;
    }
  }

  if(sectionToTakeFrom == -1){
    throw runtime_error("out of memory.");
  }

  if(DEBUG){
    cout << "Malloc will take from section #" << sectionToTakeFrom << ".\n";
  }

  if(freeList[sectionToTakeFrom] == NULL){
    splitToGetSomeInSection(sectionToTakeFrom);
  }
  Addr ret = (Addr) freeList[sectionToTakeFrom] + sizeof(Header);
  freeList[sectionToTakeFrom]->filled = true;
  freeList[sectionToTakeFrom] = freeList[sectionToTakeFrom]->next;
  return ret;
  //return malloc((size_t)_length);
}


/*void coalesce(){
  for(int i = 0; i < freeListSize; i++){
    Header* top = freeListSize[i];
    while(top != NULL){
      top = top->next;
    }
  }
}*/

Addr combineWithBuddy(Addr a){
  //Returns addr of new block if combined, null if not.
  Header* h = (Header*) a;
  int section = log2(( h -> size)/_bbs);

  if(section > freeListSize-1){
    return NULL;
  }

  if(DEBUG){
    cout << "About to calc buddy, using size " << h->size << "!\n";
  }

  Header* buddy = (Header*) getBuddyAddress(h, h->size);

  if(!buddy->filled && buddy->size == h->size){

    Addr ret;

    Header* nextInChain = freeList[section+1];

    //Find which buddy is leftmost.
    if((int) h < (int) buddy){
      freeList[section+1] = h;
      h->size = h->size*2;
      ret = (Addr) h;
    }
    else{
      freeList[section+1] = buddy;
      buddy->size = buddy->size * 2;
      ret = (Addr) buddy;
    }

    //Delete h and buddy from freeList chain
    
    Header* cur = freeList[section];
    Header* prev = NULL;

    while(cur != NULL){

      if(DEBUG){
        cout << "Traversing freeList section to delete h and buddy. Addr: " << (Addr) cur << "\n";
        cout << "H = " << (Addr) h << "\n";
        cout << "BUDDY = " << (Addr) buddy << "\n";
      }

      if(cur == h){
        if(prev == NULL){
          freeList[section] = cur->next;
        }
        else{
          prev->next = cur->next;
          prev = cur;
        }
      }
      else if(cur == buddy){
        if(prev == NULL){
          freeList[section] = cur->next;
        }
        else{
          prev->next = cur->next;
          prev = cur;
        }
      }
      cur = cur->next;
    }

    ((Header*) ret )-> next = nextInChain;

    if(DEBUG){
      cout << "Returning from combining buddies.\n";
      printFreeList();
    }

    return ret;
    
  }
  else{
    return NULL;
  }
}

extern int my_free(Addr _a) {

  if(!USE_MY_MALLOC){
    free(_a);
    return 0;
  }

  Addr headerAddress = _a - sizeof(Header);

  if(DEBUG){
    cout << "Free request at address: " << _a << "(" << headerAddress << ")" << "\n";
    printFreeList();
  }

  Header* h = (Header*) headerAddress;
  h->filled = false;
  
  int sectionToPut = log2((h->size)/_bbs);

  h->next = freeList[sectionToPut];
  freeList[sectionToPut] = h;

  if(DEBUG){
    cout << "Put back freed header.\n";
    printFreeList();
  }

  
  Addr toCombine = headerAddress;
  while(toCombine != NULL ){
    if(DEBUG){
      //cout << "Combining with buddy.\n";
    }
    toCombine = combineWithBuddy(toCombine);
  }

  //free(_a);
  return 0;
}

