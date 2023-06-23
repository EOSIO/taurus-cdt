#include <catch2/catch.hpp>
#include <eosio/tester.hpp>
#include <eosio/datastream.hpp>
#include <contracts.hpp>

using namespace std::literals;
using namespace eosio;

using std::string;

void setup(test_chain& tester) {
   // activate verify_rsa_sha256_sig protocol feature
   tester.set_code( "eosio"_n, eosio::testing::contracts::boot_wasm() );
   tester.transact( {eosio::action({"eosio"_n, "active"_n}, "eosio"_n, "activate"_n,
                    tester.make_checksum256("00bca72bd868bc602036e6dea1ede57665b57203e3daaf18e6992e77d0d0341c"))} );
   tester.finish_block();

   // deploy test contract
   tester.create_code_account( "test"_n );
   tester.finish_block();
   tester.set_code( "test"_n, eosio::testing::contracts::rsa_verify_test_wasm() );
   tester.finish_block();
}

void test_happy_path(test_chain& tester, const string& message, const string& signature, const string& exponent, const string& modulus, const string& signature_base64, const string& pubkey) {
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify"_n, std::tuple(message, signature, exponent, modulus))} );
   tester.finish_block();

   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify2"_n, std::tuple(message, signature_base64, pubkey))} );
   tester.finish_block();
}

void test_incorrect_signature(test_chain& tester, const string& message, const string& corrupt_signature, const string& exponent, const string& modulus, const string& corrupt_signature_base64, const string& pubkey) {
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify"_n, std::tuple(message, corrupt_signature, exponent, modulus))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();

   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify2"_n, std::tuple(message, corrupt_signature_base64, pubkey))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();
}

TEST_CASE("RSA verify tests 1024 bit", "[rsa_verify_1024]" ) {
   eosio::test_chain tester;
   setup(tester);
   
   const string message = "message to sign"s;
   const string signature_hex = 
      "62afd13281f01a5b5a9710c274cc9b58f9a92f8575a7b2099cd5038173d838be"
      "b77cd2912e96166a724b7fb8391c96a67e18208a52047755a5af2d01101966e5"
      "40a3e9e075675b69faa177d3401673834459ee977a8b5c11db4351e61286207d"
      "25a5194bdbd66cb57ee716dda4d342f7eec09303500a2f0cf3c8141d46fa821f"s;
   const string signature_base64 = 
      "Yq/RMoHwGltalxDCdMybWPmpL4V1p7IJnNUDgXPYOL63fNKRLpYWanJLf7g5HJamfhggilIEd1Wlry0BEBlm5UCj6eB1Z1tp+qF300AWc4NEWe6XeotcEdtDUeYShiB9"
      "JaUZS9vWbLV+5xbdpNNC9+7AkwNQCi8M88gUHUb6gh8="s;
   const string exponent = "3"s;
   const string modulus = 
      "dff568a53cafdba7b1cd654fef54ed61649cdd6cb29fa743e35c73fcba7ef9c2"
      "b25a3b91e295abcea9aa5af0625f8b06428ec3140f2dd3c60c7dbb698cb3dbf6"
      "c64b1160daec4eb7d6deca1dfc45b83d5f30e5398f6f737ee394d57c8d2bf412"
      "f056c2e8a54d9bf554149c0da31346e31f23ffb516b1f9797d650169199b7add"s;
   const string pubkey = 
      "-----BEGIN PUBLIC KEY-----"
      "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQDf9WilPK/bp7HNZU/vVO1hZJzd"
      "bLKfp0PjXHP8un75wrJaO5HilavOqapa8GJfiwZCjsMUDy3Txgx9u2mMs9v2xksR"
      "YNrsTrfW3sod/EW4PV8w5TmPb3N+45TVfI0r9BLwVsLopU2b9VQUnA2jE0bjHyP/"
      "tRax+Xl9ZQFpGZt63QIBAw=="
      "-----END PUBLIC KEY-----"s;

   // #1: happy path.
   test_happy_path(tester, message, signature_hex, exponent, modulus, signature_base64, pubkey);

   // #2: minimal message length.
   const string signature_for_1_hex = 
      "8248d7ed6562ae6e7d8d914bda6c39ba16d7f474569ec07a5ec6b49618b0cb03"
      "55bf294c583dc44cab4d10a716740391b392d1adbeeaaa29a8b1239e855e729d"
      "a035191827e011869b96d9900f4da98c500c7bdaf27b77788ff2f93fcee4706c"
      "f60a9bc735ddc6ebd8187b2e354581cb84e9a8d521980fb6d16eb736baa0197f"s;
   const string signature_for_1_base64 = 
      "gkjX7WVirm59jZFL2mw5uhbX9HRWnsB6Xsa0lhiwywNVvylMWD3ETKtNEKcWdAORs5LRrb7qqimosSOehV5ynaA1GRgn4BGGm5bZkA9NqYxQDHva8nt3eI/y+T/O5HBs"
      "9gqbxzXdxuvYGHsuNUWBy4TpqNUhmA+20W63NrqgGX8="s;
   test_happy_path(tester, "1"s, signature_for_1_hex, exponent, modulus, signature_for_1_base64, pubkey);

   // #3: incorrect signature.
   const string corrupt_signature_hex = 
      "72afd13281f01a5b5a9710c274cc9b58f9a92f8575a7b2099cd5038173d838be"
      "b77cd2912e96166a724b7fb8391c96a67e18208a52047755a5af2d01101966e5"
      "40a3e9e075675b69faa177d3401673834459ee977a8b5c11db4351e61286207d"
      "25a5194bdbd66cb57ee716dda4d342f7eec09303500a2f0cf3c8141d46fa821f"s;

   const string corrupt_signature_base64 = 
      "cq/RMoHwGltalxDCdMybWPmpL4V1p7IJnNUDgXPYOL63fNKRLpYWanJLf7g5HJamfhggilIEd1Wlry0BEBlm5UCj6eB1Z1tp+qF300AWc4NEWe6XeotcEdtDUeYShiB9"
      "JaUZS9vWbLV+5xbdpNNC9+7AkwNQCi8M88gUHUb6gh8="s;
   test_incorrect_signature(tester, message, corrupt_signature_hex, exponent, modulus, corrupt_signature_base64, pubkey);
}

