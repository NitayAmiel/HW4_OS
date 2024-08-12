#include <iostream>
#include <cassert>
#include <cstring>
#include "malloc_3.cpp"

// Assuming the existence of the functions and the MallocMetadata structure.
extern void* smalloc(size_t size);
extern size_t _num_free_blocks();
extern size_t _num_free_bytes();
extern size_t _num_allocated_blocks();
extern size_t _num_allocated_bytes();
extern size_t _num_meta_data_bytes();
extern size_t _size_meta_data();

void test_smalloc() {
    size_t meta_data_size = _size_meta_data();

    // Test: Allocation of 10 bytes
    void* p1 = smalloc(10);
    assert(p1 != NULL);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 10);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);

    // Test: Allocation of 50 bytes
    void* p2 = smalloc(50);
    assert(p2 != NULL);
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 60);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);

    // Test: Allocation of 0 bytes should return NULL
    void* p3 = smalloc(0);
    assert(p3 == NULL);

    // Test: Allocation of more than 108 bytes should return NULL
    void* p4 = smalloc(109);
    assert(p4 == NULL);
}

extern void* scalloc(size_t num, size_t size);

void test_scalloc() {
    size_t meta_data_size = _size_meta_data();

    // Test: Allocation of 10 elements, each of size 5 bytes
    void* p1 = scalloc(10, 5);
    assert(p1 != NULL);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 50);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);

    // Verify that all allocated bytes are set to 0
    for (size_t i = 0; i < 50; i++) {
        assert(((char*)p1)[i] == 0);
    }

    // Test: Allocation of 0 elements or 0 size should return NULL
    void* p2 = scalloc(0, 10);
    assert(p2 == NULL);

    void* p3 = scalloc(10, 0);
    assert(p3 == NULL);

    // Test: Allocation of size * num greater than 108 should return NULL
    void* p4 = scalloc(10, 11);
    assert(p4 == NULL);
}
extern void sfree(void* p);

void test_sfree() {
    void* p1 = smalloc(10);
    void* p2 = smalloc(20);

    // Test: Freeing p1
    sfree(p1);
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 10);

    // Test: Freeing p2
    sfree(p2);
    assert(_num_free_blocks() == 2);
    assert(_num_free_bytes() == 30);

    // Test: Freeing a NULL pointer should have no effect
    sfree(NULL);
    assert(_num_free_blocks() == 2);
    assert(_num_free_bytes() == 30);

    // Test: Freeing an already freed pointer should have no effect
    sfree(p1);
    assert(_num_free_blocks() == 2);
    assert(_num_free_bytes() == 30);
}
extern void* srealloc(void* oldp, size_t size);

void test_srealloc() {
    void* p1 = smalloc(10);

    // Test: Reallocation to a larger size
    void* p2 = srealloc(p1, 20);
    assert(p2 != NULL);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 20);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);

    // Test: Reallocation to a smaller size
    void* p3 = srealloc(p2, 5);
    assert(p3 != NULL);
    assert(_num_allocated_blocks() == 1);
    assert(_num_allocated_bytes() == 20);  // original block size remains
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);

    // Test: Reallocation with NULL pointer should allocate new block
    void* p4 = srealloc(NULL, 10);
    assert(p4 != NULL);
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 30);
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);

    // Test: Reallocation to size 0 should return NULL
    void* p5 = srealloc(p4, 0);
    assert(p5 == NULL);
}


void test_stats() {
    // Initial state
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_allocated_blocks() == 0);
    assert(_num_allocated_bytes() == 0);
    assert(_num_meta_data_bytes() == 0);
    assert(_size_meta_data() > 0);

    void* p1 = smalloc(10);
    void* p2 = smalloc(20);

    // After two allocations
    assert(_num_free_blocks() == 0);
    assert(_num_free_bytes() == 0);
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 30);
    assert(_num_meta_data_bytes() == 2 * _size_meta_data());

    sfree(p1);

    // After freeing one block
    assert(_num_free_blocks() == 1);
    assert(_num_free_bytes() == 10);
    assert(_num_allocated_blocks() == 2);
    assert(_num_allocated_bytes() == 30);
    assert(_num_meta_data_bytes() == 2 * _size_meta_data());
}


int main() {
    if(fork() == 0) {
    test_smalloc();
    std::cout << "smalloc tests passed!" << std::endl;
    exit(0);
}
    if(fork() == 0) {
    test_scalloc();
    std::cout << "scalloc tests passed!" << std::endl;
    exit(0);
    }
    if(fork() == 0){
    test_sfree();
    std::cout << "sfree tests passed!" << std::endl;
    exit(0);
    }
    if(fork() == 0){
    test_srealloc();
    std::cout << "srealloc tests passed!" << std::endl;
    }
    if(fork() ==0 ){
    test_stats();
    std::cout << "Stats functions tests passed!" << std::endl;
    return 0;
}
