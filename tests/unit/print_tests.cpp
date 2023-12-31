#include "legacy_tester.hpp"
#include <eosio/tester.hpp>


EOSIO_TEST_BEGIN(print_test)
   CHECK_PRINT("27", [](){ eosio::print((uint8_t)27); });
   CHECK_PRINT("34", [](){ eosio::print((int)34); });
   CHECK_PRINT([](std::string s){return s[0] == 'a';},  [](){ eosio::print((char)'a'); });
   // CHECK_PRINT([](std::string s){return s[0] == 'b';},  [](){ eosio::print((int8_t)'b'); });
   CHECK_PRINT("202", [](){ eosio::print((unsigned int)202); });
   CHECK_PRINT("-202", [](){ eosio::print((int)-202); });
   CHECK_PRINT("707", [](){ eosio::print((unsigned long)707); });
   CHECK_PRINT("-707", [](){ eosio::print((long)-707); });
   CHECK_PRINT("909", [](){ eosio::print((unsigned long long)909); });
   CHECK_PRINT("-909", [](){ eosio::print((long long)-909); });
   CHECK_PRINT("404", [](){ eosio::print((uint32_t)404); });
   CHECK_PRINT("-404", [](){ eosio::print((int32_t)-404); });
   CHECK_PRINT("404000000", [](){ eosio::print((uint64_t)404000000); });
   CHECK_PRINT("-404000000", [](){ eosio::print((int64_t)-404000000); });
   CHECK_PRINT("0x0066000000000000", [](){ eosio::print((uint128_t)102); });
   CHECK_PRINT("0xffffff9affffffffffffffffffffffff", [](){ eosio::print((int128_t)-102); });
EOSIO_TEST_END

int main(int argc, char** argv) {
   bool verbose = false;
   if( argc >= 2 && std::strcmp( argv[1], "-v" ) == 0 ) {
      verbose = true;
   }
   silence_output(!verbose);

   EOSIO_TEST(print_test);
   return has_failed();
}
