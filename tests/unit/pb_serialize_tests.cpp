
#include "legacy_tester.hpp"
#include <eosio/crypto.hpp>
#include <eosio/tester.hpp>
#include <eosio/datastream.hpp>

struct NewCustomerInput {
   eosio::public_key device_key;
   bool              biometric_credential;
   uint64_t          registration_nonce;
   std::vector<char> pin_commitment;
   uint32_t          customer_t;
};

EOSIO_TEST_BEGIN(pb_serialize_test)

   NewCustomerInput input{
      .device_key = eosio::public_key_from_string("EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"),
      .biometric_credential = 0,
      .registration_nonce = 1234,
      .customer_t = 56
   };

   static_assert( zpp::bits::concepts::has_pb_serialize<eosio::public_key>, "" );

   auto data = eosio::pack(eosio::to_pb(input));
   auto ret = eosio::unpack<eosio::pb<NewCustomerInput>>(data);

   CHECK_EQUAL( input.device_key, ret.device_key );
   CHECK_EQUAL( input.biometric_credential, ret.biometric_credential );
   CHECK_EQUAL( input.registration_nonce, ret.registration_nonce );
   CHECK_EQUAL( input.pin_commitment, ret.pin_commitment );
   CHECK_EQUAL( input.customer_t, ret.customer_t );
EOSIO_TEST_END

int main(int argc, char* argv[]) {
   bool verbose = false;
   if (argc >= 2 && std::strcmp(argv[1], "-v") == 0) {
      verbose = true;
   }
   silence_output(!verbose);

   EOSIO_TEST(pb_serialize_test);
   return has_failed();
}
