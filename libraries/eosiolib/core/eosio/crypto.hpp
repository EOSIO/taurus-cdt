/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#pragma once

#include <eosio/abieos_crypto.hpp>
#include <eosio/fixed_bytes.hpp>

namespace eosio {

   /**
    *  @defgroup crypto Crypto
    *  @ingroup core
    *  @brief Defines API for calculating and checking hashes
    */

   /**
    *  Tests if the SHA256 hash generated from data matches the provided digest.
    *
    *  @ingroup crypto
    *  @param data - Data you want to hash
    *  @param length - Data length
    *  @param hash - digest to compare to
    *  @note This method is optimized to a NO-OP when in fast evaluation mode.
    */
   void assert_sha256( const char* data, uint32_t length, const eosio::checksum256& hash );

   /**
    *  Tests if the SHA1 hash generated from data matches the provided digest.
    *
    *  @ingroup crypto
    *  @param data - Data you want to hash
    *  @param length - Data length
    *  @param hash - digest to compare to
    *  @note This method is optimized to a NO-OP when in fast evaluation mode.
    */
   void assert_sha1( const char* data, uint32_t length, const eosio::checksum160& hash );

   /**
    *  Tests if the SHA512 hash generated from data matches the provided digest.
    *
    *  @ingroup crypto
    *  @param data - Data you want to hash
    *  @param length - Data length
    *  @param hash - digest to compare to
    *  @note This method is optimized to a NO-OP when in fast evaluation mode.
    */
   void assert_sha512( const char* data, uint32_t length, const eosio::checksum512& hash );

   /**
    *  Tests if the RIPEMD160 hash generated from data matches the provided digest.
    *
    *  @ingroup crypto
    *  @param data - Data you want to hash
    *  @param length - Data length
    *  @param hash - digest to compare to
    */
   void assert_ripemd160( const char* data, uint32_t length, const eosio::checksum160& hash );

   /**
    *  Hashes `data` using SHA256.
    *
    *  @ingroup crypto
    *  @param data - Data you want to hash
    *  @param length - Data length
    *  @return eosio::checksum256 - Computed digest
    */
   eosio::checksum256 sha256( const char* data, uint32_t length );

   /**
    *  Hashes `data` using SHA1.
    *
    *  @ingroup crypto
    *
    *  @param data - Data you want to hash
    *  @param length - Data length
    *  @return eosio::checksum160 - Computed digest
    */
   eosio::checksum160 sha1( const char* data, uint32_t length );

   /**
    *  Hashes `data` using SHA512.
    *
    *  @ingroup crypto
    *  @param data - Data you want to hash
    *  @param length - Data length
    *  @return eosio::checksum512 - Computed digest
    */
   eosio::checksum512 sha512( const char* data, uint32_t length );

   /**
    *  Hashes `data` using RIPEMD160.
    *
    *  @ingroup crypto
    *  @param data - Data you want to hash
    *  @param length - Data length
    *  @return eosio::checksum160 - Computed digest
    */
   eosio::checksum160 ripemd160( const char* data, uint32_t length );

   /**
    *  Calculates the public key used for a given signature on a given digest.
    *
    *  @ingroup crypto
    *  @param digest - Digest of the message that was signed
    *  @param sig - Signature
    *  @return eosio::public_key - Recovered public key
    */
   eosio::public_key recover_key( const eosio::checksum256& digest, const eosio::signature& sig );

   /**
    *  Tests a given public key with the recovered public key from digest and signature.
    *
    *  @ingroup crypto
    *  @param digest - Digest of the message that was signed
    *  @param sig - Signature
    *  @param pubkey - Public key
    */
   void assert_recover_key( const eosio::checksum256& digest, const eosio::signature& sig, const eosio::public_key& pubkey );


   /**
    * @ingroup crypto
    * @param msg - Message
    * @param msglen - Message length
    * @param sig - Signature in hex string format
    * @param sig - Signature length
    * @param exp - Public exponent in hex string format
    * @param explen - Public exponent length
    * @param mod - Modulus in hex string format
    * @param modlen - Modulus length
    * @return bool - If the signature matches the message and RSA public key represented by modulus and exponent, return true; otherwise, return false
    */
   bool verify_rsa_sha256_sig( const char* msg, uint32_t msg_len, const char* sig, uint32_t sig_len, const char* exp, uint32_t exp_len, const char* mod, uint32_t mod_len);

