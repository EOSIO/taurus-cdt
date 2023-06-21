
// #include <eosio/contract.hpp>
// #include <eosio/key_value.hpp>
// #include <eosio/name.hpp>
#include <eosio/eosio.hpp>
#include <eosio/print.hpp>

namespace eosio {
template <typename T, eosio::name::raw SingletonName>
class kv_caching_singleton {
 public:
   /**
    * Initialize this instance with the name of the contract that owns the singleton.
    */
   explicit kv_caching_singleton(eosio::name contract)
       : contract_name{contract}
       , key(make_key(std::make_tuple(uint64_t(0x02), SingletonName))) {}

 private:
   const eosio::name     contract_name;
   const eosio::key_type key;
};
} // namespace eosio

struct globals {
   uint64_t next_address_uid = 0;
};

struct workflow_globals {
   uint64_t next_transaction_id = 0;
   uint64_t next_workflow_id    = 0;
};

struct state {
   uint64_t next_token{1};
};

class [[eosio::contract]] test_contract : public eosio::contract {
 public:
   using eosio::contract::contract;

   [[eosio::action]] void hi(eosio::name user) {
      require_auth(user);
      print("Hello, ", user);
      auto s = eosio::kv_caching_singleton<state, "custstate"_n>{"custody"_n};
   }

   eosio::kv_caching_singleton<workflow_globals, "wfglobals"_n> workflow_gs{"test"_n};
};

using globals_singleton = eosio::kv_caching_singleton<globals, "globals"_n>;

