#include <eosio/eosio.hpp>
#include <test/test.pb.hpp>

namespace test {

class [[eosio::contract]] pb_msg_test : public eosio::contract {
 public:
   using eosio::contract::contract;

   [[eosio::action]]  eosio::pb<ActResult> hi(const eosio::pb<ActData>& msg) {
      eosio::check(msg.id == 1, "validate msg.id");
      eosio::check(msg.type == 2, "validate msg.type");
      eosio::check(msg.note == "abc", "validate msg.note");
      eosio::check(msg.account == 4, "validate msg.accout");
      eosio::check(msg.optional_string == "def" , "validate msg.optional_string");
      eosio::check(msg.optional_bytes == std::vector<char>{'b', 'y', 't', 'e', 's'}, "validate msg.optional_bytes");
      return ActResult{1, "res_str", std::vector<char>{'r', 'e', 's', '_', 'b', 'y', 't', 'e', 's'}};
   }

   [[eosio::action]]  eosio::pb<ActResult> hi2(const eosio::pb<ActData>& msg) {
      eosio::check(msg.id == 1, "validate msg.id");
      eosio::check(msg.type == 2, "validate msg.type");
      eosio::check(msg.note == "abc", "validate msg.note");
      eosio::check(msg.account == 4, "validate msg.accout");
      return {};
   }

};
} // namespace test

