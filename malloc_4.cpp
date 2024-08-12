#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdint.h>

#define __ALIGN_KERNEL_MASK(x, mask)	(((x) + (mask)) & ~(mask))
#define __ALIGN_KERNEL(x, a)		__ALIGN_KERNEL_MASK(x, (__typeof__(x))(a) - 1)
#define ALIGN(x, a)		__ALIGN_KERNEL((x), (a))
typedef struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
} MallocMetadata;

MallocMetadata *head_array[11];
char is_initialized = 0;
const size_t META_DATA_SIZE = sizeof(MallocMetadata);
const size_t THRESH_HOLD_HUGE_PAGE = 4*1000*1000;
const int FLAGS_HUGE = MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB;
const int FLAGS_NOT_HUGE = MAP_PRIVATE | MAP_ANONYMOUS;
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
    MallocMetadata**  head_list = head_array+idx;
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
                ptr->next->prev = ptr->prev;
            }
            ptr->is_free = false;
            Statistics.list_size_nodes--;
            Statistics.list_size_sizes -= ptr->size;
            char* addr= (char*)ptr;
            return (void*)(addr + META_DATA_SIZE);
        }
        ptr = ptr->next;
    }    
    return NULL;
}

void sfree_2(void* p){

    MallocMetadata* ptr = (MallocMetadata*)((char*)p - META_DATA_SIZE);
    if(p == NULL || ptr->is_free == true ){
        return;
    }
    char opt_idx = get_optml_block(ptr->size);
    MallocMetadata** head_list = head_array+opt_idx;
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
    }else{
        iterator->prev->next = ptr;
    }
    iterator->prev = ptr;
}

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
    return Statistics.blocks_number_overall*META_DATA_SIZE;
}

size_t _size_meta_data(){
    return META_DATA_SIZE;
}


bool _init_malloc(){
    void *current_brk = sbrk(0);
    if(is_initialized != 0 || current_brk == (void *)-1){
        return false;
    }
    unsigned int num_blocks = 32;
    unsigned int block_size = 128 * 1024;
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
    char* addr = (char*)tmp;
    for(unsigned int i = 0; i < num_blocks; i++){
        tmp->is_free = true;
        tmp->size = block_size - META_DATA_SIZE;
        tmp->next = (i == num_blocks - 1) ? NULL: (MallocMetadata* )(addr + block_size);
        tmp->prev = i == 0 ? NULL : (MallocMetadata* )(addr - block_size);
        addr += block_size;
        tmp = (MallocMetadata*)addr;
    }
    Statistics = {num_blocks, num_blocks*(block_size-META_DATA_SIZE), num_blocks, num_blocks*(block_size - META_DATA_SIZE)};
    return true;
}

char get_optml_block(size_t size){
    size += META_DATA_SIZE;
    char counter = 0;
    while(size > 128){
        size++;
        size /= 2;
        ++counter;
    }
    return counter;
}

int get_rlvnt_block(int idx)
{
    while(head_array[idx] == nullptr){
        idx++;
        if(idx == 11){
            return -1;
        }
    }
    return idx;
}
/*
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
*/

void remove_from_list(MallocMetadata * item, int index){
    MallocMetadata * head = head_array[index];
    if(head == nullptr || item == NULL){
        return;
    }
    /*if(item == head){
        head_array[index] = item->next;
        return;
    }*/
    MallocMetadata* iterator = head;
    while(iterator != item && iterator->next != nullptr){
        iterator = iterator->next;
    }

    if(iterator == item) {
        if(iterator->prev){
            iterator->prev->next = iterator->next;
        }else{
            head_array[index] = iterator->next;
        }
        if(iterator->next){
            iterator->next->prev = iterator->prev;
        }
        Statistics.list_size_nodes--;
        Statistics.list_size_sizes -= item->size;
    }

}

MallocMetadata * merge_budds(MallocMetadata* item, MallocMetadata* buddy, int order){
    remove_from_list(item, order);
    remove_from_list(buddy, order);
    item = (item > buddy) ? buddy : item;
    item->size *= 2;
    item->size += META_DATA_SIZE;
    item->is_free = false;
    char * item_addr = (char *)item;
    sfree_2(item_addr + META_DATA_SIZE);
    Statistics.blocks_number_overall -= 1;
    Statistics.blocks_number_overall_bytes += META_DATA_SIZE;
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
    char * second_blk_addr = (char *)second_blk;
    sfree_2(second_blk_addr + META_DATA_SIZE);
    Statistics.blocks_number_overall += 1;
    Statistics.blocks_number_overall_bytes -= META_DATA_SIZE;

    //add_to_list(second_blk, get_optml_block(second_blk->size));
}

