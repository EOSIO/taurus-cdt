#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>

class [[eosio::contract]] ecdsa_verify_test : public eosio::contract {
public:
   using eosio::contract::contract;

   [[eosio::action]]
   void validkey(const std::string& pubkey)
   {
      bool res = eosio::is_supported_ecdsa_pubkey(pubkey.data(), pubkey.size());
      eosio::check(res, "is_supported_ecdsa_pubkey() failed for string pointer and size input");

      res = eosio::is_supported_ecdsa_pubkey(pubkey);
      eosio::check(res, "is_supported_ecdsa_pubkey() failed for string input");
   }

   [[eosio::action]]
   void verify(const std::string& msg, const std::string& sig, const std::string& pubkey)
   {
      bool res = eosio::verify_ecdsa_sig(msg.data(), msg.size(), sig.data(), sig.size(), pubkey.data(), pubkey.size());
      eosio::check(res, "verify_ecdsa_sig() failed for string pointer and size input");

      res = eosio::verify_ecdsa_sig(msg, sig, pubkey);
      eosio::check(res, "verify_ecdsa_sig() failed for string input");
   }
};