TEST_CASE("RSA verify tests 2048 bit", "[rsa_verify_2048]" ) {
   eosio::test_chain tester;
   setup(tester);
   
   const string message = "message to sign"s;
   const string signature_hex = 
      "476455bfe93c2fdc226b3c0f325feb9fbf22234e92e417aa22ed1b24ee0b127ba2cab513bb4089488fb70199a53a736f0baf3db3d4bb630b6574f685259125dc"
      "18777a9e8f66a853ea69edf04965a80aa35b2bbf4b46f50fd744b676864a5933e53c9b2cc7d8dcc7edebba58d762652350d8a3ca64265319135cb92621731824"
      "452dfc839d4412874f1ada5ff41b2bbbb2d10d878125bbf9632787d2c0ec4c3912eb07187a103623298b2233a07b051e0e34151b7e1ed6095bfe3d4994284013"
      "bd6998d7a84ca6725497dce9bb7c3fe6e2481b5b050ad0a5d91622945cf62a9f22524dc32bb2cdf9a1cb0b77be0a1dd3bc58d29899bfa5a2688f6353d75e4c16"s;
   const string signature_base64 = 
      "R2RVv+k8L9wiazwPMl/rn78iI06S5BeqIu0bJO4LEnuiyrUTu0CJSI+3AZmlOnNvC689s9S7YwtldPaFJZEl3Bh3ep6PZqhT6mnt8EllqAqjWyu/S0b1D9dEtnaGSlkz"
      "5TybLMfY3Mft67pY12JlI1DYo8pkJlMZE1y5JiFzGCRFLfyDnUQSh08a2l/0Gyu7stENh4Elu/ljJ4fSwOxMORLrBxh6EDYjKYsiM6B7BR4ONBUbfh7WCVv+PUmUKEAT"
      "vWmY16hMpnJUl9zpu3w/5uJIG1sFCtCl2RYilFz2Kp8iUk3DK7LN+aHLC3e+Ch3TvFjSmJm/paJoj2NT115MFg=="s;
   const string exponent = "10001"s;
   const string modulus = 
      "e06bccbf7d2cbe0d5420d62e8448a8b4165eb2b6431e64e5bbdf84580f3c4dfb49da522a6f66897a5a8b8c6c8bb448cb7b51a08e5f70c199a4e13e567b496636"
      "9a503226418c10838c109c3b37cca70157dbbcad7682bdf348b625f88492260780d3bc2efa94f2d3018a74df68ccfa6edcd01531b7a546af170f74116dabb1ab"
      "4951798e389c37ae12c5b4845e9e2a287ff4d23fa785c137a8bb3af6b147c260aabc0d1c92a3e429cdaf7b3d1903df53569e0eb284e530fd23eef57cd07c8468"
      "362bd63c41b8abdad3645dab9e74bc49d8fc040bb16f2afb167bb6e9a95454e124f8c3fc3c46420862c5f42f0c82f08a04b3309312a23161740ef6d38b3eead5"s;
   const string pubkey = 
      "-----BEGIN PUBLIC KEY-----"
      "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4GvMv30svg1UINYuhEio"
      "tBZesrZDHmTlu9+EWA88TftJ2lIqb2aJelqLjGyLtEjLe1Ggjl9wwZmk4T5We0lm"
      "NppQMiZBjBCDjBCcOzfMpwFX27ytdoK980i2JfiEkiYHgNO8LvqU8tMBinTfaMz6"
      "btzQFTG3pUavFw90EW2rsatJUXmOOJw3rhLFtIRenioof/TSP6eFwTeouzr2sUfC"
      "YKq8DRySo+Qpza97PRkD31NWng6yhOUw/SPu9XzQfIRoNivWPEG4q9rTZF2rnnS8"
      "Sdj8BAuxbyr7Fnu26alUVOEk+MP8PEZCCGLF9C8MgvCKBLMwkxKiMWF0DvbTiz7q"
      "1QIDAQAB"
      "-----END PUBLIC KEY-----"s;

   test_happy_path(tester, message, signature_hex, exponent, modulus, signature_base64, pubkey);

   const string signature_for_1_hex = 
      "9ed86640bc0236066f03071d2f96cf3b60f5c62eaed5b9cc590f3f88f6828238b12acbcef25ccac37c74710cbd9537650dbcd1384b9f89e0e6478baa76696177"
      "2f018d02d9bf1a1bde07dfb8c56c5cb40224615beade4fd77f745347585bc7d95fa251fef382da8983ca8ffa73014823377942236a6b448c0cf9af3355481c44"
      "87e1e257702f95e875e526cbdb5825ded6b5a4812b733e9997cb2be3b18dcf4ed7ad221fe93ca1f4f799ad4685ba17d64af62caafe43d9ccb26179dc20c0908f"
      "6bdc9bf78c09bf03409bebb96b064e1ab5585b5fd32e2fd203dfbb411a991c4717a78ff77db6283b5f629936507021033b3ca35e7d481cfac4233a9949c99cb1"s;
   const string signature_for_1_base64 = 
      "nthmQLwCNgZvAwcdL5bPO2D1xi6u1bnMWQ8/iPaCgjixKsvO8lzKw3x0cQy9lTdlDbzROEufieDmR4uqdmlhdy8BjQLZvxob3gffuMVsXLQCJGFb6t5P1390U0dYW8fZ"
      "X6JR/vOC2omDyo/6cwFIIzd5QiNqa0SMDPmvM1VIHESH4eJXcC+V6HXlJsvbWCXe1rWkgStzPpmXyyvjsY3PTtetIh/pPKH095mtRoW6F9ZK9iyq/kPZzLJhedwgwJCP"
      "a9yb94wJvwNAm+u5awZOGrVYW1/TLi/SA9+7QRqZHEcXp4/3fbYoO19imTZQcCEDOzyjXn1IHPrEIzqZScmcsQ=="s;
   test_happy_path(tester, "1"s, signature_for_1_hex, exponent, modulus, signature_for_1_base64, pubkey);

   const string corrupt_signature_hex = 
      "576455bfe93c2fdc226b3c0f325feb9fbf22234e92e417aa22ed1b24ee0b127ba2cab513bb4089488fb70199a53a736f0baf3db3d4bb630b6574f685259125dc"
      "18777a9e8f66a853ea69edf04965a80aa35b2bbf4b46f50fd744b676864a5933e53c9b2cc7d8dcc7edebba58d762652350d8a3ca64265319135cb92621731824"
      "452dfc839d4412874f1ada5ff41b2bbbb2d10d878125bbf9632787d2c0ec4c3912eb07187a103623298b2233a07b051e0e34151b7e1ed6095bfe3d4994284013"
      "bd6998d7a84ca6725497dce9bb7c3fe6e2481b5b050ad0a5d91622945cf62a9f22524dc32bb2cdf9a1cb0b77be0a1dd3bc58d29899bfa5a2688f6353d75e4c16"s;
   const string corrupt_signature_base64 = 
      "V2RVv+k8L9wiazwPMl/rn78iI06S5BeqIu0bJO4LEnuiyrUTu0CJSI+3AZmlOnNvC689s9S7YwtldPaFJZEl3Bh3ep6PZqhT6mnt8EllqAqjWyu/S0b1D9dEtnaGSlkz"
      "5TybLMfY3Mft67pY12JlI1DYo8pkJlMZE1y5JiFzGCRFLfyDnUQSh08a2l/0Gyu7stENh4Elu/ljJ4fSwOxMORLrBxh6EDYjKYsiM6B7BR4ONBUbfh7WCVv+PUmUKEAT"
      "vWmY16hMpnJUl9zpu3w/5uJIG1sFCtCl2RYilFz2Kp8iUk3DK7LN+aHLC3e+Ch3TvFjSmJm/paJoj2NT115MFg=="s;
   test_incorrect_signature(tester, message, corrupt_signature_hex, exponent, modulus, corrupt_signature_base64, pubkey);
}

