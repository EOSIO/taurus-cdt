#include <errno.h>
#include <memory>
#include <eosio/check.hpp>
#include "eosio/small_memory_alloc.hpp"
#include "eosio/big_memory_alloc.hpp"

#ifndef __wasm__
   extern "C" {
      size_t _current_memory();
      size_t _grow_memory(size_t);
   }
#define CURRENT_MEMORY _current_memory()
#define GROW_MEMORY(X) _grow_memory(X)
#else
#define CURRENT_MEMORY __builtin_wasm_memory_size(0)
#define GROW_MEMORY(X) __builtin_wasm_memory_grow(0, X)
#endif

extern "C" char __heap_base;

namespace {
   int *_eosio_malloc_is_free_enabled() {
      static int enabled = 0;
      return &enabled;
   }

   static constexpr int free_list_meta_size = 16; // overhead of each allocation, at least 8 bytes (4 bytes list index + 4 buffer capacity)
}

namespace eosio {   
   struct dsmalloc {

      small_mem_alloc small_memory;
      big_mem_alloc big_memory;

      inline char* align(char* ptr, uint8_t align_amt) {
         return (char*)((((size_t)ptr) + align_amt-1) & ~(align_amt-1));
      }

      inline size_t align(size_t ptr, uint8_t align_amt) {
         return (ptr + align_amt-1) & ~(align_amt-1);
      }

      dsmalloc() {
         last_ptr = &__heap_base;
         next_page = CURRENT_MEMORY;
      }
      
      void free(char *ptr) {
         if ((int)ptr >= free_list_meta_size) {
            ptr -= free_list_meta_size;
            int id = *(int *)ptr;
            if (id >= 0 && id < sizeof(small_memory.free_lists) / sizeof(small_memory.free_lists[0])) {
               small_memory.add_to_freelist(ptr, id);
            } else {
               int sz = *(int *)(ptr + 4); // in (ptr + 4) size of the memory is stored.
               big_memory.add_to_bigchunk(ptr, sz);
            }
         }
      }
      
      char* operator()(size_t sz, uint8_t align_amt=16) {
         if (sz == 0)
            return NULL;

         sz += free_list_meta_size;
         sz = align(sz, align_amt);
         // adjust requested memory size to sizes in _free_list_chunk_size
         int16_t free_list_id = small_memory.find_small_mem_size(&sz);

         if(free_list_id >= 0 && small_memory.free_lists[free_list_id])
            return small_memory.allocate_from_free_list(free_list_id, sz) + free_list_meta_size;

         char *old_last_ptr = last_ptr;
         size_t old_next_page = next_page;

         char* ret = last_ptr;
         last_ptr = align(last_ptr+sz, align_amt);

         size_t pages_to_alloc = sz >> 16;
         next_page += pages_to_alloc;
         if ((next_page << 16) <= (size_t)last_ptr) {
            next_page++;
            pages_to_alloc++;
         }
         // GROW_MEMORY resize wasm linear memory in unit of WebAssembly page (64kb)
         // throws -1 if it passes the max memory of 33mb
         if (GROW_MEMORY(pages_to_alloc) == -1) {
            last_ptr = old_last_ptr;
            next_page = old_next_page;
            // check to reuse freed momory stored in bigchunk array.
            ret = big_memory.alloc_from_bigchunk(sz);
            eosio::check(ret != nullptr,  "failed to allocate pages");  
         }

         *(int *)ret       = free_list_id;
         *(int *)(ret + 4) = sz;
         return ret + free_list_meta_size;
      }

      char*  last_ptr;
      size_t next_page;
   };
   dsmalloc _dsmalloc __attribute__((init_priority(101)));
} // ns eosio

extern "C" {
   // use eosio_malloc_enable_free() to enable free for big migrations
   void eosio_malloc_enable_free() {
      *_eosio_malloc_is_free_enabled() = 1;
   }

   void eosio_malloc_disable_free() {
      *_eosio_malloc_is_free_enabled() = 0;
   }

   void* malloc(size_t size) {
      void* ret = eosio::_dsmalloc(size);
      return ret;
   }


   int posix_memalign(void** memptr, size_t alignment, size_t size) {
      if (alignment < sizeof(void*) || (alignment & (alignment - size_t(1))) != 0)
         return EINVAL;
      *memptr = eosio::_dsmalloc(size, alignment > 16 ? alignment : 16);
      return 0;
   }

   void* memset(void*,int,size_t);
   void* calloc(size_t count, size_t size) {
      if (void* ptr = eosio::_dsmalloc(count*size)) {
         memset(ptr, 0, count*size);
         return ptr;
      }
      return nullptr;
   }

   void* realloc(void* ptr_, size_t size) {
      char *ptr = (char *)ptr_;
      if ((int)ptr >= free_list_meta_size) {
         ptr -= free_list_meta_size;
         int sz = *(int *)(ptr + 4);
         if (sz - free_list_meta_size >= size) return ptr_;
      }
      if (void* result = eosio::_dsmalloc(size)) {
         // May read out of bounds, but that's okay, as the
         // contents of the memory are undefined anyway.
         memmove(result, ptr_, size);
         free(ptr_);
         return result;
      }
      return nullptr;
   }

   void free(void* ptr) {
      if (*_eosio_malloc_is_free_enabled()) {
         eosio::_dsmalloc.free((char *)ptr);
      }
   }
}