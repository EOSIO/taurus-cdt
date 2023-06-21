#include "legacy_tester.hpp"
#include <exception>
#include <inttypes.h>

eosio::cdt::output_stream std_out;

extern "C" {

bool ___disable_output;
bool ___has_failed;
bool ___earlier_unit_test_has_failed;

void eosio_assert(uint32_t test, const char* msg) {
   if (test == 0) {
      throw std::runtime_error(msg);
   }
}

void eosio_assert_message(uint32_t test, const char* msg, uint32_t len) {
   if (test == 0) {
      throw std::runtime_error({msg, len});
   }
}

void eosio_assert_code(uint32_t test, uint64_t code) {
   if (test == 0) {
      char buff[32];
      snprintf(buff, 32, "%" PRIu64, code);
      throw std::runtime_error(buff);
   }
}

// preset the print functions
void prints_l(const char* cs, uint32_t l) { 
    std_out.put({cs, l});
    if (!___disable_output)
        std::cout << std::string_view(cs, l);
}

void prints(const char* cs) { prints_l(cs, strlen(cs)); }

void printi(int64_t v) {
   char buf[32];
   int  l = sprintf(buf, "%" PRId64, v);
   prints_l(buf, l);
}

void printui(uint64_t v) {
   char buf[32];
   int  l = sprintf(buf, "%" PRIu64, v);
   prints_l(buf, l);
};

void printi128(const int128_t* v) {
   char buf[128];
   int* tmp = (int*)v;
   int  l   = sprintf(buf, "0x%04x%04x%04x%04x", tmp[0], tmp[1], tmp[2], tmp[3]);
   prints_l(buf, l);
}

void printui128(const uint128_t* v) {
   char buf[128];
   int* tmp = (int*)v;
   int  l   = sprintf(buf, "0x%04x%04x%04x%04x", tmp[0], tmp[1], tmp[2], tmp[3]);
   prints_l(buf, l);
}

void  printn(uint64_t nm) {
   std::string s = eosio::name(nm).to_string();
   prints_l(s.c_str(), s.length());
}

void printhex(const void* data, uint32_t len) {
   constexpr static uint32_t max_stack_buffer_size = 512;
   const char*               hex_characters        = "0123456789abcdef";

   uint32_t buffer_size = 2 * len;
   if (buffer_size < len)
      eosio_assert(false, "length passed into printhex is too large");

   void* buffer = (max_stack_buffer_size < buffer_size) ? malloc(buffer_size) : alloca(buffer_size);

   char*          b = reinterpret_cast<char*>(buffer);
   const uint8_t* d = reinterpret_cast<const uint8_t*>(data);
   for (uint32_t i = 0; i < len; ++i) {
      *b = hex_characters[d[i] >> 4];
      ++b;
      *b = hex_characters[d[i] & 0x0f];
      ++b;
   }

   prints_l(reinterpret_cast<const char*>(buffer), buffer_size);

   if (max_stack_buffer_size < buffer_size)
      free(buffer);
}
}