#pragma once
#include <cstddef>
class big_mem_alloc {
    public:
    static constexpr int max_bigchunk_items = 32; // maximum number of freed big chunks to keep track of (33mb is the wasm max memory)
    struct bigchunk_t {
        char     *ptr = nullptr;
        size_t   capacity = 0;
    } bigchunks[max_bigchunk_items];

    char *alloc_from_bigchunk(size_t sz) {
        auto itr = std::find_if(std::begin(bigchunks), std::end(bigchunks), 
                        [sz](auto v) { return v.ptr && v.capacity >= sz; });
        if (itr != std::end(bigchunks)) {
            char *ret = itr->ptr;
            itr->ptr += sz;
            itr->capacity -=sz;
            return ret;
        }
        return nullptr;
    }

    void add_to_bigchunk(char *freed_mem, size_t capacity) {
        int smallest_idx = 0;
        int smallest_cap = bigchunks[0].capacity;
        for (int i = 0; i < max_bigchunk_items; ++i) {
            if (bigchunks[i].ptr) {
                if (bigchunks[i].ptr + bigchunks[i].capacity == freed_mem) { // merge right
                    freed_mem = bigchunks[i].ptr;
                    capacity += bigchunks[i].capacity;
                    bigchunks[i].ptr = nullptr;
                    bigchunks[i].capacity = 0;
                }
                else if (freed_mem + capacity == bigchunks[i].ptr) { // merge left
                    capacity += bigchunks[i].capacity;
                    bigchunks[i].ptr = nullptr;
                    bigchunks[i].capacity = 0;
                }
            }
            if (bigchunks[i].capacity < smallest_cap) { // find the smallest chunk to replace with
                smallest_cap = bigchunks[i].capacity;
                smallest_idx = i;
            }
        }
        if (capacity > smallest_cap) {
            bigchunks[smallest_idx].ptr = freed_mem;
            bigchunks[smallest_idx].capacity = capacity;
        }
    }
};