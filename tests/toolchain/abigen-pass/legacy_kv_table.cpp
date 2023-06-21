#include <eosio/eosio.hpp>

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

   std::tuple<std::string, uint32_t> non_unique_name;

   bool operator==(const my_struct& b) const {
      return primary_key == b.primary_key &&
               foo == b.foo &&
               bar == b.bar &&
               fullname == b.fullname &&
               age == b.age;
   }

   uint32_t by_age() const { return age; }
   auto by_non_unique_name() const { return non_unique_name; }
};

struct my_table : eosio::kv::table<my_struct,  "testtable"_n> {
   index<eosio::name> primary_key {
      "primarykey"_n,
      &my_struct::primary_key };

   index<std::string> foo {
      "foo"_n,
      &my_struct::foo };

   index<uint64_t> bar{"bar"_n, &value_type::bar};


   KV_NAMED_INDEX("nonunique"_n, by_non_unique_name)
   KV_NAMED_INDEX("age"_n, by_age)

   my_table(eosio::name contract_name) {
      init(contract_name, primary_key, foo, bar, by_non_unique_name, by_age);
   }
};

struct my_struct1 {
   uint64_t primary_key;
};

struct [[deprecated]] my_table1 : eosio::kv::table<my_struct1,  "testtable"_n> {
   KV_NAMED_INDEX("primarykey"_n, primary_key)
   
   my_table1(eosio::name contract_name) {
      init(contract_name, primary_key);
   }
};

struct my_struct2 {
   uint32_t primary_key;
};

struct [[eosio::contract("xxx")]] my_table2 : eosio::kv::table<my_struct2,  "testtable"_n> {
   KV_NAMED_INDEX("primarykey"_n, primary_key)
   
   my_table2(eosio::name contract_name) {
      init(contract_name, primary_key);
   }
};

class [[eosio::contract]] kv_table_key_type : public eosio::contract {
public:
   using contract::contract;

   [[eosio::action]]
   void noop() {}

};
