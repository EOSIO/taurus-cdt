#include <catch2/catch.hpp>
#include <eosio/tester.hpp>
#include <test.pb.hpp>
#include <contracts.hpp>

using namespace eosio;
using eosio::testing::contracts;

TEST_CASE_METHOD( test_chain, "protobuf tests", "[proto]" ) {

   eosio::print("start protobuf tests");

   set_code( "eosio"_n, contracts::boot_wasm() );
   transact({action({"eosio"_n, "active"_n}, "eosio"_n, "activate"_n,
                           make_checksum256("c3a6138c5061cf291310887c0b5c71fcaffeab90d5deb50d3b9e687cead45071"))});
   finish_block();
   create_code_account( "test"_n );
   finish_block();
   set_code( "test"_n, contracts::pb_msg_test_wasm());
   finish_block();

   SECTION("hi action", "All optional action data and results have values") {
      eosio::pb<test::ActData> data = test::ActData{ 1, 2, "abc", 4, "def", std::vector<char>{'b', 'y', 't', 'e', 's'}};

      auto trace = transact({action({"test"_n, "active"_n}, "test"_n, "hi"_n, data)});

      REQUIRE(trace.action_traces.size() == 1);
      CHECK(trace.action_traces[0].return_value == std::vector<char>{ 0x16, 0x08, 0x01, 0x12, 0x07, 0x72, 0x65, 0x73,
                                                                      0x5F, 0x73, 0x74, 0x72, 0x1A, 0x09, 0x72, 0x65,
                                                                      0x73, 0x5F, 0x62, 0x79, 0x74, 0x65, 0x73 });

      test::ActResult res;

      zpp::bits::in in{ trace.action_traces[0].return_value, zpp::bits::size_varint{}, zpp::bits::protobuf{} };
      REQUIRE(in(res) == std::errc{});
      CHECK(res.value == 1);
      CHECK(res.opt_str_value == "res_str");
      CHECK(res.opt_bytes_value == std::vector<char>{'r', 'e', 's', '_', 'b', 'y', 't', 'e', 's'});
      finish_block();
   }

   SECTION("hi2 action", "No optional action data or results have values") {
      eosio::pb<test::ActData> data = test::ActData{ 1, 2, "abc", 4 };
      auto trace = transact({action({"test"_n, "active"_n}, "test"_n, "hi2"_n, data)});

      REQUIRE(trace.action_traces.size() == 1);
      CHECK(trace.action_traces[0].return_value == std::vector<char>{ 0x00 });

      test::ActResult res;

      zpp::bits::in in{ trace.action_traces[0].return_value, zpp::bits::size_varint{}, zpp::bits::protobuf{} };
      REQUIRE(in(res) == std::errc{});
      CHECK(!res.value.has_value());
      CHECK(!res.opt_str_value.has_value());
      CHECK(!res.opt_bytes_value.has_value());
      finish_block();
   }
}
