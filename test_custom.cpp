#include "malloc_3.cpp"
const int MAX_ELEMENT_SIZE = (1024*128);
void* smalloc(size_t size);
void* scalloc(size_t num, size_t size);
void sfree(void* p);
void* srealloc(void* oldp, size_t size);
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();
int main(){

   void* ptr1 = smalloc(40);
    void* ptr2 = srealloc(ptr1, 128*4 -64);

    return 0;

}