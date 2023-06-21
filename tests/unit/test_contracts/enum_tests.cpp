#include <eosio/eosio.hpp>

using namespace eosio;

class [[eosio::contract]] enum_tests : public contract {
   public:
      using contract::contract;

   enum class enumchar : char {
      e0 = -5,
      e1 = -3,
      e2 = 1,
      e3 = 8
   };
   enum class enumint : int {
      e0 = -5,
      e1 = -3,
      e2 = 1,
      e3 = 8
   };

   [[eosio::action]] enumchar techar(enumchar & var)
   {
      enumchar ev = var;
      return ev;
   }

   [[eosio::action]] enumint teint(enumint & var)
   {
      enumint ev = var;
      return ev;
   }
};