TEST_CASE("RSA verify tests 3072 bit", "[rsa_verify_3072]" ) {
   eosio::test_chain tester;
   setup(tester);
   
   const string message = "message to sign"s;
   const string signature_hex = 
      "9ddb7f0a71d2b2cbc4b9d2d31c781f52639aa5b019909a62b8aef7fe6350477fd8f3fa968fa5433e69fff8439b88881cad52e4af6f722dd41e27b9226fa43b81"
      "0bfb9907d464e1f855a8f2c5a6733db86942f55d5751c7d80df3078ca8e10ded44e77abab484fbda557407c9df9371aea849e3807f5958cdb031c8e2b7b6991f"
      "eb55af410e3ad3d5d822ca868c27e23c4718c40f14c1cca448e3067cc3926e08cae26f3ce4ee4b7d632bdede35a52d73ffc50ceab94e451db143ad2a8582d967"
      "5cabe47d99ba9e7da8657720e6cb2ff34b0af942059666d94a320d966c4adb52fbc4fcb9b2618ea705aa4d3ebee5d9915c748cbedba54c47464bb2c6ab7cce29"
      "872172e1a2e66493e7813746ec3a366245fad4e2ac96b78e7c1bab04e3db1c1318ebebff9f21c434382f9f36f812030de51f5f58ddbf9f3ccdb1b8f1f15c8185"
      "c99c4a8cd0d969f511d4a8ba5dd947c2e036b59f610fa5fce509286dde53ceec3f61de216c2cfd729e6586925f32d2660d6dda7bc9c362ed7775cf0085fb3260"s;
   const string signature_base64 = 
      "ndt/CnHSssvEudLTHHgfUmOapbAZkJpiuK73/mNQR3/Y8/qWj6VDPmn/+EObiIgcrVLkr29yLdQeJ7kib6Q7gQv7mQfUZOH4VajyxaZzPbhpQvVdV1HH2A3zB4yo4Q3t"
      "ROd6urSE+9pVdAfJ35NxrqhJ44B/WVjNsDHI4re2mR/rVa9BDjrT1dgiyoaMJ+I8RxjEDxTBzKRI4wZ8w5JuCMribzzk7kt9Yyve3jWlLXP/xQzquU5FHbFDrSqFgtln"
      "XKvkfZm6nn2oZXcg5ssv80sK+UIFlmbZSjINlmxK21L7xPy5smGOpwWqTT6+5dmRXHSMvtulTEdGS7LGq3zOKYchcuGi5mST54E3Ruw6NmJF+tTirJa3jnwbqwTj2xwT"
      "GOvr/58hxDQ4L582+BIDDeUfX1jdv588zbG48fFcgYXJnEqM0Nlp9RHUqLpd2UfC4Da1n2EPpfzlCSht3lPO7D9h3iFsLP1ynmWGkl8y0mYNbdp7ycNi7Xd1zwCF+zJg"s;
   const string exponent = "10001"s;
   const string modulus = 
      "df05d9a2d0209ec535f9987e574864efb53a0944cd45cfee779277ab335e9b0ae15ac55004ff3a70f23773017af49dda374bb2747ffa0751b39c63e4d7f98f59"
      "ac80568f4c1a9847ffe4ee74d3049ec2f5b3a2997f81c4f33291b5ffa25d49386972332df766daca2c15b40bce8c40e4233e7dd200a4f7352bd4ddbb81274f41"
      "c51498fbf2077416a073d833263492b040380ea50876dbce249fc60e2320655ae2198d72f2cdbb841a5497c45e5198e7b9b7903f0b991fbe5e2b0b0f27225c2a"
      "75f147efc35cc0eafc1607ff50aaf3c0bf1d001af27f7a6aff907f42a1da13a07ec745614c3a12a7d551281257a655eee9c08c2524edc54775ce5501d7ae1bd1"
      "f0a4d510943cf25e93db2ec1928de4c7682fcb0a4a9f68ad68b76f6b98a6fb928f23283850c859a2aa334ec131d04867b81bf80308c8cf1553eaa33ffd8585b2"
      "e8126183d7d09891d7dff0d5f0ddf96bdb29d8380e72dbda31f309b42bd916105ccccf3b596d73b490c0eba432813cda7580bdc8a65fbdd6b0091b37c273c88f"s;
   const string pubkey = 
      "-----BEGIN PUBLIC KEY-----"
      "MIIBojANBgkqhkiG9w0BAQEFAAOCAY8AMIIBigKCAYEA3wXZotAgnsU1+Zh+V0hk"
      "77U6CUTNRc/ud5J3qzNemwrhWsVQBP86cPI3cwF69J3aN0uydH/6B1GznGPk1/mP"
      "WayAVo9MGphH/+TudNMEnsL1s6KZf4HE8zKRtf+iXUk4aXIzLfdm2sosFbQLzoxA"
      "5CM+fdIApPc1K9Tdu4EnT0HFFJj78gd0FqBz2DMmNJKwQDgOpQh2284kn8YOIyBl"
      "WuIZjXLyzbuEGlSXxF5RmOe5t5A/C5kfvl4rCw8nIlwqdfFH78NcwOr8Fgf/UKrz"
      "wL8dABryf3pq/5B/QqHaE6B+x0VhTDoSp9VRKBJXplXu6cCMJSTtxUd1zlUB164b"
      "0fCk1RCUPPJek9suwZKN5MdoL8sKSp9orWi3b2uYpvuSjyMoOFDIWaKqM07BMdBI"
      "Z7gb+AMIyM8VU+qjP/2FhbLoEmGD19CYkdff8NXw3flr2ynYOA5y29ox8wm0K9kW"
      "EFzMzztZbXO0kMDrpDKBPNp1gL3Ipl+91rAJGzfCc8iPAgMBAAE="
      "-----END PUBLIC KEY-----"s;

   test_happy_path(tester, message, signature_hex, exponent, modulus, signature_base64, pubkey);

   const string signature_for_1_hex = 
      "0d438058d079323b270895bf0f3d40eebbc9c122fe8e5344ec260d9bb3244ab37b8331b6ca8bd5ae6dc673d36ace84dc17825a44e8cd222fca26c553f19192ab"
      "33fbbe0f4b359d975bb5abf125ded0fa03f1a70cffc0527d57a54ee768b29c46011dabc7466c5ca221f2df4b5d5cc230b381a5275c0a6c30713bace72f52d6ca"
      "8da76eb0e58c7c6cf0309bc461a19a9da13501b607c002fbc1686f90f159468f545e38ff5743c600b3ff14ba4ff3ed1ad8c26b1b244a45768a12ee8bc2d43da3"
      "56c6f87ae42eed38a7e42787a20d738fdd90fc184122b45dcfede4a4c7c506929d9dec3127638d14d4cd883a568afcb6079b0af9d742090f9e256a3cfc33d9af"
      "a39cb89f4a08067d9b2c593c8f54ebd5157bc1313f161bc8609af60afbcb33dcd60b5b4f8f55d21eddf17d3010575aa0f232b7a8b499a1141b82b36dd3b12fc3"
      "bbf5e850f54d540f5bed8cb9fd943352548f801d4b26c282c8d953bb53f98404c72b1fd38e14184aa139dcd2482449a571ecfaf6c65bbfeba52df34c2870e9f0"s;
   const string signature_for_1_base64 = 
      "DUOAWNB5MjsnCJW/Dz1A7rvJwSL+jlNE7CYNm7MkSrN7gzG2yovVrm3Gc9NqzoTcF4JaROjNIi/KJsVT8ZGSqzP7vg9LNZ2XW7Wr8SXe0PoD8acM/8BSfVelTudospxG"
      "AR2rx0ZsXKIh8t9LXVzCMLOBpSdcCmwwcTus5y9S1sqNp26w5Yx8bPAwm8RhoZqdoTUBtgfAAvvBaG+Q8VlGj1ReOP9XQ8YAs/8Uuk/z7RrYwmsbJEpFdooS7ovC1D2j"
      "Vsb4euQu7Tin5CeHog1zj92Q/BhBIrRdz+3kpMfFBpKdnewxJ2ONFNTNiDpWivy2B5sK+ddCCQ+eJWo8/DPZr6OcuJ9KCAZ9myxZPI9U69UVe8ExPxYbyGCa9gr7yzPc"
      "1gtbT49V0h7d8X0wEFdaoPIyt6i0maEUG4KzbdOxL8O79ehQ9U1UD1vtjLn9lDNSVI+AHUsmwoLI2VO7U/mEBMcrH9OOFBhKoTnc0kgkSaVx7Pr2xlu/66Ut80wocOnw"s;
   test_happy_path(tester, "1"s, signature_for_1_hex, exponent, modulus, signature_for_1_base64, pubkey);

   const string corrupt_signature_hex = 
      "addb7f0a71d2b2cbc4b9d2d31c781f52639aa5b019909a62b8aef7fe6350477fd8f3fa968fa5433e69fff8439b88881cad52e4af6f722dd41e27b9226fa43b81"
      "0bfb9907d464e1f855a8f2c5a6733db86942f55d5751c7d80df3078ca8e10ded44e77abab484fbda557407c9df9371aea849e3807f5958cdb031c8e2b7b6991f"
      "eb55af410e3ad3d5d822ca868c27e23c4718c40f14c1cca448e3067cc3926e08cae26f3ce4ee4b7d632bdede35a52d73ffc50ceab94e451db143ad2a8582d967"
      "5cabe47d99ba9e7da8657720e6cb2ff34b0af942059666d94a320d966c4adb52fbc4fcb9b2618ea705aa4d3ebee5d9915c748cbedba54c47464bb2c6ab7cce29"
      "872172e1a2e66493e7813746ec3a366245fad4e2ac96b78e7c1bab04e3db1c1318ebebff9f21c434382f9f36f812030de51f5f58ddbf9f3ccdb1b8f1f15c8185"
      "c99c4a8cd0d969f511d4a8ba5dd947c2e036b59f610fa5fce509286dde53ceec3f61de216c2cfd729e6586925f32d2660d6dda7bc9c362ed7775cf0085fb3260"s;
   const string corrupt_signature_base64 = 
      "rdt/CnHSssvEudLTHHgfUmOapbAZkJpiuK73/mNQR3/Y8/qWj6VDPmn/+EObiIgcrVLkr29yLdQeJ7kib6Q7gQv7mQfUZOH4VajyxaZzPbhpQvVdV1HH2A3zB4yo4Q3t"
      "ROd6urSE+9pVdAfJ35NxrqhJ44B/WVjNsDHI4re2mR/rVa9BDjrT1dgiyoaMJ+I8RxjEDxTBzKRI4wZ8w5JuCMribzzk7kt9Yyve3jWlLXP/xQzquU5FHbFDrSqFgtln"
      "XKvkfZm6nn2oZXcg5ssv80sK+UIFlmbZSjINlmxK21L7xPy5smGOpwWqTT6+5dmRXHSMvtulTEdGS7LGq3zOKYchcuGi5mST54E3Ruw6NmJF+tTirJa3jnwbqwTj2xwT"
      "GOvr/58hxDQ4L582+BIDDeUfX1jdv588zbG48fFcgYXJnEqM0Nlp9RHUqLpd2UfC4Da1n2EPpfzlCSht3lPO7D9h3iFsLP1ynmWGkl8y0mYNbdp7ycNi7Xd1zwCF+zJg"s;
   test_incorrect_signature(tester, message, corrupt_signature_hex, exponent, modulus, corrupt_signature_base64, pubkey);
}