   /**
    * @ingroup crypto
    * @param msg - Message in hex string format
    * @param sig - Signature in hex string format
    * @param exp - Public exponent in hex string format
    * @param mod - Modulus
    * @return bool - If the signature matches the message and RSA public key represented by modulus and exponent, return true; otherwise, return false
    */
   bool verify_rsa_sha256_sig( const std::string& msg, const std::string& sig, const std::string& exp, const std::string& mod );

   /**
    * @ingroup crypto
    * @param msg - Message
    * @param msg_len - Message length
    * @param sig - Signature in Base64 encoding format
    * @param sig_len - Signature length
    * @param pubkey - RSA public key in X.509 SubjectPublicKeyInfo format, PEM encoded
    * @param pubkey_len - Public key length
    * @return bool - If the signature matches the message and RSA public key, return true; otherwise, return false
    */
   bool verify_rsa_sha256_sig( const char* msg, uint32_t msg_len, const char* sig, uint32_t sig_len, const char* pubkey, uint32_t pubkey_len );

   /**
    * @ingroup crypto
    * @param msg - Message
    * @param sig - Signature in Base64 encoding format
    * @param key - RSA public key in X.509 SubjectPublicKeyInfo format, PEM encoded
    * @return bool - If the signature matches the message and public key, return true; otherwise, return false
    */
   bool verify_rsa_sha256_sig( const std::string& msg, const std::string& sig, const std::string& pubkey );

   /**
    * @ingroup crypto
    * @param key - RSA public key PEM string
    * @return bool - If the key string is supported by CDT/taurus-node (currently X.509 SubjectPublicKeyInfo format, PEM encoded), return true; otherwise, return false
    */    
   bool is_supported_rsa_pubkey(const std::string& pubkey);

   bool is_supported_rsa_pubkey(const char* pubkey, uint32_t pubkey_len);

   /**
    * @ingroup crypto
    * @param msg - Message
    * @param msg_len - Message length
    * @param sig - Signature in ASN.1 DER format, base64 encoded
    * @param sig_len - Signature length
    * @param pubkey - ECDSA public key in X.509 SubjectPublicKeyInfo format, PEM encoded
    * @param pubkey_len - Public key length
    * @return bool - If the signature matches the message and ECDSA public key, return true; otherwise, return false
    */
   bool verify_ecdsa_sig( const char* msg, uint32_t msg_len, const char* sig, uint32_t sig_len, const char* pubkey, uint32_t pubkey_len );

   bool verify_ecdsa_sig( const std::string& msg, const std::string& sig, const std::string& pubkey );

   /**
    * @ingroup crypto
    * @param pubkey - ECDSA public key char array pointer
    * @param pubkey_len - ECDSA public key char array length
    * @return bool - If the key string is supported by CDT/taurus-node (currently X.509 SubjectPublicKeyInfo format, PEM encoded), return true; otherwise, return false
    */
   bool is_supported_ecdsa_pubkey( const char* pubkey, uint32_t pubkey_len );

   bool is_supported_ecdsa_pubkey( const std::string& pubkey );


   auto pb_serialize(auto& archive, const public_key& v) {
      return archive(public_key_to_string(v));
   }

   auto pb_serialize(auto& archive, public_key& v) {
      auto data = archive.remaining_data();
      v = public_key_from_string(std::string{ (const char*)data.data(), data.size()});  
      return std::errc{};
   }

   auto pb_serialize(auto& archive, const private_key& v) {
      return archive(private_key_to_string(v));
   }

   auto pb_serialize(auto& archive, private_key& v) {
      auto data = archive.remaining_data();
      v = private_key_from_string(std::string{ (const char*)data.data(), data.size()});  
      return std::errc{};
   }

   auto pb_serialize(auto& archive, const signature& v) {
      return archive(signature_to_string(v));
   }

   auto pb_serialize(auto& archive, signature& v) {
      auto data = archive.remaining_data();
      v = signature_from_string(std::string{ (const char*)data.data(), data.size()});  
      return std::errc{};
   }
}
