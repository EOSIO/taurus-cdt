#include <eosio/eosio.hpp>
#include <eosio/system.hpp>

class [[eosio::contract]] push_event_test : public eosio::contract {
public:
   using eosio::contract::contract;

   [[eosio::action]]
   void push( eosio::name tag, std::string route, const std::vector<char>& data )
   {
      eosio::push_event(tag, route, data);
   }

   [[eosio::action]]
   void pushtaurus(std::string route, const std::vector<char>& data )
   {
      eosio::push_event(route, data);
   }
};
