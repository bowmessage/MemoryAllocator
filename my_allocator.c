/* 
    File: my_allocator.c

    Author: James Bowman
            Department of Computer Science
            Texas A&M University
    Date  : 9/4/2013

    Modified: 9/4/2013

    This file contains the implementation of the module "MY_ALLOCATOR".

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include<stdlib.h>
#include "my_allocator.h"
#include "math.h"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

Addr* freeList;

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

unsigned int _bbs;
unsigned int _l;
int freeListSize;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FUNCTIONS FOR MODULE MY_ALLOCATOR */
/*--------------------------------------------------------------------------*/

/* Don't forget to implement "init_allocator" and "release_allocator"! */
extern unsigned int init_allocator(unsigned int _basic_block_size, 
			    unsigned int _length) {
  //Needs to:
  //Initialize free list array
  //Init local vars of length and basic block size
  //Init header of first block (all memory).
  unsigned int ret = 0;

  _l = _length;
  _bbs = _basic_block_size;

  Addr startAddress;
  freeListSize = log(_length / _basic_block_size)/log(2) + 1;
  freeList = new Addr[freeListSize];

  try {
    startAddress = malloc(_length);
    *(Addr*)startAddress = NULL;
    freeList[freeListSize-1] = startAddress;

    ret = _length;
  }
  catch(int e){
    ret = 0;
  }
  return ret;
}

extern int release_allocator() {
}

void splitFirstBlockInSection(int section) {
  if(section == 0) return;
  if(freeList[section] != NULL){
    Addr startOfBlock = freeList[section];
    //Find the end of the next section down to insert it..
    Addr curSpot = freeList[section-1];
    while(*(Addr*)curSpot != NULL){
      curSpot = *(Addr*)curSpot;
    }
    *(Addr*)curSpot = startOfBlock;
    *(Addr*)startOfBlock = startOfBlock + (int)(pow(2,section-1));



    //Remove old block from free list section.
    freeList[section] = *(Addr*)freeList[section];
  }
}

extern Addr my_malloc(unsigned int _length) {

  /* This preliminary implementation simply hands the call over the 
     the C standard library! 
     Of course this needs to be replaced by your implementation.
   {*/
  //Needs to:
  //Find size the smallest possible block to give that is still larger than length
  //Check the freeList to see if we have any free blocks of that size.
  //If not, check up the free list to find a block large enough to be broken down into that size
  //Break blocks down from that block until we have one of the size we need.
  //Now we have the block of the correct size, give it a header and return address to it.
  //

  int freeListSectionToUse = 0;

  for(int i = freeListSize-1; i >= 0; i--){
    if((pow(2,i) * _bbs) < _length){
      freeListSectionToUse = i+1;
      break;
    } 
  }

  if(freeList[freeListSectionToUse] == NULL){
    //We have no free chunks of this size... climb the list til we find something to split
    int freeListSectionToSplit = -1;
    for(int i = freeListSectionToSplit; i < freeListSize; i++){
      if(freeList[i] != NULL){
        freeListSectionToSplit = i;
        break;
      }
    }
    if(freeListSectionToSplit == -1){
      //We have a problem... not enough memory to facilitate this request.
    }
    else{
      //We should start splitting until we have something in the right freeList spot.
      while(freeList[freeListSectionToUse] == NULL){
        splitFirstBlockInSection(freeListSectionToSplit);
        freeListSectionToSplit--;
      }
    }
  }
  else{
    //We have a free chunk of this size! Return it.
  }





  return malloc((size_t)_length);
}


extern int my_free(Addr _a) {
  /* Same here! */
  free(_a);
  return 0;
}