TEST_CASE("RSA verify tests 4096 bit", "[rsa_verify_4096]" ) {
   eosio::test_chain tester;
   setup(tester);
   
   const string message = "message to sign"s;
   const string signature_hex = 
      "3d2644876c4d231da2e6293541db48fca54d3f161fb8f755c9a10c240d2d8621fbd824b47c5110e6776c5556eb91d8f88b27a83568a1a379665406750f9ee6a8"
      "4943831a5d9b72f4c487c2371d8dcf3f09d4bfa5cc52213d3d25ad57333d6948655edf5b43c5997fc8ee5efdbf69a7ed1f8947015c91afa15d7981499c2a9fa9"
      "3fc973a6054deea4274fa4977226328f7155cabf26b27d49f06ff3c339be11df9f10dc4c2917e084330792ae9f27151dc5f97b5464c5b28280feb12d3786691d"
      "a1ca6895dcfbffe0101190efbdad8f3c2fe9d20fcfcf03eac089a750806de6fc5e2a5087592290866a66bb19a6cf9c74d8be26f89d11a0ac8deb60eabd9e9ac0"
      "9c16eaac352c2f9488c2785a85eb176a3d1e70dab0177f3b0fd1b9ed7da576e9529d4bc2691c0240ddaac93502719a8d69e46ee16727e456dbe1e98c44ddac6d"
      "be9f5a0b7c5d2e82fc527c524f9c25cb3a354dafcfdecaf0b6b04f4539e72240440ecfed3d78c307e4fb36fc4102bda1db646e1f029996787966cfbf5d8b9677"
      "91ef023a1ba3111bcc0f63aa4438aa13b4edc6a9db61ef329885207e7ae34939a068f6cbd1056df6f8b7383b97b1256aeb792de7d0866be65abedf95bb44218a"
      "d10287480ee167b05a6011f41155af51e9706af7f98f20b4b33ac6b8fb82c70847bb02816d561585d130b4963811cc5da5463160b0a4e310c0a323ebb0203820"s;
   const string signature_base64 = 
      "PSZEh2xNIx2i5ik1QdtI/KVNPxYfuPdVyaEMJA0thiH72CS0fFEQ5ndsVVbrkdj4iyeoNWiho3lmVAZ1D57mqElDgxpdm3L0xIfCNx2Nzz8J1L+lzFIhPT0lrVczPWlI"
      "ZV7fW0PFmX/I7l79v2mn7R+JRwFcka+hXXmBSZwqn6k/yXOmBU3upCdPpJdyJjKPcVXKvyayfUnwb/PDOb4R358Q3EwpF+CEMweSrp8nFR3F+XtUZMWygoD+sS03hmkd"
      "ocpoldz7/+AQEZDvva2PPC/p0g/PzwPqwImnUIBt5vxeKlCHWSKQhmpmuxmmz5x02L4m+J0RoKyN62DqvZ6awJwW6qw1LC+UiMJ4WoXrF2o9HnDasBd/Ow/Rue19pXbp"
      "Up1LwmkcAkDdqsk1AnGajWnkbuFnJ+RW2+HpjETdrG2+n1oLfF0ugvxSfFJPnCXLOjVNr8/eyvC2sE9FOeciQEQOz+09eMMH5Ps2/EECvaHbZG4fApmWeHlmz79di5Z3"
      "ke8COhujERvMD2OqRDiqE7TtxqnbYe8ymIUgfnrjSTmgaPbL0QVt9vi3ODuXsSVq63kt59CGa+Zavt+Vu0QhitECh0gO4WewWmAR9BFVr1HpcGr3+Y8gtLM6xrj7gscI"
      "R7sCgW1WFYXRMLSWOBHMXaVGMWCwpOMQwKMj67AgOCA="s;
   const string exponent = "10001"s;
   const string modulus = 
      "b22b4d012c9bb745a9ea09d743765ae0f2f97ea50dfff03e0bdf8891eed12b440da1ab11a44cda07758ff40484aaa571a7e7d1552264c7bf06af9c3b9a8b5ca1"
      "4751cb86bdddd52d88b4bccfd1ca6393e0aae0ad89a19c2bb91c07ae1ec6cfa1642c3773f92e5872e0a9c6c38240415cc99f205e11c5f9692e1f62373dd37af1"
      "d325a3b68b685e64fff605b07e7b24963177819107b724f84da4c2f9f583469aafb77f5023e03281883e8df6746c54bae51e428dcf38897681a7f99ad01f3310"
      "98cbc4657a92825f5a625372114fc05a2a76ffdb8b9be2f13db17ece9473b47dbdb771abad7abbca0198904be80db590e3759806d97dbe2910a0fe7ee5d1f9cc"
      "89407a13bda1185ea6bdd319c6e0fa64ad181cbf4fed79e08b2a8a47d6f6a060e5f768d6e2bfb057ceb20993f1d61e5574551537eec43568b530c37044981cd2"
      "c5c12c8c327d55a2b46348e673bc549d943c9a658e7721f9d64fab7e8f3586cad832ae2fd707395f08ed201f886ca292a4b52a6636c54685e4840476e99648fb"
      "1274ddf06f120b03cc7e660771618edeee655adefcec7ebac09d6a34ef6b39aa9045b3793621c49a0f5cf1fe78229068f3478ced811b8a14f06ab5985019cf71"
      "abfd1ed7e25e817de65bfbd35a32d6021341773fc42f0a4e5139fae50c7ce245cad7c05ded1facb33fad6f210e997b44dffb1bc820b0a232851994894196a427"s;
   const string pubkey = 
      "-----BEGIN PUBLIC KEY-----"
      "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAsitNASybt0Wp6gnXQ3Za"
      "4PL5fqUN//A+C9+Ike7RK0QNoasRpEzaB3WP9ASEqqVxp+fRVSJkx78Gr5w7motc"
      "oUdRy4a93dUtiLS8z9HKY5PgquCtiaGcK7kcB64exs+hZCw3c/kuWHLgqcbDgkBB"
      "XMmfIF4RxflpLh9iNz3TevHTJaO2i2heZP/2BbB+eySWMXeBkQe3JPhNpML59YNG"
      "mq+3f1Aj4DKBiD6N9nRsVLrlHkKNzziJdoGn+ZrQHzMQmMvEZXqSgl9aYlNyEU/A"
      "Wip2/9uLm+LxPbF+zpRztH29t3GrrXq7ygGYkEvoDbWQ43WYBtl9vikQoP5+5dH5"
      "zIlAehO9oRhepr3TGcbg+mStGBy/T+154IsqikfW9qBg5fdo1uK/sFfOsgmT8dYe"
      "VXRVFTfuxDVotTDDcESYHNLFwSyMMn1VorRjSOZzvFSdlDyaZY53IfnWT6t+jzWG"
      "ytgyri/XBzlfCO0gH4hsopKktSpmNsVGheSEBHbplkj7EnTd8G8SCwPMfmYHcWGO"
      "3u5lWt787H66wJ1qNO9rOaqQRbN5NiHEmg9c8f54IpBo80eM7YEbihTwarWYUBnP"
      "cav9HtfiXoF95lv701oy1gITQXc/xC8KTlE5+uUMfOJFytfAXe0frLM/rW8hDpl7"
      "RN/7G8ggsKIyhRmUiUGWpCcCAwEAAQ=="
      "-----END PUBLIC KEY-----";

   test_happy_path(tester, message, signature_hex, exponent, modulus, signature_base64, pubkey);

   const string signature_for_1_hex = 
      "91957ef8ef91378dd5e88697be4be3a8447c54c71b942c7001751442709bef73b394f06eb0e9f82af9226a8d9008394d7f1fb456b23cb409e547e9027484c5cb"
      "c92b4d4557395e5c47b6193f06914131982cd154317a9dd17116b4014e36b0b147dd4446e7b34c781c921c20e1025c7312d8adde2bc39e3c7aa356fc92798804"
      "c4418d3d47c7ab5b452688d8f0a2952942d4b95f21cc947171cdf7e354e251433a63016e6eac242e79f808579c179aa1479f230d2b2019b16c62ce1303f34a0d"
      "cd075883e72ddba002f62fa73ba4bb7c06b0e3c95955e2fbc28a4a81766a5ee6ae5e28b1d9afb6dae3575bf2f8ac18ced7690163a1ade86931c24370789d7661"
      "6724de105498e6834a47ad2c12cfc9dfe14a11f599c4bdfbdf243e085a581d1361be0f25882e256b17b6006f5181ac9aacb5fae79273201a66717c8823d452f0"
      "1a62df653b086836483834c61011e248b7db776795ec9e75863dabef4ff4cbb435c2782e1f2c7de89787d37be29932bb279e613e7cacf0bbe1ad64b30104de80"
      "1600ba168c6864dec7cc74e5e62894b9b57a64ef9c4e0dce570db979e02e7e6734fd758c3d4c92ab2be4d4952cb1a954f02a445a84a6889015535cc22effaf2a"
      "083aa7bd4c4fd80cbf094e793b79697d705cb01ab07dc23a8da4b3763a86296d37dd4349a6bd0ecb8e4a5dbd65918b5dac70a4f915de18a015433d351321ee6e"s;
   const string signature_for_1_base64 = 
      "kZV++O+RN43V6IaXvkvjqER8VMcblCxwAXUUQnCb73OzlPBusOn4Kvkiao2QCDlNfx+0VrI8tAnlR+kCdITFy8krTUVXOV5cR7YZPwaRQTGYLNFUMXqd0XEWtAFONrCx"
      "R91ERuezTHgckhwg4QJccxLYrd4rw548eqNW/JJ5iATEQY09R8erW0UmiNjwopUpQtS5XyHMlHFxzffjVOJRQzpjAW5urCQuefgIV5wXmqFHnyMNKyAZsWxizhMD80oN"
      "zQdYg+ct26AC9i+nO6S7fAaw48lZVeL7wopKgXZqXuauXiix2a+22uNXW/L4rBjO12kBY6Gt6GkxwkNweJ12YWck3hBUmOaDSketLBLPyd/hShH1mcS9+98kPghaWB0T"
      "Yb4PJYguJWsXtgBvUYGsmqy1+ueScyAaZnF8iCPUUvAaYt9lOwhoNkg4NMYQEeJIt9t3Z5XsnnWGPavvT/TLtDXCeC4fLH3ol4fTe+KZMrsnnmE+fKzwu+GtZLMBBN6A"
      "FgC6FoxoZN7HzHTl5iiUubV6ZO+cTg3OVw25eeAufmc0/XWMPUySqyvk1JUssalU8CpEWoSmiJAVU1zCLv+vKgg6p71MT9gMvwlOeTt5aX1wXLAasH3COo2ks3Y6hilt"
      "N91DSaa9DsuOSl29ZZGLXaxwpPkV3higFUM9NRMh7m4="s;
   test_happy_path(tester, "1"s, signature_for_1_hex, exponent, modulus, signature_for_1_base64, pubkey);

   const string corrupt_signature_hex = 
      "4d2644876c4d231da2e6293541db48fca54d3f161fb8f755c9a10c240d2d8621fbd824b47c5110e6776c5556eb91d8f88b27a83568a1a379665406750f9ee6a8"
      "4943831a5d9b72f4c487c2371d8dcf3f09d4bfa5cc52213d3d25ad57333d6948655edf5b43c5997fc8ee5efdbf69a7ed1f8947015c91afa15d7981499c2a9fa9"
      "3fc973a6054deea4274fa4977226328f7155cabf26b27d49f06ff3c339be11df9f10dc4c2917e084330792ae9f27151dc5f97b5464c5b28280feb12d3786691d"
      "a1ca6895dcfbffe0101190efbdad8f3c2fe9d20fcfcf03eac089a750806de6fc5e2a5087592290866a66bb19a6cf9c74d8be26f89d11a0ac8deb60eabd9e9ac0"
      "9c16eaac352c2f9488c2785a85eb176a3d1e70dab0177f3b0fd1b9ed7da576e9529d4bc2691c0240ddaac93502719a8d69e46ee16727e456dbe1e98c44ddac6d"
      "be9f5a0b7c5d2e82fc527c524f9c25cb3a354dafcfdecaf0b6b04f4539e72240440ecfed3d78c307e4fb36fc4102bda1db646e1f029996787966cfbf5d8b9677"
      "91ef023a1ba3111bcc0f63aa4438aa13b4edc6a9db61ef329885207e7ae34939a068f6cbd1056df6f8b7383b97b1256aeb792de7d0866be65abedf95bb44218a"
      "d10287480ee167b05a6011f41155af51e9706af7f98f20b4b33ac6b8fb82c70847bb02816d561585d130b4963811cc5da5463160b0a4e310c0a323ebb0203820"s;
   const string corrupt_signature_base64 =      
      "TSZEh2xNIx2i5ik1QdtI/KVNPxYfuPdVyaEMJA0thiH72CS0fFEQ5ndsVVbrkdj4iyeoNWiho3lmVAZ1D57mqElDgxpdm3L0xIfCNx2Nzz8J1L+lzFIhPT0lrVczPWlI"
      "ZV7fW0PFmX/I7l79v2mn7R+JRwFcka+hXXmBSZwqn6k/yXOmBU3upCdPpJdyJjKPcVXKvyayfUnwb/PDOb4R358Q3EwpF+CEMweSrp8nFR3F+XtUZMWygoD+sS03hmkd"
      "ocpoldz7/+AQEZDvva2PPC/p0g/PzwPqwImnUIBt5vxeKlCHWSKQhmpmuxmmz5x02L4m+J0RoKyN62DqvZ6awJwW6qw1LC+UiMJ4WoXrF2o9HnDasBd/Ow/Rue19pXbp"
      "Up1LwmkcAkDdqsk1AnGajWnkbuFnJ+RW2+HpjETdrG2+n1oLfF0ugvxSfFJPnCXLOjVNr8/eyvC2sE9FOeciQEQOz+09eMMH5Ps2/EECvaHbZG4fApmWeHlmz79di5Z3"
      "ke8COhujERvMD2OqRDiqE7TtxqnbYe8ymIUgfnrjSTmgaPbL0QVt9vi3ODuXsSVq63kt59CGa+Zavt+Vu0QhitECh0gO4WewWmAR9BFVr1HpcGr3+Y8gtLM6xrj7gscI"
      "R7sCgW1WFYXRMLSWOBHMXaVGMWCwpOMQwKMj67AgOCA="s;
   test_incorrect_signature(tester, message, corrupt_signature_hex, exponent, modulus, corrupt_signature_base64, pubkey);
}

