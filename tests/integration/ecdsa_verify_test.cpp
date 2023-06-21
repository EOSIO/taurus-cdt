#include <catch2/catch.hpp>
#include <eosio/tester.hpp>
#include <eosio/datastream.hpp>
#include <contracts.hpp>

using namespace std::literals;
using namespace eosio;

using std::string;

void ecdsa_test_setup(test_chain& tester) {
   // activate verify_ecdsa_sig protocol feature
   tester.set_code( "eosio"_n, eosio::testing::contracts::boot_wasm() );
   tester.transact( {eosio::action({"eosio"_n, "active"_n}, "eosio"_n, "activate"_n,
                    tester.make_checksum256("fe3fb515e05e40f47d7a2058836200dd4b478241bdcb36bf175f9a40a056b5e3"))} );
   tester.finish_block();

   // deploy test contract
   tester.create_code_account( "test"_n );
   tester.finish_block();
   tester.set_code( "test"_n, eosio::testing::contracts::ecdsa_verify_test_wasm() );
   tester.finish_block();
}

void test_happy_path_validkey(test_chain& tester, const string& pubkey) {
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "validkey"_n, std::tuple(pubkey))} );
   tester.finish_block();
}

void test_happy_path_verify(test_chain& tester, const string& message, const string& signature, const string& pubkey) {
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify"_n, std::tuple(message, signature, pubkey))} );
   tester.finish_block();
}

void test_incorrect_pubkey(test_chain& tester, const string& pubkey) {
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "validkey"_n, std::tuple(pubkey))},
                    "is_supported_ecdsa_pubkey() failed" );
   tester.finish_block();
}

void test_incorrect_signature(test_chain& tester, const string& message, const string& corrupt_signature, const string& pubkey) {
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify"_n, std::tuple(message, corrupt_signature, pubkey))},
                    "verify_ecdsa_sig() failed" );
   tester.finish_block();
}

TEST_CASE("ECDSA supported public key tests", "[ecdsa_supported_pubkey]" ) {
   eosio::test_chain tester;
   ecdsa_test_setup(tester);

   const string pubkey = 
      "-----BEGIN PUBLIC KEY-----\n"
      "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEzjca5ANoUF+XT+4gIZj2/X3V2UuT\n"
      "E9MTw3sQVcJzjyC/p7KeaXommTC/7n501p4Gd1TiTiH+YM6fw/YYJUPSPg==\n"
      "-----END PUBLIC KEY-----"s;

   test_happy_path_validkey(tester, pubkey);
}

TEST_CASE("ECDSA invalid public key tests", "[ecdsa_invalid_pubkey]" ) {
   eosio::test_chain tester;
   ecdsa_test_setup(tester);

   const std::string pubkey_k1 = 
      "-----BEGIN PUBLIC KEY-----\n"
      "MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEbmhQwC/AWLtHKs7S7mecwMw2z0Q7EKll\n"
      "5wASKTK4NI1BNFcud0kdSKyck5RSxjhbriEyy0jdS17iUbaBolbtiA==\n"
      "-----END PUBLIC KEY-----";
   test_incorrect_pubkey(tester, pubkey_k1);
   
   const std::string pubkey_no_header = 
      // "-----BEGIN PUBLIC KEY-----\n"
      "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE8+Wkyp8/imL8XQPOPY8kAzpl4fjI\n"
      "G3GG4plUH4r9c4de0b2Aevh3dEvRyy3Ap4wsW+v7aV9aETVz8KjcVAYaMw==\n"
      "-----END PUBLIC KEY-----";
   test_incorrect_pubkey(tester, pubkey_no_header);

   const std::string pubkey_no_trailer = 
      "-----BEGIN PUBLIC KEY-----\n"
      "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE8+Wkyp8/imL8XQPOPY8kAzpl4fjI\n"
      "G3GG4plUH4r9c4de0b2Aevh3dEvRyy3Ap4wsW+v7aV9aETVz8KjcVAYaMw==\n";
      // "-----END PUBLIC KEY-----";
   test_incorrect_pubkey(tester, pubkey_no_trailer);

   std::string pubkey_wrong_format = 
      "-----BEGIN PUBLIC KEY-----\n"
      "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE8+Wkyp8/imL8XQPOPY8kAzpl4fjI\n"
      "G3GG4plUH4r9c4de0b2Aevh3dEvRyy3Ap4wsW+v7aV9aETVz8KjcVAYaMw=f\n" // replace ending '=' with 'f'
      "-----END PUBLIC KEY-----";
   test_incorrect_pubkey(tester, pubkey_wrong_format);

   pubkey_wrong_format = 
      "-----BEGIN PUBLIC KEY-----\n"
      "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE8+Wkyp8/imL8XQPOPY8kAzpl4fjI\n"
      "G3GG4plUH4r9c4de0b2Aevh3dEvRyy3Ap4wsW+v7aV9aETVz8KjcVAYaMw=\n" // remove ending '='
      "-----END PUBLIC KEY-----";
   test_incorrect_pubkey(tester, pubkey_wrong_format);

   pubkey_wrong_format = 
      "-----BEGIN PUBLIC KEY-----\n"
      "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE8+Wkyp8/imL8XQPOPY8kAzpl4fjI\n"
      "G3GG4plUH4r9c4de0b2Aevh3dEvRyy3Ap4wsW+v7aV9aETVz8KjcVAYaM==\n" // remove ending 'w' befofe `==`
      "-----END PUBLIC KEY-----";
   test_incorrect_pubkey(tester, pubkey_wrong_format);

   pubkey_wrong_format = "";
   test_incorrect_pubkey(tester, pubkey_wrong_format);

   pubkey_wrong_format = "1";
   test_incorrect_pubkey(tester, pubkey_wrong_format);
}

