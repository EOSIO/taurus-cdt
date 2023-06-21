/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#include <eosio/crypto.hpp>
#include <eosio/datastream.hpp>

#include <eosio/crypto_utils.hpp>

extern "C" {
   struct __attribute__((aligned (16))) capi_checksum160 { uint8_t hash[20]; };
   struct __attribute__((aligned (16))) capi_checksum256 { uint8_t hash[32]; };
   struct __attribute__((aligned (16))) capi_checksum512 { uint8_t hash[64]; };
   __attribute__((import_name("assert_sha256"))) void assert_sha256( const char* data, uint32_t length, const capi_checksum256* hash );

   __attribute__((import_name("assert_sha1"))) void assert_sha1( const char* data, uint32_t length, const capi_checksum160* hash );

   __attribute__((import_name("assert_sha512"))) void assert_sha512( const char* data, uint32_t length, const capi_checksum512* hash );

   __attribute__((import_name("assert_ripemd160"))) void assert_ripemd160( const char* data, uint32_t length, const capi_checksum160* hash );

   __attribute__((import_name("sha256"))) void sha256( const char* data, uint32_t length, capi_checksum256* hash );

   __attribute__((import_name("sha1"))) void sha1( const char* data, uint32_t length, capi_checksum160* hash );

   __attribute__((import_name("sha512"))) void sha512( const char* data, uint32_t length, capi_checksum512* hash );

   __attribute__((import_name("ripemd160"))) void ripemd160( const char* data, uint32_t length, capi_checksum160* hash );

   __attribute__((import_name("recover_key"))) int recover_key( const capi_checksum256* digest, const char* sig,
                    uint32_t siglen, char* pub, uint32_t publen );

   __attribute__((import_name("assert_recover_key"))) void assert_recover_key( const capi_checksum256* digest, const char* sig,
                    uint32_t siglen, const char* pub, uint32_t publen );

   __attribute__((import_name("verify_rsa_sha256_sig"))) bool verify_rsa_sha256_sig( const char* msg, uint32_t msglen,
                    const char* sig, uint32_t siglen, const char* exp, uint32_t explen, const char* mod, uint32_t modlen );

   __attribute__((import_name("verify_ecdsa_sig"))) bool verify_ecdsa_sig( const char* msg, uint32_t msg_len, 
                    const char* sig, uint32_t sig_len, const char* pubkey, uint32_t pubkey_len );
   __attribute__((import_name("is_supported_ecdsa_pubkey"))) bool is_supported_ecdsa_pubkey( const char* pubkey, uint32_t pubkey_len );
}

namespace eosio {

   void assert_sha256( const char* data, uint32_t length, const eosio::checksum256& hash ) {
      auto hash_data = hash.extract_as_byte_array();
      ::assert_sha256( data, length, reinterpret_cast<const ::capi_checksum256*>(hash_data.data()) );
   }

   void assert_sha1( const char* data, uint32_t length, const eosio::checksum160& hash ) {
      auto hash_data = hash.extract_as_byte_array();
      ::assert_sha1( data, length, reinterpret_cast<const ::capi_checksum160*>(hash_data.data()) );
   }

   void assert_sha512( const char* data, uint32_t length, const eosio::checksum512& hash ) {
      auto hash_data = hash.extract_as_byte_array();
      ::assert_sha512( data, length, reinterpret_cast<const ::capi_checksum512*>(hash_data.data()) );
   }

   void assert_ripemd160( const char* data, uint32_t length, const eosio::checksum160& hash ) {
      auto hash_data = hash.extract_as_byte_array();
      ::assert_ripemd160( data, length, reinterpret_cast<const ::capi_checksum160*>(hash_data.data()) );
   }

   eosio::checksum256 sha256( const char* data, uint32_t length ) {
      ::capi_checksum256 hash;
      ::sha256( data, length, &hash );
      return {hash.hash};
   }

   eosio::checksum160 sha1( const char* data, uint32_t length ) {
      ::capi_checksum160 hash;
      ::sha1( data, length, &hash );
      return {hash.hash};
   }

   eosio::checksum512 sha512( const char* data, uint32_t length ) {
      ::capi_checksum512 hash;
      ::sha512( data, length, &hash );
      return {hash.hash};
   }

   eosio::checksum160 ripemd160( const char* data, uint32_t length ) {
      ::capi_checksum160 hash;
      ::ripemd160( data, length, &hash );
      return {hash.hash};
   }

