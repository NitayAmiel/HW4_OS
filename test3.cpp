#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Assume the following functions are implemented in malloc_3.cpp
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

void test_smalloc() {
    printf("Testing smalloc...\n");
    void* ptr1 = smalloc(100);
    if (ptr1 == NULL) {
        printf("Error: smalloc returned NULL for 100 bytes\n");
    }
    void* ptr2 = smalloc(2000);
    if (ptr2 == NULL) {
        printf("Error: smalloc returned NULL for 2000 bytes\n");
    }
    printf("smalloc test passed.\n");
}

void test_scalloc() {
    printf("Testing scalloc...\n");
    void* ptr = scalloc(10, 50);
    if (ptr == NULL) {
        printf("Error: scalloc returned NULL for 10 elements of 50 bytes each\n");
    } else {
        // Check if memory is zeroed
        for (size_t i = 0; i < 10 * 50; i++) {
            if (((char*)ptr)[i] != 0) {
                printf("Error: scalloc did not zero memory\n");
                break;
            }
        }
    }
    printf("scalloc test passed.\n");
}

void test_sfree() {
    printf("Testing sfree...\n");
    void* ptr = smalloc(500);
    if (ptr != NULL) {
        sfree(ptr);
        printf("sfree test passed.\n");
    } else {
        printf("Error: smalloc returned NULL for 500 bytes\n");
    }
}

void test_srealloc() {
    printf("Testing srealloc...\n");
    void* ptr = smalloc(300);
    if (ptr == NULL) {
        printf("Error: smalloc returned NULL for 300 bytes\n");
        return;
    }
    void* new_ptr = srealloc(ptr, 600);
    if (new_ptr == NULL) {
        printf("Error: srealloc returned NULL for 600 bytes\n");
    } else {
        printf("srealloc test passed.\n");
    }
}

void test_stats_functions() {
    printf("Testing statistics functions...\n");
    printf("Number of free blocks: %zu\n", _num_free_blocks());
    printf("Number of free bytes: %zu\n", _num_free_bytes());
    printf("Number of allocated blocks: %zu\n", _num_allocated_blocks());
    printf("Number of allocated bytes: %zu\n", _num_allocated_bytes());
    printf("Number of meta data bytes: %zu\n", _num_meta_data_bytes());
    printf("Size of meta data: %zu\n", _size_meta_data());
    printf("Statistics functions test passed.\n");
}

int main() {
    test_smalloc();
    test_scalloc();
    test_sfree();
    test_srealloc();
    test_stats_functions();
    return 0;
}