TEST_CASE("RSA verify tests invalid parameter lengths", "[rsa_verify_invalid_para_len]") {
   eosio::test_chain tester;
   setup(tester);

   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify"_n, std::tuple(""s, "abcd"s, "abcd"s, "abcd"s))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();

   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify"_n, std::tuple("abcd"s, ""s, "abcd"s, "abcd"s))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();

   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify"_n, std::tuple("abcd"s, "abcd"s, ""s, "abcd"s))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();

   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify"_n, std::tuple("abcd"s, "abcd"s, "abcd"s, ""s))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();
}

TEST_CASE("RSA verify tests invalid hex strings", "[rsa_verify_invalid_hex_str]") {
   eosio::test_chain tester;
   setup(tester);

   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify"_n, std::tuple("foo"s, "XXXX"s, "abcd"s, "abcd"s))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();

   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify"_n, std::tuple("foo"s, "abcd"s, "XXXX"s, "abcd"s))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();

   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify"_n, std::tuple("foo"s, "abcd"s, "abcd"s, "XXXX"s))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();
}

TEST_CASE("RSA verify tests malformed parameter length", "[rsa_verify_malformed_para_length]") {
   eosio::test_chain tester;
   setup(tester);

   const string message = "message to sign"s;
   const string signature = 
      "62afd13281f01a5b5a9710c274cc9b58f9a92f8575a7b2099cd5038173d838be"
      "b77cd2912e96166a724b7fb8391c96a67e18208a52047755a5af2d01101966e5"
      "40a3e9e075675b69faa177d3401673834459ee977a8b5c11db4351e61286207d"
      "25a5194bdbd66cb57ee716dda4d342f7eec09303500a2f0cf3c8141d46fa821f"s;
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify"_n, std::tuple(message, signature, "A"s, "B"s))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();
}