TEST_CASE("ECDSA verify tests", "[ecdsa_verify]" ) {
   eosio::test_chain tester;
   ecdsa_test_setup(tester);
   
   const string message = "message to sign"s;
   // const string signature_hex = //"3046022100a2e5bcb2fc902f2ef1568cc3fc96cef3d2db4ff04f540299b0ebd381cc5589de022100d144da5de33b72fc73f93011fb9632787cab505cae3631de6dfaf1773fed0254"s;
   const string signature_base64 = "MEYCIQCi5byy/JAvLvFWjMP8ls7z0ttP8E9UApmw69OBzFWJ3gIhANFE2l3jO3L8c/kwEfuWMnh8q1BcrjYx3m368Xc/7QJU"s;
   const string pubkey = 
      "-----BEGIN PUBLIC KEY-----\n"
      "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEzjca5ANoUF+XT+4gIZj2/X3V2UuT\n"
      "E9MTw3sQVcJzjyC/p7KeaXommTC/7n501p4Gd1TiTiH+YM6fw/YYJUPSPg==\n"
      "-----END PUBLIC KEY-----"s;

   test_happy_path_verify(tester, message, signature_base64, pubkey);
}

TEST_CASE("ECDSA invalid signature tests", "[ecdsa_invalid_sig]" ) {
   eosio::test_chain tester;
   ecdsa_test_setup(tester);
   
   const string message = "message to sign"s;
   const string pubkey = 
      "-----BEGIN PUBLIC KEY-----\n"
      "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEzjca5ANoUF+XT+4gIZj2/X3V2UuT\n"
      "E9MTw3sQVcJzjyC/p7KeaXommTC/7n501p4Gd1TiTiH+YM6fw/YYJUPSPg==\n"
      "-----END PUBLIC KEY-----"s;

   const string corrupt_signature_base64 = "MEYCIQCi5byy/JAvLvFWjMP8ls7z0ttP8E9UApmw69OBzFWJ3gIhANFE2l3jO3L8c/kwEfuWMnh8q1BcrjYx3m368Xc/7QJf"s; // ending 'U' to 'f'
   test_incorrect_signature(tester, message, corrupt_signature_base64, pubkey);

   const string empty_signature = ""s;
   test_incorrect_signature(tester, message, empty_signature, pubkey);

   const string one_char_signature = "1"s;
   test_incorrect_signature(tester, message, one_char_signature, pubkey);
}

TEST_CASE("ECDSA invalid message or pubkey tests", "[ecdsa_invalid_msg]" ) {
   eosio::test_chain tester;
   ecdsa_test_setup(tester);
   
   const string message = "message to sign"s;
   const string signature_base64 = "MEYCIQCi5byy/JAvLvFWjMP8ls7z0ttP8E9UApmw69OBzFWJ3gIhANFE2l3jO3L8c/kwEfuWMnh8q1BcrjYx3m368Xc/7QJU"s;

   const string pubkey = 
      "-----BEGIN PUBLIC KEY-----\n"
      "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEzjca5ANoUF+XT+4gIZj2/X3V2UuT\n"
      "E9MTw3sQVcJzjyC/p7KeaXommTC/7n501p4Gd1TiTiH+YM6fw/YYJUPSPg==\n"
      "-----END PUBLIC KEY-----"s;

   // empty message
   const string empty_message = ""s;
   test_incorrect_signature(tester, empty_message, signature_base64, pubkey);

   // other message
   const string other_message = "message to signn"s;
   test_incorrect_signature(tester, other_message, signature_base64, pubkey);

   // empty pubkey
   const string empty_pubkey = ""s;
   test_incorrect_signature(tester, message, signature_base64, empty_pubkey);

   // other pubkey
   const string other_pubkey = 
      "-----BEGIN PUBLIC KEY-----\n"
      "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE8+Wkyp8/imL8XQPOPY8kAzpl4fjI\n"
      "G3GG4plUH4r9c4de0b2Aevh3dEvRyy3Ap4wsW+v7aV9aETVz8KjcVAYaMw==\n"
      "-----END PUBLIC KEY-----";
   test_incorrect_signature(tester, message, signature_base64, other_pubkey);
}
