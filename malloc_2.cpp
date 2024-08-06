
#include <unistd.h>
#include <string.h>

void * head_list =  NULL;
typedef struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;

} MallocMetadata;

struct stats{
    size_t list_size_nodes;
    size_t list_size_sizes;
    size_t blocks_number_overall;
    size_t blocks_number_overall_bytes;
};

struct stats Statistics = {0,0,0,0}; 

void* smalloc(size_t size){
    if(size == 0 ||size > 100000000){
        return NULL;
    }

    size_t metadataSize = sizeof(MallocMetadata);
    MallocMetadata* ptr = head_list;
    
    while (ptr != NULL) {
        if(ptr->size >= size){
            if(ptr->prev != NULL ){ 
                ptr->prev->next = ptr->next;
            }else{
                head_list = ptr->next;
            }
            if( ptr->next != NULL ){
                ptr->next->prev == ptr->prev;
            }
            ptr->is_free = false;
            Statistics.list_size_nodes--;
            Statistics.list_size_sizes -= ptr->size;
            return ptr + metadataSize;
        }
        ptr = ptr->next;
    }

    void * res = NULL;
    res = sbrk(size + metadataSize); 
    if(res == (void *)(-1) ){
        return NULL;
    }

    Statistics.blocks_number_overall++;
    Statistics.blocks_number_overall_bytes += size;
    
    MallocMetadata* metadata = (MallocMetadata *)res;
    metadata->size = size;
    metadata->is_free = false;
    return res + metadataSize;
}

void* scalloc(size_t num, size_t size){
    void * res = smalloc(size * num);
    if(res == NULL) return NULL;
    memset(res, 0, size * num);
    return res;
}

void sfree(void* p){
    size_t metadataSize = sizeof(MallocMetadata);

    MallocMetadata* ptr = (MallocMetadata*)(p - metadataSize);
    if(p == NULL || ptr->is_free == true ){
        return;
    }
    
    ptr->is_free = true;
    Statistics.list_size_nodes++;
    Statistics.list_size_sizes += ptr->size;
    if(head_list == NULL){
        head_list = ptr; 
        ptr->next = NULL; 
        ptr->prev = NULL;   
        return;
    }

    MallocMetadata* iterator = (MallocMetadata*)head_list;
    while(iterator < ptr || iterator->next == NULL){
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
        head_list = ptr;
    }
    iterator->prev = ptr;
}

void* srealloc(void* oldp, size_t size){
    size_t metadataSize = sizeof(MallocMetadata);
    if(oldp == NULL){
        return smalloc(size);
    }
    
    MallocMetadata* ptr = (MallocMetadata*)(oldp - metadataSize);
    
    if(ptr->size >= size){
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
}