   eosio::public_key recover_key( const eosio::checksum256& digest, const eosio::signature& sig ) {
      auto digest_data = digest.extract_as_byte_array();

      auto sig_data = eosio::pack(sig);

      char optimistic_pubkey_data[256];
      uint32_t pubkey_size = ::recover_key( reinterpret_cast<const capi_checksum256*>(digest_data.data()),
                                          sig_data.data(), sig_data.size(),
                                          optimistic_pubkey_data, sizeof(optimistic_pubkey_data) );

      eosio::public_key pubkey;
      if ( pubkey_size <= sizeof(optimistic_pubkey_data) ) {
         eosio::datastream<const char*> pubkey_ds( optimistic_pubkey_data, pubkey_size );
         pubkey_ds >> pubkey;
      } else {
         constexpr static uint32_t max_stack_buffer_size = 512;

         auto free_memory = [pubkey_size](char* buf) { if (max_stack_buffer_size < pubkey_size) free(buf);};
         std::unique_ptr<char, decltype(free_memory)> pubkey_data( (char*)(max_stack_buffer_size < pubkey_size ? malloc(pubkey_size) : alloca(pubkey_size)), free_memory);

         ::recover_key( reinterpret_cast<const capi_checksum256*>(digest_data.data()),
                        sig_data.data(), sig_data.size(),
                        pubkey_data.get(), pubkey_size );
         eosio::datastream<const char*> pubkey_ds( pubkey_data.get(), pubkey_size );
         pubkey_ds >> pubkey;
      }
      return pubkey;
   }

   void assert_recover_key( const eosio::checksum256& digest, const eosio::signature& sig, const eosio::public_key& pubkey ) {
      auto digest_data = digest.extract_as_byte_array();

      auto sig_data = eosio::pack(sig);
      auto pubkey_data = eosio::pack(pubkey);

      ::assert_recover_key( reinterpret_cast<const capi_checksum256*>(digest_data.data()),
                            sig_data.data(), sig_data.size(),
                            pubkey_data.data(), pubkey_data.size() );
   }

   bool verify_rsa_sha256_sig( const char* msg, uint32_t msg_len, const char* sig, uint32_t sig_len, const char* exp, uint32_t exp_len, const char* mod, uint32_t mod_len) {
      return ::verify_rsa_sha256_sig( msg, msg_len, 
                                      sig, sig_len, 
                                      exp, exp_len, 
                                      mod, mod_len );
   }

   bool verify_rsa_sha256_sig( const std::string& msg, const std::string& sig, const std::string& exp, const std::string& mod ) {
      return verify_rsa_sha256_sig( msg.data(), msg.size(),
                                    sig.data(), sig.size(),
                                    exp.data(), exp.size(),
                                    mod.data(), mod.size() );
   }

   bool verify_rsa_sha256_sig( const std::string& msg, const std::string& sig, const std::string& pubkey ) {
      std::string sig_hex = base64_to_hex(strip_newline(sig));
      if (sig_hex.empty()) {
         return false;
      }

      std::string mod, exp;
      if (parse_rsa_pubkey(pubkey, mod, exp) != 0) {
         return false;
      }

      return verify_rsa_sha256_sig( msg.data(), msg.size(),
                                    sig_hex.data(), sig_hex.size(),
                                    exp.data(), exp.size(),
                                    mod.data(), mod.size() );
   }

   bool verify_rsa_sha256_sig( const char* msg, uint32_t msg_len, const char* sig, uint32_t sig_len, const char* pubkey, uint32_t pubkey_len ) {
      return verify_rsa_sha256_sig( std::string(msg, msg_len), std::string(sig, sig_len), std::string(pubkey, pubkey_len) );
   }

   bool is_supported_rsa_pubkey( const std::string& pub_key ) {
      std::string mod, exp;
      return parse_rsa_pubkey(pub_key, mod, exp) == 0;
   }

   bool is_supported_rsa_pubkey( const char* pub_key, uint32_t pubkey_len ) {
      return is_supported_rsa_pubkey( std::string(pub_key, pubkey_len) );
   }

   bool verify_ecdsa_sig( const char* msg, uint32_t msg_len, const char* sig, uint32_t sig_len, const char* pubkey, uint32_t pubkey_len ) {
      return ::verify_ecdsa_sig( msg, msg_len, sig, sig_len, pubkey, pubkey_len);
   }

   bool verify_ecdsa_sig( const std::string& msg, const std::string& sig, const std::string& pubkey ) {
      return ::verify_ecdsa_sig( msg.data(), msg.size(), 
                                 sig.data(), sig.size(), 
                                 pubkey.data(), pubkey.size() );
   }

   bool is_supported_ecdsa_pubkey( const char* pubkey, uint32_t pubkey_len ) {
      return ::is_supported_ecdsa_pubkey( pubkey, pubkey_len);
   }

   bool is_supported_ecdsa_pubkey( const std::string& pubkey ) {
      return ::is_supported_ecdsa_pubkey( pubkey.data(), pubkey.size());
   }
}
