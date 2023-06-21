#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>

class [[eosio::contract]] rsa_verify_test : public eosio::contract {
public:
   using eosio::contract::contract;

   [[eosio::action]]
   void verify(const std::string& msg, const std::string& sig, const std::string& exp, const std::string& mod)
   {
      bool res = eosio::verify_rsa_sha256_sig(msg, sig, exp, mod);
      eosio::check(res, "verify_rsa_sha256_sig() failed for std::string input");

      res = eosio::verify_rsa_sha256_sig(msg.data(), msg.size(), sig.data(), sig.size(), exp.data(), exp.size(), mod.data(), mod.size());
      eosio::check(res, "verify_rsa_sha256_sig() failed for char array input");
   }

   [[eosio::action]]
   void verify2(const std::string& msg, const std::string& sig, const std::string& pubkey)
   {
      bool res = eosio::verify_rsa_sha256_sig(msg, sig, pubkey);
      eosio::check(res, "verify_rsa_sha256_sig() failed for std::string input");

      res = eosio::verify_rsa_sha256_sig(msg.data(), msg.size(), sig.data(), sig.size(), pubkey.data(), pubkey.size());
      eosio::check(res, "verify_rsa_sha256_sig() failed for char array input");
   }

   [[eosio::action]]
   void acceptkey(const std::string& pubkey)
   {
      bool res = eosio::is_supported_rsa_pubkey(pubkey);
      eosio::check(res, "is_supported_rsa_pubkey() failed for std::string input");

      res = eosio::is_supported_rsa_pubkey(pubkey.data(), pubkey.size());
      eosio::check(res, "is_supported_rsa_pubkey() failed for char array input");
   }
};
