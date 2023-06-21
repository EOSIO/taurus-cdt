#include <eosio/crypto.hpp>
#include <eosio/datastream.hpp>
#include "zpp_json_test.pb.hpp"
#include <iostream>

#include "legacy_tester.hpp"


EOSIO_TEST_BEGIN(zpp_json_test)
    const test::AddApiKeyInput origin{
        .account_id = 1,
        .public_key = eosio::public_key_from_string("EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"),
        .ip_whitelist_commitment = { 'a', 'b' , 'c'},
        .capabilities = { test::Capability::ADD_API_KEY, test::Capability::MARGIN },
        .role_id = 2
    };

    std::vector<char> data;
    eosio::vector_stream strm(data);
    eosio::to_json(origin, strm);

    const std::string_view origin_json = R"({"account_id":"1","public_key":"PUB_K1_6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5BoDq63","ip_whitelist_commitment":"616263","capabilities":["ADD_API_KEY","MARGIN"],"role_id":"2"})";
    
    CHECK_EQUAL( origin_json, std::string_view(data.data(), data.size()) )

    {
        data.push_back('\0');
        test::AddApiKeyInput restored;
        eosio::json_token_stream stream(data.data());
        eosio::from_json(restored, stream);

        CHECK_EQUAL( origin, restored )
    }

EOSIO_TEST_END


int main(int argc, char* argv[]) {
   bool verbose = false;
   if( argc >= 2 && std::strcmp( argv[1], "-v" ) == 0 ) {
      verbose = true;
   }
   silence_output(!verbose);

   EOSIO_TEST(zpp_json_test);
   return has_failed();
}

