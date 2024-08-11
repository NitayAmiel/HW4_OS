
#include <unistd.h>
#include <string.h>

typedef struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
} MallocMetadata;

MallocMetadata *head_array[11];
char is_initialized = 0;
const size_t META_DATA_SIZE = sizeof(MallocMetadata);

struct stats{
    size_t list_size_nodes;
    size_t list_size_sizes;
    size_t blocks_number_overall;
    size_t blocks_number_overall_bytes;
};

struct stats Statistics = {0,0,0,0}; 

char get_optml_block(size_t size);

void* smalloc_2(size_t size,char idx ){
    if(size == 0 ||size > 100000000){
        return NULL;
    }
    MallocMetadata**  head_list = &head_array[idx];
    MallocMetadata* ptr = *head_list;
    //size += META_DATA_SIZE;
    while (ptr != NULL) {
        if(ptr->size >= size){
            if(ptr->prev != NULL ){ 
                ptr->prev->next = ptr->next;
            }else{
                *head_list = ptr->next;
            }
            if( ptr->next != NULL ){
                ptr->next->prev == ptr->prev;
            }
            ptr->is_free = false;
            Statistics.list_size_nodes--;
            Statistics.list_size_sizes -= ptr->size;
            return ptr + META_DATA_SIZE;
        }
        ptr = ptr->next;
    }    
    return NULL;
}
/*
void* scalloc_2(size_t num, size_t size){
    void * res = smalloc(size * num);
    if(res == NULL) return NULL;
    memset(res, 0, size * num);
    return res;
}
*/
void sfree_2(void* p){

    MallocMetadata* ptr = (MallocMetadata*)(p - META_DATA_SIZE);
    if(p == NULL || ptr->is_free == true ){
        return;
    }
    char opt_idx = get_optml_block(ptr->size);
    MallocMetadata** head_list = &head_array[opt_idx];
    ptr->is_free = true;
    Statistics.list_size_nodes++;
    Statistics.list_size_sizes += ptr->size;
    if(*head_list == NULL){
        *head_list = ptr; 
        ptr->next = NULL; 
        ptr->prev = NULL;   
        return;
    }

    MallocMetadata* iterator = *head_list;
    while(iterator < ptr && iterator->next != NULL){
        iterator = iterator->next;
    }

    if(iterator->next == NULL && iterator < ptr) //the end of the list
    {
        iterator->next = ptr;
        ptr->prev = iterator;
        ptr->next = NULL;
        return;
    }

    ptr->next = iterator;
    ptr->prev = iterator->prev;

    if(iterator->prev == NULL)
    {
        *head_list = ptr;
    }
    iterator->prev = ptr;
}
/*
void* srealloc_2(void* oldp, size_t size){
    size_t metadataSize = sizeof(MallocMetadata);
    if(oldp == NULL){
        return smalloc(size);
    }
    
    MallocMetadata* ptr = (MallocMetadata*)(oldp - metadataSize);
    
    if(ptr->size >= size && size > 0){
        return oldp;
    }
    
    void* res = smalloc(size);
    if(res == NULL){
        return NULL;
    }

    memmove(res, oldp, size);
    sfree(oldp);
    return res;
}

/*
size_t _num_free_blocks(){
    return Statistics.list_size_nodes;
}

size_t _num_free_bytes(){
    return Statistics.list_size_sizes;
}

size_t _num_allocated_blocks(){
    return Statistics.blocks_number_overall;
}

size_t _num_allocated_bytes(){
    return Statistics.blocks_number_overall_bytes;
}

size_t _num_meta_data_bytes(){
    return Statistics.blocks_number_overall*sizeof(MallocMetadata);
}

size_t _size_meta_data(){
    return sizeof(MallocMetadata);
}*/

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

bool _init_malloc(){
    void *current_brk = sbrk(0);
    if(is_initialized != 0 || current_brk == (void *)-1){
        return false;
    }
    int num_blocks = 32;
    int block_size = 128 * 1024;
    int alignment = num_blocks * block_size;
    uintptr_t current_addr = (uintptr_t)current_brk;
    uintptr_t aligned_addr = (current_addr + (alignment) - 1) & ~(alignment - 1);
    intptr_t gap = aligned_addr - current_addr;

    if (sbrk(gap) == (void *)-1) {
        return false;
    }

    void *aligned_memory = sbrk(num_blocks * block_size);
    if (aligned_memory == (void *)-1) {
        return false;
    }
    
    // Initializing the 32 blocks as free and inserting them to the array and list
    head_array[10] = (MallocMetadata*)aligned_memory;
    MallocMetadata* tmp = head_array[10];
    void* addr = tmp;
    for(int i = 0; i < num_blocks; i++){
        tmp->is_free = true;
        tmp->size = block_size - META_DATA_SIZE;
        tmp->next = (i == num_blocks - 1) ? NULL: (MallocMetadata* )(addr + block_size);
        tmp->prev = i == 0 ? NULL : (MallocMetadata* )(addr - block_size);
        addr += block_size;
        tmp = (MallocMetadata*)addr;
    }
    return true;
}

