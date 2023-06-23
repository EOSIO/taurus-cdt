#include <eosio/eosio.hpp>
#include <eosio/system.hpp>

using namespace eosio;
class [[eosio::contract]] malloc_free : public contract {
    public:
        using contract::contract;

        [[eosio::action]]
        void memsmall() {
            int numitr = 20000;
            int memsize = 100000;
            eosio::malloc_enable_free();
            for (size_t i = 0; i < numitr; ++i) {
               char* ptr = (char*)malloc(memsize);
               eosio::check(ptr, "malloc failed");
               free(ptr);
            }
        }
        [[eosio::action]]
        void memlarge() {
            int numitr = 20000;
            int memsize = 2000000;
            eosio::malloc_enable_free();
            for(size_t i = 0; i < numitr; ++i) {
                char* ptr = (char*)malloc(memsize);
                eosio::check(ptr, "malloc failed");
                free(ptr);
            }
        }
};