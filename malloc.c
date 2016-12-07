#include <stdlib.h>
#include <stdio.h>
#include "heaplib.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#define ADD_BYTES(base_addr, num_bytes) (((char *)(base_addr)) + (num_bytes))
#define SUB_BYTES(base_addr, num_bytes) (((char *)(base_addr)) - (num_bytes))
/* You must implement these functions according to the specification
 *
 * Student 1 Name: Leah Greenstein
 * Student 1 NetID: lrg79
 * Student 2 Name: Max Chu
 * Student 2 NetID: mmc278
 * 
 * Include a description of your approach here.
 *
 */

//heap header, tells you how many remaining
//units there are

 //24 bytes
typedef struct block_header{
  void* prev;
  void* next;
  int size; //start of payload to beginning of footer
  bool in_use;
} block_header;

typedef struct footer{
  unsigned int size;
} footer;

//4 bytes
typedef struct meta{
    unsigned int remaining;
    block_header * block; //ptr to first free block
    unsigned int size;
} meta;

void * align(void* ptr){
 unsigned long ptr_int = (unsigned long) ptr;
 ptr_int = ptr_int - ((unsigned long)ptr_int % 8);
 return (void *)ptr_int;
}
unsigned int resize_payload(unsigned int payload_size){
  unsigned int allocSize;
  if(payload_size <= 4){
    allocSize = 4;
  }
  else{
    //making sure will be 8-byte aligned
    int mod;
    mod = payload_size % 8;
    if (mod < 4){
      allocSize = payload_size + 4 - mod;
    }
    else{
      if (mod == 4){
        allocSize = payload_size;
      }
      if (mod == 5){
        allocSize = payload_size + 7;
      }
      if (mod == 6){
        allocSize = payload_size + 6;
      }
      if (mod == 7){
        allocSize = payload_size + 5;
      }
    }
  }
  return allocSize;
}

int hl_init(void *heap_ptr, unsigned int heap_size) {
  if(!heap_ptr)return FAILURE;
  if(heap_size<=0) return FAILURE;

  unsigned long rem = 8- ((unsigned long) heap_ptr%8);

  heap_ptr = align(heap_ptr);

  if(sizeof(meta) + sizeof(block_header) + sizeof(footer) > heap_size)return FAILURE;

  meta* heap = (meta*) heap_ptr;
  heap->remaining = heap_size - sizeof(meta) - sizeof(block_header)- sizeof(footer);
  block_header* block = (block_header*)ADD_BYTES(heap, sizeof(meta));
  block->in_use=false;
  block->prev=NULL;
  block->next=NULL;
  block->size= heap_size-sizeof(meta)-sizeof(block_header)-sizeof(footer);
  heap->block = block;
  footer* foot= (footer*)ADD_BYTES(block, sizeof(block_header) + block->size);
  foot->size = block->size;
    #ifdef PRINT_DEBUG
    printf("\n");
    printf("%s\n", "hl init complete");
    printf("%s%u\n", "block's size: ", block->size);
    printf("%s%u\n", "footer's size: ", foot->size);
    printf("%s%u\n", "remaining: ", heap->remaining);
    printf("\n");
    #endif
  heap->size = heap_size - rem;
  return SUCCESS;

}

