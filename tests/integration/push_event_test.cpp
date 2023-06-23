#include <catch2/catch.hpp>
#include <eosio/tester.hpp>
#include <eosio/datastream.hpp>
#include <contracts.hpp>

using namespace std::literals;

TEST_CASE("push_event tests", "[push_event]" ) {
   eosio::test_chain tester;

   // activate push_event protocol feature
   tester.set_code( "eosio"_n, eosio::testing::contracts::boot_wasm() );
   tester.transact({eosio::action({"eosio"_n, "active"_n}, "eosio"_n, "activate"_n,
                           tester.make_checksum256("02d887562f046c4e35acec4f64006b2a756f5573db3bdee4cd1c1a1d76a1356d"))});
   tester.finish_block();

   // deploy test contract
   tester.create_code_account( "test"_n );
   tester.finish_block();
   tester.set_code( "test"_n, eosio::testing::contracts::push_event_test_wasm());
   tester.finish_block();

   // setup event queue
   tester.setup_event_queue();
   
   // push an event
   auto data = std::vector<char>{'d', 'a', 't', 'a'};
   tester.transact({eosio::action({"test"_n, "active"_n}, "test"_n, "push"_n, std::tuple("testevent"_n, "event_route"s, data))});
   tester.finish_block();

   // verify the pushed event
   REQUIRE( tester.pull_events() == std::vector<eosio::event_wrapper>{{ "testevent"_n, "event_route", data }} );

   // push an event with default fixed tag ("taurus")
   tester.transact({eosio::action({"test"_n, "active"_n}, "test"_n, "pushtaurus"_n, std::tuple("event_route"s, data))});
   tester.finish_block();

   // verify the pushed event with default fixed tag
   REQUIRE( tester.pull_events() == std::vector<eosio::event_wrapper>{{ "taurus"_n, "event_route", data }} );
}