TEST_CASE("RSA verify tests malformed base64 signature", "[rsa_verify_malformed_base64_sig]") {
   eosio::test_chain tester;
   setup(tester);

   const string message = "message to sign"s;
   const string base64_signature_malformat = 
      "?q/RMoHwGltalxDCdMybWPmpL4V1p7IJnNUDgXPYOL63fNKRLpYWanJLf7g5HJamfhggilIEd1Wlry0BEBlm5UCj6eB1Z1tp+qF300AWc4NEWe6XeotcEdtDUeYShiB9"
      "JaUZS9vWbLV+5xbdpNNC9+7AkwNQCi8M88gUHUb6gh8="s;
   const string pubkey = 
      "-----BEGIN PUBLIC KEY-----"
      "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQDf9WilPK/bp7HNZU/vVO1hZJzd"
      "bLKfp0PjXHP8un75wrJaO5HilavOqapa8GJfiwZCjsMUDy3Txgx9u2mMs9v2xksR"
      "YNrsTrfW3sod/EW4PV8w5TmPb3N+45TVfI0r9BLwVsLopU2b9VQUnA2jE0bjHyP/"
      "tRax+Xl9ZQFpGZt63QIBAw=="
      "-----END PUBLIC KEY-----"s;
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify2"_n, std::tuple(message, base64_signature_malformat, pubkey))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();
}