void *hl_alloc(void *heap_ptr, unsigned int payload_size) {
  if(!heap_ptr)return FAILURE;
  heap_ptr = align(heap_ptr);
  payload_size = resize_payload(payload_size);
  meta* heap = (meta*) heap_ptr;

  //making sure there is enough room to alloc another block
  bool makeBlock = false;

  if(heap->remaining<payload_size) return FAILURE;

  block_header* block = heap->block;
  while(block != NULL){
    if(block->size > payload_size+sizeof(block_header)+sizeof(footer) && block->in_use == false){
        makeBlock = true;
        break;
    }
    else if(block->size >= payload_size && block->in_use == false){
      makeBlock=false;
      break;
    }
    else{
      block=block->next;
    }
  }

  if(block == NULL) return FAILURE;

  if(makeBlock == true){
    block->in_use = true;
    int orgSize = block->size;
    block_header* next = block->next;
    heap->remaining = heap->remaining - sizeof(block_header)- sizeof(footer) - payload_size;
    //new block and new block foot will go behind
    footer* block_foot = (footer*) ADD_BYTES(block, sizeof(block_header) + payload_size);
    block_header* new_block = (block_header*)ADD_BYTES(block, sizeof(block_header)+payload_size+ sizeof(footer));
    new_block->size= orgSize - payload_size - sizeof(block_header) - sizeof(footer);
    block->size=payload_size;
    block_foot->size=block->size;
    footer* new_foot = (footer*) ADD_BYTES(new_block, sizeof(block_header) + new_block->size);
    new_foot->size=new_block->size;
    //adjusting these pointers
    block->next = new_block;
    new_block->in_use = false;
    new_block->prev = block;
    new_block->next = next;
      #ifdef PRINT_DEBUG
      printf("\n");
      printf("%s%u\n", "block's size: ", block->size);
      printf("%s%u\n", "new block's size: ", new_block->size);
      printf("%s%u\n", "footer's size: ", block_foot->size);
      printf("%s%u\n", "new foot's size: ", new_foot->size);
      printf("%s%u\n", "remaining: ", heap->remaining);
      printf("%s%u\n", "nextBlock: ", heap->remaining);
      printf("%s%p\n", "new block address: ", new_block);
      printf("%s%p\n", "block pointer: ", block);
      printf("%s%p\n", "block's next: ", block->next);
      printf("%s%p\n", "new block's next: ", next);
      printf("%s%p\n", "prevBlock: ", block->prev);
      printf("\n");
      #endif

    if(next != NULL){
      next->prev = new_block;
    }


    return ADD_BYTES(block, sizeof(block_header));
  }

  else{

    #ifdef PRINT_DEBUG
    printf("\n");
    printf("%s\n", "make block is false");
    printf("\n");
    #endif

    block->in_use=true;
    heap->remaining = heap->remaining - block->size;

    #ifdef PRINT_DEBUG
    printf("\n");
    printf("%s\n", "make block succeeded");
    printf("%s%u\n", "new block's size: ", block->size);
    printf("%s%u\n", "remaining: ", heap->remaining);
    printf("%s%p\n", "block pointer: ", block);
    printf("%s%p\n", "block's next: ", block->next);
    printf("%s%p\n", "block's prev: ", block->prev);
    printf("\n");
    #endif

    return ADD_BYTES(block, sizeof(block_header));
  }
  return FAILURE;
}


