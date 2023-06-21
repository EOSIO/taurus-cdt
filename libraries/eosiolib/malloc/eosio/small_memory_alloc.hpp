#pragma once
#include <cstddef>
class small_mem_alloc {
    private:
    // possible sizes of small chunks
    static constexpr size_t _free_list_chunk_size[]=
        { 1 << 6, 1<<7, 1<<8, 1<<9, 1<<10, 1<<11, 1<<12, 1<<13, 1<<14, 1<<15, 1<<16, 1<<17, 1<<18, 1<<19, 1<<20};
    public:
    char *free_lists[sizeof(_free_list_chunk_size) / sizeof(_free_list_chunk_size[0])];

    small_mem_alloc(){
        for (int i = 0; i < sizeof(free_lists) / sizeof(free_lists[0]); ++i) {
            free_lists[i] = nullptr;
        }
    }

    // adjust memory allocation size to sizes inside _free_list_chunk_size[]
    // Returns -1 if memory allocation size is bigger than 1mb + 16 bytes
    int16_t find_small_mem_size(size_t* sz) {
        for (size_t i = 0; i < sizeof(_free_list_chunk_size) / sizeof(_free_list_chunk_size[0]); ++i) {
            if (*sz <= _free_list_chunk_size[i]) {
                *sz = _free_list_chunk_size[i];
                return i;
            }
        }
        return -1;
    }

    char* allocate_from_free_list(int id, size_t sz){
        if (id >= 0 && free_lists[id]) {
            // link list data stucture to keep track of memories addresses
            char* cur                             = free_lists[id];
            char* next                            = *(char**)cur;
            free_lists[id] = next;
            *(int*)cur                            = id; // store index of free_list
            *(int*)(cur + 4)                      = sz;           // store size of memory
            return cur;
        }
        return NULL;
    }

    void add_to_freelist(char *ptr, int id){
        if (id >= 0 && id < sizeof(free_lists) / sizeof(free_lists[0])) {
            *(char **)ptr = free_lists[id];
            free_lists[id] = ptr;
        }
    }
};