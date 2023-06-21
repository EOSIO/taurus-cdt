#pragma once
#include <eosio/eosio.hpp>
#include <vector>
#include <iostream>

extern "C" bool ___disable_output;
extern "C" bool ___has_failed;
extern "C" bool ___earlier_unit_test_has_failed;

inline void silence_output(bool t) {
   ___disable_output = t;
}
inline bool has_failed() {
   return ___has_failed;
}

namespace eosio { namespace cdt {
   struct output_stream {
      char output[1024*2];
      size_t index = 0;
      std::string to_string()const { return std::string((const char*)output, index); }
      const char* get()const { return output; }
      void push(char c) { output[index++] = c; }
      void clear() { index = 0; }
      void put(std::string_view str) { std::copy(str.begin(), str.end(), output+index); index += str.size();}
   };
}} //ns eosio::cdt

extern eosio::cdt::output_stream std_out;

template <typename Pred, typename F, typename... Args>
inline bool expect_assert(bool check, const std::string& li, Pred&& pred, F&& func, Args&&... args) {
   try {
      func(std::forward<Args&&>(args)...);
      if (!check)
         eosio::check(false, std::string("error : expect_assert, no assert {"+li+"}").c_str());
      std::cout << "error : expect_assert, no assert {" << li << "}\n";
      return false;
   }
   catch (std::runtime_error& err) {
      bool passed = pred(err.what());
      if (!check)
         eosio::check(passed, std::string("error : expect_assert, wrong assert {"+li+"}").c_str());
      if (!passed)
         std::cout << "error : expect_assert, no assert {" << li << "}\n";
      return passed;
   }
}

template <size_t N, typename F, typename... Args>
inline bool expect_assert(bool check, const std::string& li, const char (&expected)[N], F&& func, Args... args) {
   return expect_assert(check, li,
         [&](const std::string& s) {
            return s.size() == N-1 &&
            memcmp(expected, s.c_str(), N-1) == 0; }, func, args...);
}

template <typename Pred, typename F, typename... Args>
inline bool expect_print(bool check, const std::string& li, Pred&& pred, F&& func, Args... args) {
   std_out.clear();
   func(args...);
   bool passed = pred(std_out.get());
   std_out.clear();
   bool disable_out = ___disable_output;
   silence_output(false);
   if (!check)
      eosio::check(passed, std::string("error : wrong print message {"+li+"}").c_str());
   if (!passed)
      std::cout << "error : wrong print message {" << li << "}\n";
   silence_output(disable_out);
   return passed;
}

template <size_t N, typename F, typename... Args>
inline bool expect_print(bool check, const std::string& li, const char (&expected)[N], F&& func, Args... args) {
   return expect_print(check, li,
         [&](const std::string& s) {
            return std_out.index == N-1 &&
            memcmp(expected, s.c_str(), N-1) == 0; }, func, args...);

}

#define CHECK_ASSERT(...) \
   ___has_failed |= !expect_assert(true, std::string(__FILE__)+":"+__func__+":"+(std::to_string(__LINE__)), __VA_ARGS__);

#define REQUIRE_ASSERT(...) \
   expect_assert(false, std::string(__FILE__)+":"+__func__+":"+(std::to_string(__LINE__)),  __VA_ARGS__);

#define CHECK_PRINT(...) \
   ___has_failed |= !expect_print(true, std::string(__FILE__)+":"+__func__+":"+(std::to_string(__LINE__)), __VA_ARGS__);

#define REQUIRE_PRINT(...) \
   expect_print(false, std::string(__FILE__)+":"+__func__+":"+(std::to_string(__LINE__)),  __VA_ARGS__);

#define CHECK_EQUAL(X, Y) \
   if (!(X == Y)) { \
      ___has_failed = true; \
      std::cout << std::string("CHECK_EQUAL failed (")+#X+" != "+#Y+") {"+__FILE__+":"+std::to_string(__LINE__)+"}\n"; \
   }

#define REQUIRE_EQUAL(X, Y) \
   eosio::check(X == Y, std::string(std::string("REQUIRE_EQUAL failed (")+#X+" != "+#Y+") {"+__FILE__+":"+std::to_string(__LINE__)+"}").c_str());

#define EOSIO_TEST(X) \
   try { X(); } \
   catch (std::runtime_error& err) { \
      std::cout << "\033[1;37m" #X, " \033[0;37munit test \033[1;31mfailed\033[0m (aborted)\n"; \
      ___has_failed = true; \
   }

#define EOSIO_TEST_BEGIN(X) \
   void X() { \
      static constexpr const char* __test_name = #X; \
      ___earlier_unit_test_has_failed = ___has_failed; \
      ___has_failed = false;

#define EOSIO_TEST_END \
      bool ___original_disable_output = ___disable_output; \
      silence_output(false); \
      if (___has_failed) \
         std::cout << "\033[1;37m", __test_name, " \033[0;37munit test \033[1;31mfailed\033[0m\n"; \
      else \
         std::cout << "\033[1;37m", __test_name, " \033[0;37munit test \033[1;32mpassed\033[0m\n"; \
      silence_output(___original_disable_output); \
      ___has_failed |= ___earlier_unit_test_has_failed; \
      ___earlier_unit_test_has_failed = ___has_failed; \
   }