void hl_release(void *heap_ptr, void *payload_ptr) {
  if(payload_ptr == 0) return;
  block_header* block = (block_header*) SUB_BYTES(payload_ptr, sizeof(block_header));
  meta* heap = (meta*) heap_ptr;
  block_header* prevBlock = block->prev;
  block_header* nextBlock = block->next;
  bool hasNext = true;
  if(nextBlock == NULL || (void*) nextBlock > (void *)ADD_BYTES(heap, heap->size) || (void *)nextBlock < heap_ptr){
    hasNext = false;
  }
  bool hasPrev = true;
  if(prevBlock == NULL || (void*) prevBlock < heap_ptr || (void*) prevBlock > (void *)ADD_BYTES(heap,heap->size)){
    hasPrev = false;
  }

  if((hasPrev == false ||(hasPrev == true && prevBlock != NULL && prevBlock->in_use == true)) && (hasNext == false ||(hasNext == true && nextBlock != NULL && nextBlock->in_use == true))){
    //case1
    heap->remaining = heap->remaining + block->size;
    block->in_use=false;

            #ifdef PRINT_DEBUG
            printf("\n");
            printf("%s\n", "case 1");
            printf("%s%p\n", "block: ", block);
            printf("%s%u\n", "released block's size: ", block->size);
            printf("%s%p\n", "prevBlock: ", prevBlock);
            printf("%s%p\n", "nextBlock: ", nextBlock);
            printf("%s%u\n", "heap's remaining: ", heap->remaining);
            printf("\n");
            #endif

    return;
  }
  else if((prevBlock == NULL || (hasPrev == true && prevBlock!=NULL&&prevBlock->in_use==true)) && (hasNext == true && nextBlock!=NULL&&nextBlock->in_use==false)){
    //case2
            #ifdef PRINT_DEBUG
            printf("\n");
            printf("%s\n", "case 2");
            #endif
    block->next=nextBlock->next;
    block_header* nextBlockNext = NULL;
    if(block->next != NULL){
      nextBlockNext = nextBlock->next; 
    }
    footer*new_foot= (footer*)ADD_BYTES(block, sizeof(block_header) + block->size + sizeof(footer)+ sizeof(block_header) + nextBlock->size);
    block->in_use=false;
    heap->remaining=heap->remaining+sizeof(footer)+sizeof(block_header)+block->size;
    block->size=block->size+sizeof(footer)+sizeof(block_header)+nextBlock->size;
    new_foot->size=block->size;

            #ifdef PRINT_DEBUG
            printf("%s%p\n", "released block's address: ", block);
            printf("%s%u\n", "block's new size: ", block->size);
            printf("%s%u\n", "footer's size: ", new_foot->size);
            printf("%s%p\n", "prevBlock: ", prevBlock);
            printf("%s%p\n", "nextBlock: ", nextBlock);
            printf("%s%u\n", "heap remaining: ", heap->remaining);
            printf("\n");
            #endif
    if(nextBlock->next != NULL && nextBlockNext != NULL) nextBlockNext->prev = block;
    return;
  }
  else if((hasPrev == true && prevBlock != NULL && prevBlock->in_use==false) && (hasNext == false || (hasNext == true && nextBlock!=NULL && nextBlock->in_use==true))){
   //case 3
            #ifdef PRINT_DEBUG
            printf("\n");
            printf("%s\n", "case 3");
            printf("\n");
            printf("%s%p\n", "block: ", block);
            printf("%s%u\n", "released block's size: ", block->size);
            printf("%s%p\n", "prevBlock: ", prevBlock);
            printf("%s%u\n", "heap's remaining: ", heap->remaining);
            #endif
    footer* foot = (footer*) ADD_BYTES(block, sizeof(block_header)+ block->size);
    prevBlock->size=prevBlock->size+sizeof(block_header)+sizeof(footer)+block->size;
    heap->remaining = heap->remaining + sizeof(block_header) + sizeof(footer) + block->size;
    foot->size = prevBlock->size;
    prevBlock->next = nextBlock;
    if(nextBlock != NULL){
      nextBlock->prev = prevBlock;
    } 
            printf("\n");
    return;
  }
  else if((hasPrev == true && prevBlock != NULL && prevBlock->in_use == false) && (nextBlock!=NULL && nextBlock->in_use==false)){
    //case4
    block_header* nextBlockNext = NULL;
    if(nextBlock !=NULL){
      nextBlockNext = nextBlock->next;
    }
    heap->remaining = heap->remaining + sizeof(footer) + sizeof(block_header) + block->size + sizeof(footer) + sizeof(block_header);
    prevBlock->next = nextBlock->next;
    prevBlock->size=prevBlock->size+sizeof(block_header)+block->size+sizeof(footer)+sizeof(block_header)+nextBlock->size;
    footer* next_foot= (footer*)ADD_BYTES(prevBlock, sizeof(block_header) + prevBlock->size);
    next_foot->size=prevBlock->size;
            #ifdef PRINT_DEBUG
            printf("\n");
            printf("%s\n", "case 4.1");
            printf("\n");
            printf("%s%p\n", "block: ", block);
            printf("%s%u\n", "released block's size: ", block->size);
            printf("%s%p\n", "prevBlock: ", prevBlock);
            printf("%s%p\n", "nextBlock: ", nextBlock);
            printf("%s%u\n", "nextBlock filled: ", nextBlock->in_use);
            printf("%s%u\n", "heap's remaining: ", heap->remaining);
            printf("\n");
            #endif

    if(nextBlock != NULL && nextBlockNext != NULL && nextBlock->next != NULL){
        nextBlockNext->prev = prevBlock;
    } 
        #ifdef PRINT_DEBUG
            printf("\n");
            printf("%s\n", "case 4.2");
            printf("\n");
            printf("%s%p\n", "block: ", block);
            printf("%s%u\n", "released block's size: ", block->size);
            printf("%s%p\n", "prevBlock: ", prevBlock);
            printf("%s%p\n", "nextBlock: ", nextBlock);
            printf("%s%u\n", "nextBlock filled: ", nextBlock->in_use);
            printf("%s%u\n", "heap's remaining: ", heap->remaining);
            printf("\n");
            #endif
    return;
  }
}

void *hl_resize(void *heap_ptr, void *payload_ptr, unsigned int new_size) {
  //behave same as alloc
  if(payload_ptr ==0) return hl_alloc(heap_ptr, new_size);

  block_header* block = (block_header*) SUB_BYTES(payload_ptr, sizeof(block_header));

  int blockSize = block->size;

  int toCopy = (blockSize > new_size)? new_size:blockSize;

  void* ptr = hl_alloc(heap_ptr, new_size);

  if(ptr == FAILURE) return FAILURE;

  memcpy(ptr, payload_ptr, toCopy);

  hl_release(heap_ptr, payload_ptr);
  
  return ptr;
}