TEST_CASE("RSA verify tests malformed public key", "[rsa_verify_malformed_pubkey]") {
   eosio::test_chain tester;
   setup(tester);

   const string message = "message to sign"s;
   const string signature = 
      "Yq/RMoHwGltalxDCdMybWPmpL4V1p7IJnNUDgXPYOL63fNKRLpYWanJLf7g5HJamfhggilIEd1Wlry0BEBlm5UCj6eB1Z1tp+qF300AWc4NEWe6XeotcEdtDUeYShiB9"
      "JaUZS9vWbLV+5xbdpNNC9+7AkwNQCi8M88gUHUb6gh8="s;
   string malformed_pubkey = 
      "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQDf9WilPK/bp7HNZU/vVO1hZJzd"
      "bLKfp0PjXHP8un75wrJaO5HilavOqapa8GJfiwZCjsMUDy3Txgx9u2mMs9v2xksR"
      "YNrsTrfW3sod/EW4PV8w5TmPb3N+45TVfI0r9BLwVsLopU2b9VQUnA2jE0bjHyP/"
      "tRax+Xl9ZQFpGZt63QIBAw=="
      "-----END PUBLIC KEY-----"s;
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify2"_n, std::tuple(message, signature, malformed_pubkey))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();

   malformed_pubkey = 
      "-----BEGIN PUBLIC KEY-----"
      "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQDf9WilPK/bp7HNZU/vVO1hZJzd"
      "bLKfp0PjXHP8un75wrJaO5HilavOqapa8GJfiwZCjsMUDy3Txgx9u2mMs9v2xksR"
      "YNrsTrfW3sod/EW4PV8w5TmPb3N+45TVfI0r9BLwVsLopU2b9VQUnA2jE0bjHyP/"
      "tRax+Xl9ZQFpGZt63QIBAw=="s;
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify2"_n, std::tuple(message, signature, malformed_pubkey))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();

   // RSA OID: 300d06092a864886f70d0101010500 => 400d06092a864886f70d0101010500
   malformed_pubkey = 
      "-----BEGIN PUBLIC KEY-----"
      "MIGdQA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQDf9WilPK/bp7HNZU/vVO1hZJzd"
      "bLKfp0PjXHP8un75wrJaO5HilavOqapa8GJfiwZCjsMUDy3Txgx9u2mMs9v2xksR"
      "YNrsTrfW3sod/EW4PV8w5TmPb3N+45TVfI0r9BLwVsLopU2b9VQUnA2jE0bjHyP/"
      "tRax+Xl9ZQFpGZt63QIBAw=="
      "-----END PUBLIC KEY-----"s;
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify2"_n, std::tuple(message, signature, malformed_pubkey))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();

   malformed_pubkey = ""s;
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify2"_n, std::tuple(message, signature, malformed_pubkey))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();

   malformed_pubkey = "30"s;
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify2"_n, std::tuple(message, signature, malformed_pubkey))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();

   malformed_pubkey = 
      "-----BEGIN PUBLIC KEY-----"
      "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQDf9WilPK/bp7HNZU/vVO1hZJzd"
      "bLKfp0PjXHP8un75wrJaO5HilavOqapa8GJfiwZCjsMUDy3Txgx9u2mMs9v2xksR"
      "YNrsTrfW3sod/EW4PV8w5TmPb3N+45TVfI0r9BLwVsLopU2b9VQUnA2jE0bjHyP/"
      "tRax+Xl9ZQFpGZt63QIBAw"s + "A"s + "=="s + // add additional char
      "-----END PUBLIC KEY-----"s; 
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "verify2"_n, std::tuple(message, signature, malformed_pubkey))},
                    "verify_rsa_sha256_sig() failed" );
   tester.finish_block();

}

TEST_CASE("Supported public key", "[supported_public_key]") {
   eosio::test_chain tester;
   setup(tester);

   string supported_pubkey = 
      "-----BEGIN PUBLIC KEY-----"
      "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDAcA+tI6Aid17NCYfafiI2onCf"
      "9QaLJJUp0WKAGh7FS7s4Qwbvu/iN17P2gdrAwsN5uNQjLV6E+fyi7bUTN8Za0zPh"
      "j2FVnnOGt4fzbaQdyp/bBzOuGJg6/650xm/I39IjA2cAJIRThoF2E4ejEpJx2NsG"
      "u1uVGckwJOxp6hpO5wIDAQAB"
      "-----END PUBLIC KEY-----"s;
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "acceptkey"_n, std::tuple(supported_pubkey))} );
   tester.finish_block();

   supported_pubkey =
      "-----BEGIN PUBLIC KEY-----"
      "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA6oD3s2HnTWNIu43KYMbO"
      "cpfNWAPoOyItJaizDubJdnIGX9BF1QAMgmCOBMZTMQVxYYkkejt1ZfvtRtkm+y33"
      "N8Vf5B9euG3rT/NGMVNWDpZOOneZ8h06GVTBdfonfysbh73bKf0JaKtIXKnkFvvw"
      "rDc+qsBIcYoCRUVq007iEsW2uni+PlVAura3vnhdxjqH+y0x1gsQcYD2mZtkaPVH"
      "tj0Bs/GWebI9E3x8D511meqHX+lxkXq6/wLVFrHNz/wRDJBhfuljT8E0j0Nk8y7b"
      "Wpfu6juHnAtNEx12qPOFp19Kd32XNMLSNefCYwndLgMkhRPZEjTHSbq+pZjCK5Gu"
      "kQIDAQAB"
      "-----END PUBLIC KEY-----"s;
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "acceptkey"_n, std::tuple(supported_pubkey))} );
   tester.finish_block();

   supported_pubkey =
      "-----BEGIN PUBLIC KEY-----"
      "MIIBojANBgkqhkiG9w0BAQEFAAOCAY8AMIIBigKCAYEAu0PcMY+blqn94ON+Ts7E"
      "m6ykFF1y/Q1XZ4lrH7hj0A7vEqzDAZIDHW7FrIDamnc9513YhQ8G+AZMtmwa2onS"
      "UrfcbDE0MWf5FfysUCDFgz/ZpHQiIIbmplttY63qcekGJBJyiHiVGjjGpqLtaPnY"
      "+E5SOeiTh+NTtAUCfqCU+qp4Foi+qB8EB2rP1VPIcp0SHF/gtTsrUm4Wxc26lv/i"
      "OtGLw8bztEDHGwlrSkRvAE5pCJBwwz5x5tMfzlEOQwscGL9+Op/NsAZI6ePTFKR0"
      "I2GszCeQm8X26gCi2mUKjjHOOBc265nYWCl6kDOp96k88dF8ZmK6uU28GllQ9dHB"
      "89r3WJU1hFwBnlzpOjEtMEpRiw3JTfRtIWD+QRXAJ1ZT1YXZ2TrfWWa3KVeSGcAv"
      "9Jd1nXIMwFKqkpQADnly07tDn8q/J3SUhhVHR0H+WLTVgss0Ni1XUtYXSiqGUNFv"
      "4ALgbaVr3i+x3KC2BSVacwFppS+kDXpUnPHVIs4PqwvnAgMBAAE="
      "-----END PUBLIC KEY-----"s;
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "acceptkey"_n, std::tuple(supported_pubkey))} );
   tester.finish_block();

   supported_pubkey =
      "-----BEGIN PUBLIC KEY-----"
      "MIIBojANBgkqhkiG9w0BAQEFAAOCAY8AMIIBigKCAYEAu0PcMY+blqn94ON+Ts7E"
      "m6ykFF1y/Q1XZ4lrH7hj0A7vEqzDAZIDHW7FrIDamnc9513YhQ8G+AZMtmwa2onS"
      "UrfcbDE0MWf5FfysUCDFgz/ZpHQiIIbmplttY63qcekGJBJyiHiVGjjGpqLtaPnY"
      "+E5SOeiTh+NTtAUCfqCU+qp4Foi+qB8EB2rP1VPIcp0SHF/gtTsrUm4Wxc26lv/i"
      "OtGLw8bztEDHGwlrSkRvAE5pCJBwwz5x5tMfzlEOQwscGL9+Op/NsAZI6ePTFKR0"
      "I2GszCeQm8X26gCi2mUKjjHOOBc265nYWCl6kDOp96k88dF8ZmK6uU28GllQ9dHB"
      "89r3WJU1hFwBnlzpOjEtMEpRiw3JTfRtIWD+QRXAJ1ZT1YXZ2TrfWWa3KVeSGcAv"
      "9Jd1nXIMwFKqkpQADnly07tDn8q/J3SUhhVHR0H+WLTVgss0Ni1XUtYXSiqGUNFv"
      "4ALgbaVr3i+x3KC2BSVacwFppS+kDXpUnPHVIs4PqwvnAgMBAAE="
      "-----END PUBLIC KEY-----"s;
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "acceptkey"_n, std::tuple(supported_pubkey))} );
   tester.finish_block();

   supported_pubkey =
      "-----BEGIN PUBLIC KEY-----"
      "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA01auyBWkfTTg8GOX3vKX"
      "BpAvflVH0nA1yjw3V1GOwzWUUJ5RfvkcPja4acsCwgIqfktoBSJT0BkWVrsIWixN"
      "Ij79UX6E3Q/Gl+9DLEgIubMLCki+nHODfHu521sT8VshUrIC5k5WgCg55hHD/pt0"
      "pvkKjaztk2AW3EnP3z+emii5Jll/2yIqQs+ri47DgL6TQPxlsvx8NxQ6PSFv+Ekg"
      "KmBKoQm9TE5WaKDX4KgxWYMebhHduM5hs/SEaLAXsREkgiLO6KnlCG4bUxbuZC0M"
      "tYm9UppAtiiRVBc03aK/Jh6yj6qWZP2WgXclfdQ834hYRw50Sp6aIh2D4D0uk8MV"
      "lHaPnOffZrbznykcu/GNimady1WbqdKE0U+0ysEk7VKpcXtU1aryUXuJB+vi23Mp"
      "O/jZj32ZxiUUDvpRtPZZ6vuq8CN+XKO0QysaY5qDfBpnxNgWwIIAZP2/9HpxWJ5k"
      "rtK9BJax0Z/G+as4e6XFZM5DhkoaJMwcmWAo0SJZSNz6EOSgafjsg7lTpTx4gUnA"
      "UXJTZfybbXW2wiB2bPfG9CicLYb6aItnXr9cIqGT4at9M96+LXNNq8qojsuku1/C"
      "Cehtas7/G2NIj9BDi8uvhG6hPTreB4KH8W/zrrA6e+Eg+lrcyH2Gtoyq0ygaMWcR"
      "VTEx+mV+ImxxBpSfHHManHkCAwEAAQ=="
      "-----END PUBLIC KEY-----"s;
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "acceptkey"_n, std::tuple(supported_pubkey))} );
   tester.finish_block();
}