char get_optml_block(size_t size){
    size += META_DATA_SIZE;
    size = size >> 7;
    char counter = 0;
    while(size != 0){
        size /= 2;
        if(++counter > 10){
            break;
        }
    }
    return counter;
}

char get_rlvnt_block(char idx)
{
    while(head_array[idx] == nullptr){
        idx++;
    }
    return idx;
}

void add_to_list(MallocMetadata * item, int index){
    MallocMetadata * head = head_array[index];
    if(head == nullptr) {
        head_array[index] = item;
        item->next = NULL;
        item->prev = NULL;
        return;
    }

    MallocMetadata* iterator = head;
    while(iterator < item && iterator->next != nullptr){
        iterator = iterator->next;
    }

    if(iterator->next == nullptr && iterator < item) //the end of the list
    {
        iterator->next = item;
        item->prev = iterator;
        item->next = nullptr;
        return;
    }

    item->next = iterator;
    item->prev = iterator->prev;

    if(iterator->prev == nullptr)
    {
        head_array[index] = item;
    }
    iterator->prev = item;
}

void remove_from_list(MallocMetadata * item, int index){
    MallocMetadata * head = head_array[index];

    if(item == head)
        head_array[index] = item->next;

    MallocMetadata* iterator = head;
    while(iterator != item && iterator->next != nullptr){
        iterator = iterator->next;
    }

    if(iterator == item) {
        if(iterator->prev)
            iterator->prev->next = iterator->next;
        if(iterator->next)
            iterator->next->prev = iterator->prev;
    }
}

MallocMetadata * merge_budds(MallocMetadata* item, MallocMetadata* buddy, int order){
    remove_from_list(item, order);
    remove_from_list(buddy, order);
    item = (item > buddy) ? buddy : item;
    item->size *= 2;
    item->size += META_DATA_SIZE;
    void * item_addr = (void *)item;
    sfree_2(item_addr + META_DATA_SIZE);
    //add_to_list(item, order + 1);
    return item;
}

void divide_blk_to_2(void * allocated_block)
{
    MallocMetadata *blk =(MallocMetadata *) allocated_block; 
    blk->size += META_DATA_SIZE;
    blk->size /= 2;
    blk->size -= META_DATA_SIZE;
    MallocMetadata* second_blk = (MallocMetadata*)(void *)((size_t)allocated_block ^ (blk->size + META_DATA_SIZE));
    second_blk->size = blk->size;
    second_blk->is_free = false;//just for enabling handling in sfree_2;
    void * second_blk_addr = (void *)second_blk;
    sfree_2(second_blk_addr + META_DATA_SIZE);
    //add_to_list(second_blk, get_optml_block(second_blk->size));
}

void* smalloc(size_t size)
{
    if(is_initialized == 0){
        if(!_init_malloc()){
            return NULL;
        }
        is_initialized  = 1;
    }
    if(size == 0){
        return NULL;
    }

    char idx_optml_block = get_optml_block(size);
    if(idx_optml_block > 10){
        //TODO : MMAP
        //mmap_handle();
    }
    char idx_of_rlvnt_blk = get_rlvnt_block(idx_optml_block);
    char tmp = idx_of_rlvnt_blk;
    void * allocated_block = smalloc_2(size, tmp);
    if(!allocated_block){
        return NULL;
    }

    while(tmp > idx_optml_block){
        divide_blk_to_2(allocated_block-META_DATA_SIZE);
        tmp--;
    }
    return allocated_block;
}


void sfree(void* p){
    MallocMetadata * meta_data = (MallocMetadata*)(p - META_DATA_SIZE);
    int order = get_optml_block(meta_data->size);

    meta_data->is_free = false;
    void * meta_data_addr = (void *)meta_data;
    sfree_2(meta_data_addr + META_DATA_SIZE);
    if(order == 10) {
        return;
    }
    
    //merge buddies
    MallocMetadata * my_buddy = (MallocMetadata*)((uintptr_t)meta_data ^ (meta_data->size + META_DATA_SIZE));
    while(my_buddy->is_free && order < 10){
        meta_data = merge_budds(meta_data, my_buddy, order);
        my_buddy = (MallocMetadata*)((uintptr_t)meta_data ^ (meta_data->size  + META_DATA_SIZE));
        order = get_optml_block(meta_data->size);
    }
   //add_to_list(meta_data, order);
}