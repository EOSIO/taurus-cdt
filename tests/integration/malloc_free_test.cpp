#include <catch2/catch.hpp>
#include <eosio/tester.hpp>

#include <contracts.hpp>

using namespace eosio;
using eosio::testing::contracts;
using std::tuple;

TEST_CASE_METHOD( test_chain, "Tests malloc", "[malloc]" ) {
   create_code_account( "test"_n );
   finish_block();
   set_code( "test"_n, contracts::malloc_free_wasm() );
   finish_block();

   transact({{{"test"_n, "active"_n}, "test"_n, "memsmall"_n, tuple()}});
   transact({{{"test"_n, "active"_n}, "test"_n, "memlarge"_n, tuple()}});
}