TEST_CASE("Unsupported public key", "[unsupported_public_key]") {
   eosio::test_chain tester;
   setup(tester);

   string unsupported_pubkey = 
      "-----BEGIN RSA PUBLIC KEY-----"
      "MIGJAoGBANjczlc2RxE4lroc+GubOyxwC2AmqY/Hy0MV/cvd6Uok5bU1VWSrJr8q"
      "aeLjpy5U/5G0F97KetfFi6gV7HnqRrk1y0SrabDmb1UsCLnfcwnQEiVXElBJIXqm"
      "ulBGgHIdH44lBEdoFFqgQSfnoqVXv4WJ5WSjXvyPuWTtGjdSnz8JAgMBAAE="
      "-----END RSA PUBLIC KEY-----"s;
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "acceptkey"_n, std::tuple(unsupported_pubkey))},
                    "is_supported_rsa_pubkey() failed" );
   tester.finish_block();

   unsupported_pubkey = ""s;
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "acceptkey"_n, std::tuple(unsupported_pubkey))},
                    "is_supported_rsa_pubkey() failed" );
   tester.finish_block();

   unsupported_pubkey = "30"s;
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "acceptkey"_n, std::tuple(unsupported_pubkey))},
                    "is_supported_rsa_pubkey() failed" );
   tester.finish_block();

   unsupported_pubkey = 
      "-----BEGIN PUBLIC KEY-----"
      "MIGdMA0GCSqGSIb3DQEBAQUAA4GLADCBhwKBgQDf9WilPK/bp7HNZU/vVO1hZJzd"
      "bLKfp0PjXHP8un75wrJaO5HilavOqapa8GJfiwZCjsMUDy3Txgx9u2mMs9v2xksR"
      "YNrsTrfW3sod/EW4PV8w5TmPb3N+45TVfI0r9BLwVsLopU2b9VQUnA2jE0bjHyP/"
      "tRax+Xl9ZQFpGZt63QIBAw"s + "A"s + "=="s +// ending with an additional char
      "-----END PUBLIC KEY-----"s;
   tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "acceptkey"_n, std::tuple(unsupported_pubkey))},
                    "is_supported_rsa_pubkey() failed" );

   unsupported_pubkey = 
      "-----BEGIN PUBLIC KEY-----"
      "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAqGrSS7I74muvDNArUgHP"
      "T9JFIhGZDSaUv6MgtZ5XLJ659jN4rxlmKmqP1AoT156B5H8gpktxRcBkGxv7+AVc"
      "iTNTuTX5EqEiJqKKhOIPx72JdH8V+JHMedVgKrGTr2Og3dlXZ9+ZIp0R33cdCBrq"
      "BTXN/cev4QcdAWZlJhaff/F9/n7yjAOjmAQkUtMRCRSknsbq4AM/IGPwc/MwlbTN"
      "mmT21xpvjiesRBZW6Fr1Qaf/I/yKyfJ0/gqEUrU386ObvG2asVPfepBX//fdxq80"
      "bf8RBRRppt0HYuqjwUjUEly3pqkrakAkH3r+U5eXfcwfsoLPCF+XNAw9TYF/O3Qr"
      "2Kn9k+O2YBZH+sf43PYTcN1bSfxhO96HOKw1ITahBEFzSWkmFOOkSZRPPhi0JmzD"
      "kF58ujcLHNLHaCCiKKwm96W+YixWLjxThHPubNxgapRUnHCWrvmSUIancJM/5eLn"
      "fJcc+l4BfrOeTf3BZNHrfeTx217rU9CB+1lV/NGNtFObDcVLi9iD4fIzVCHcubO8"
      "M1eTcOlkWUOpI9L5sgDH/fORYOhdZPKHPgCLGP3N6iRh1MH3yI7"//Xg4bAxirqkrqH, remove some chars
      "ZwA34/UvyKeqqMAWx833Lt62Pk/A2Xz8FsX9GM2hMChzqgY7zuLoh6xCkqfSsEYP"
      "5SHrUmD2STiPRU165QllubECAwEAAQ=="
      "-----END PUBLIC KEY-----"s;
      tester.transact( {eosio::action({"test"_n, "active"_n},  "test"_n, "acceptkey"_n, std::tuple(unsupported_pubkey))},
                    "is_supported_rsa_pubkey() failed" );
      tester.finish_block();
}