#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>

class [[eosio::contract]] table_test : public eosio::contract {
public:
   using contract::contract;

   struct my_struct {
      eosio::name primary_key;
      std::string foo;
      uint64_t bar;

      std::string fullname;
      uint32_t age;

      std::pair<int, int> a;
      std::optional<float> b;
      std::list<double> c;
      std::vector<int> d;
      std::variant<int, bool, float> e;
      std::vector<char> f;
      std::vector<std::pair<uint64_t, uint32_t>> g;
      std::vector<std::optional<float>> h;
      eosio::checksum256 i;
      eosio::ecc_public_key j;
      std::variant<eosio::checksum160, eosio::checksum256, eosio::checksum512> k;
      eosio::public_key l;
      eosio::private_key m;
      eosio::signature n;
      eosio::block_timestamp o;

      std::tuple<std::string, uint32_t> non_unique_name;

   };

   struct [[eosio::table]] my_table : eosio::kv::table<my_struct, "testtable"_n> {
      KV_NAMED_INDEX("primarykey"_n, primary_key)
      KV_NAMED_INDEX("foo"_n, foo)
      index<uint64_t> bar{eosio::name{"bar"_n}, &value_type::bar};
      KV_NAMED_INDEX("nonuniqnme"_n, non_unique_name)
      KV_NAMED_INDEX("age"_n, age)
      KV_NAMED_INDEX("pubk"_n, l)
      KV_NAMED_INDEX("privk"_n, m)
      KV_NAMED_INDEX("signature"_n, n)
      KV_NAMED_INDEX("checksum"_n, i)

      my_table(eosio::name contract_name) {
         init(contract_name, primary_key, foo, bar, non_unique_name, age);
      }
   };

   [[eosio::action]]
   void noop() {}

};