void* mmap_handle(size_t size,int flags ){
    void * base_address = mmap(NULL, size + META_DATA_SIZE, PROT_READ | PROT_WRITE , flags, -1, 0);
    MallocMetadata* ptr_meta_data;

    if (base_address == MAP_FAILED) {
        return NULL;
    }
    ptr_meta_data = (MallocMetadata*)base_address;
    ptr_meta_data->size = size;
    ptr_meta_data->is_free = false;
    Statistics.blocks_number_overall++;
    Statistics.blocks_number_overall_bytes += size;
    return (char *)base_address+META_DATA_SIZE;
}

void munmmap_handle(MallocMetadata* meta_data, size_t size){
    size_t tmp = meta_data->size;

    int res = (size == 0) ? munmap((void*)meta_data, meta_data->size + META_DATA_SIZE) : munmap((void*)meta_data, size) ;
    if(res == 0){
        Statistics.blocks_number_overall--;
        Statistics.blocks_number_overall_bytes -= tmp;
    }
}
void* smalloc(size_t size)
{
    if(size == 0 || size > 100000000){
        return NULL;
    }
    if(size >= THRESH_HOLD_HUGE_PAGE ){
        return mmap_handle(size,FLAGS_HUGE );
    }
    char idx_optml_block = get_optml_block(size);
    if(idx_optml_block > 10){
        //TODO : MMAP
        return mmap_handle(size, FLAGS_NOT_HUGE);
    }
    if(is_initialized == 0){
        if(!_init_malloc()){
            return NULL;
        }
        is_initialized  = 1;
    }
    char idx_of_rlvnt_blk = get_rlvnt_block(idx_optml_block);
    if(idx_of_rlvnt_blk < 0){
        return NULL;
    }
    char tmp = idx_of_rlvnt_blk;
    void * allocated_block = smalloc_2(size, tmp);
    if(!allocated_block){
        return NULL;
    }

    while(tmp > idx_optml_block){
        divide_blk_to_2((char *)allocated_block - META_DATA_SIZE);
        tmp--;
    }
    return allocated_block;
}


void sfree(void* p){
    if( p == NULL ) {
        return;
    }
    MallocMetadata * meta_data = (MallocMetadata*)((char *)p - META_DATA_SIZE);
    if(meta_data->is_free == true){
        return;
    }
    size_t size = meta_data->size;
    if(size >= THRESH_HOLD_HUGE_PAGE ){
        munmmap_handle(meta_data , ALIGN(size, THRESH_HOLD_HUGE_PAGE) );
        return;
    }
    int order = get_optml_block(meta_data->size);
    if(order > 10){
        //statistics
        munmmap_handle(meta_data, 0);
        return;
    }
    meta_data->is_free = false;
    void * meta_data_addr = (void *)meta_data;
    sfree_2((char *)meta_data_addr + META_DATA_SIZE);
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

void* scalloc(size_t num, size_t size){
    void * res = smalloc(size * num);
    if(res == NULL) return NULL;
    memset(res, 0, size * num);
    return res;
}


MallocMetadata* get_my_buddy(MallocMetadata * meta_data){
    return (MallocMetadata*)((uintptr_t)meta_data ^ (meta_data->size + META_DATA_SIZE));
}


void* merge_budds_up_to_size(MallocMetadata* ptr, int counter){
    int order = get_optml_block(ptr->size);
    while(counter >0){
        ptr = merge_budds(ptr, get_my_buddy(ptr),order);
        counter--;
        order = get_optml_block(ptr->size);
    }
    remove_from_list(ptr, order);
    ptr->is_free = false;
    return ptr;
}

void* srealloc(void* oldp, size_t size){
    if(oldp == NULL){
        return smalloc(size);
    }
    MallocMetadata* meta_data = (MallocMetadata*)((char *)oldp - META_DATA_SIZE);

    if(meta_data->size >= size && size > 0){
        return oldp;
    }

    size_t current_size = meta_data->size ;
    int order = get_optml_block(current_size);
    int counter = 0;

    MallocMetadata * my_buddy = (MallocMetadata*)((uintptr_t)meta_data ^ (current_size + META_DATA_SIZE));
    while(my_buddy->is_free ){
        counter++;
        if(order++ >= 10){
            //no relevant merge
            break;
        }
        if(my_buddy->size != current_size){
            break;
        }
        if(my_buddy->size + current_size+META_DATA_SIZE >= size){
            return (char*)merge_budds_up_to_size((MallocMetadata*)((char *)oldp-META_DATA_SIZE), counter) + META_DATA_SIZE;
        }
        current_size =(current_size *2) + META_DATA_SIZE;
        my_buddy = (MallocMetadata*)((uintptr_t)meta_data ^ (current_size  + META_DATA_SIZE));
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
*/