#include <eosio/eosio.hpp>

struct my_struct_v {
   uint64_t age;
   std::string full_name;
};

struct my_struct_v2 {
   std::string first_name;
   std::string last_name;
   uint64_t age;
};

struct my_table : eosio::kv::table<my_struct_v, "testtable"_n> {
   KV_NAMED_INDEX("fullname"_n, full_name);
   KV_NAMED_INDEX("age"_n, age);

   my_table(eosio::name contract_name) {
      init(contract_name, full_name, age);
   }
};

struct my_table_v : eosio::kv::table<std::variant<my_struct_v, my_struct_v2>, "testtable2"_n> {
   index<std::string> primary_key{"fullname"_n, [](const auto& obj) {
      return std::visit([&](auto&& a) {
         using V = std::decay_t<decltype(a)>;
         if constexpr(std::is_same_v<V, my_struct_v>) {
            return a.full_name;
         } else if constexpr(std::is_same_v<V, my_struct_v2>) {
            return a.first_name + " : " + a.last_name;
         } else {
            eosio::check(false, "BAD TYPE");
            return "";
         }
      }, *obj);
   }};
   index<uint64_t> age{"age"_n, [](const auto& obj) {
      return std::visit([&](auto&& a) {
         return a.age;
      }, *obj);
   }};

   my_table_v(eosio::name contract_name) {
      init(contract_name, primary_key, age);
   }
};

class [[eosio::contract]] kv_variant_tests : public eosio::contract {
public:
   using contract::contract;

   // Empty action to avoid having conditional logic in the integration tests file.
   [[eosio::action]]
   void setup() {}

   [[eosio::action]]
   void vriantupgrd() {
      my_table t{"kvtest"_n};

      my_struct_v s1{
         .age = 25,
         .full_name = "Dan Larimer"
      };

      my_struct_v s2{
         .age = 24,
         .full_name = "Brendan Blumer"
      };

      t.put(s1);
      t.put(s2);
   }

   [[eosio::action]]
   void vriant() {
      my_table_v t{"kvtest"_n};

      my_struct_v s1{
         .age = 25,
         .full_name = "Dan Larimer"
      };

      my_struct_v s2{
         .age = 24,
         .full_name = "Brendan Blumer"
      };

      my_struct_v2 s3{
         .first_name = "Bob",
         .last_name = "Smith",
         .age = 30
      };

      t.put(s1);
      t.put(s2);
      t.put(s3);

      auto itr = t.primary_key.find("Dan Larimer");
      auto val = itr.value();
      auto vval = std::get<my_struct_v>(val);
      eosio::check(vval.age == 25, "wrong value");

      auto val2 = t.primary_key.get("Brendan Blumer");
      auto vval2 = std::get<my_struct_v>(*val2);
      eosio::check(vval2.age == 24, "wrong value");

      auto val3 = t.primary_key.get("Bob : Smith");
      auto vval3 = std::get<my_struct_v2>(*val3);
      eosio::check(vval3.age == 30, "wrong value");
   }
};
