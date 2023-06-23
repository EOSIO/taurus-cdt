#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "eosio/small_memory_alloc.hpp"
#include "eosio/big_memory_alloc.hpp"

SCENARIO( "alloc from big chunk") {
    GIVEN( "allocated memory slot" ){
        big_mem_alloc big_memory; 
        char* charPtr = (char*) std::malloc(65536);
        WHEN( "add to big chunk" ) {
            big_memory.add_to_bigchunk(charPtr, 65536);
            THEN( "allocate memory bigger than the available size" ) {
                REQUIRE(big_memory.alloc_from_bigchunk(67000) == nullptr);
            }
            THEN("allocate memory equal to the available size"){
                REQUIRE(big_memory.alloc_from_bigchunk(65536) == charPtr);
                REQUIRE(big_memory.bigchunks[0].ptr == charPtr+65536);
                REQUIRE(big_memory.bigchunks[0].capacity == 0);
            }
        }
    }
}
SCENARIO( "add to big chunk test merge left and right" ) {
    GIVEN( "allocated memory slot" ){
        big_mem_alloc big_memory; 
        char* charPtr_1 = (char*) std::malloc(2048);
        char* charPtr_2 = charPtr_1 + 1024;
        WHEN( "add two back to back memory slot to big chunk" ) {
            big_memory.add_to_bigchunk(charPtr_1, 1024);
            big_memory.add_to_bigchunk(charPtr_2, 1024);
            THEN( "memory slots merge with charPtr_1 as address of memory" ) {
                REQUIRE(big_memory.bigchunks[0].ptr == charPtr_1);
                REQUIRE(big_memory.bigchunks[0].capacity == 2048);
            }
        }
        WHEN( " add the second slot first and then the first slot " ) {
            big_memory.add_to_bigchunk(charPtr_2, 1024);
            big_memory.add_to_bigchunk(charPtr_1, 1024);
            THEN( "memory slots merge with charPtr_1 as address of memory" ) {
                REQUIRE(big_memory.bigchunks[0].ptr == charPtr_1);
                REQUIRE(big_memory.bigchunks[0].capacity == 2048);
            }
        }
    }
}
SCENARIO( "add/allocate from small memory free list" ) {
    GIVEN( "small memory object" ){
        small_mem_alloc small_memory; 
        WHEN( "request memory size larger than 1mb+16bytes from small memory " ){
            size_t sz = 1<<21;
            auto id = small_memory.find_small_mem_size(&sz);
            THEN( " requested memory bigger than small memory sizes" ) {
                REQUIRE(id == -1);
                REQUIRE(small_memory.allocate_from_free_list(id, sz) == nullptr);
            }
        }
        WHEN( " test adjust size with 475895 bytes memory request" ) {
            size_t sz = 475895;
            int id = small_memory.find_small_mem_size(&sz);
            THEN( " size adjusted to fix size bigger/equal to sz from _free_list_chunk_size " ) {
                REQUIRE(id == 13);
                REQUIRE(sz == 1<<19);
            }
        }
        WHEN( " test adjust size with 1048576 bytes memory request" ) {
            size_t sz = 1048576;
            int id = small_memory.find_small_mem_size(&sz);
            THEN( " size adjusted to fix size bigger/equal to sz from _free_list_chunk_size " ) {
                REQUIRE(id == 14);
                REQUIRE(sz == 1<<20);
            }
        }
        WHEN( " test adjust size with 5 bytes memory request" ) {
            size_t sz = 5;
            int id = small_memory.find_small_mem_size(&sz);
            THEN( " size adjusted to fix size bigger/equal to sz from _free_list_chunk_size " ) {
                REQUIRE(id == 0);
                REQUIRE(sz == 1<<6);
            }
        }
        WHEN( " test add and request two memory slots in free list " ) {
            size_t sz = 128;
            int id = small_memory.find_small_mem_size(&sz);
            char* charPtr_1 = (char*) std::malloc(128);
            char* charPtr_2 = (char*) std::malloc(128);
            small_memory.add_to_freelist(charPtr_1, id);
            small_memory.add_to_freelist(charPtr_2, id);
            THEN( " request memory with size and id " ) {
                REQUIRE(small_memory.allocate_from_free_list(id, sz) == charPtr_2);
                REQUIRE(small_memory.allocate_from_free_list(id, sz) == charPtr_1);
                REQUIRE(small_memory.allocate_from_free_list(id, sz) == nullptr);
            }
        }
    }
}