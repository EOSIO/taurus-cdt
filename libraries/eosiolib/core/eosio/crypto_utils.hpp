#pragma once

#include "name.hpp"

#include <string_view>

namespace eosio {

   // base64 and hex functions are adapted from fc base64.cpp and hex.cpp
   static inline bool is_base64_char(unsigned char c) {
      return isalnum(c) || c == '+' || c == '/';
   }

   static constexpr uint8_t base64_lookup[128] = {
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 16
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 32
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63, // 48
       52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255, 255, 255, 255, 255, // 64
      255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14, // 80
       15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255, // 96
      255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40, // 112
       41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255  // 128
   };

   static inline std::string base64_decode(std::string_view const& encoded_string) {
      int in_len = encoded_string.size();
      int i = 0;
      int j = 0;
      int in_ = 0;
      unsigned char char_array_4[4], char_array_3[3];
      std::string ret;
      ret.reserve(in_len / 3 * 4);

      while (in_len-- && encoded_string[in_] != '=') {
         if(!is_base64_char(encoded_string[in_])) {
            return "";
         }
         char_array_4[i++] = encoded_string[in_]; in_++;
         if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = base64_lookup[char_array_4[i]];
            }
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++) {
               ret += char_array_3[i];
            }
            i = 0;
         }
      }

      if (i) {
         for (j = i; j < 4; j++) {
            char_array_4[j] = 0;
         }

         for (j = 0; j < 4; j++) {
            char_array_4[j] = base64_lookup[char_array_4[j]];
         }

         char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
         char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
         char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

         for (j = 0; j < i - 1; j++) {
            ret += char_array_3[j];
         }
      }

      return ret;
   }

   static inline std::string to_hex( const char* d, uint32_t s ) {
      std::string r;
      const char* to_hex="0123456789abcdef";
      uint8_t* c = (uint8_t*)d;
      for( uint32_t i = 0; i < s; ++i ) {
         (r += to_hex[(c[i]>>4)]) += to_hex[(c[i] &0x0f)];
      }
      return r;
    }

   static inline std::string base64_to_hex( const std::string_view& base64_str) {
      std::string decoded = base64_decode(base64_str);
      if (decoded.empty()) {
         return "";
      }
      return to_hex(decoded.data(), decoded.size());
   }

   static inline std::string strip_newline(const std::string_view& in) {
      std::string r{in};
      r.erase(std::remove(r.begin(), r.end(), '\n'), r.cend());
      return r;
   }

   static inline std::string strip_header_trailer(const std::string_view& in) {
      const std::string header = "-----BEGIN PUBLIC KEY-----";
      const std::string trailer = "-----END PUBLIC KEY-----";

      std::string r{in};
      if (r.compare(0, header.size(), header) != 0 || 
          r.compare(r.size() - trailer.size(), trailer.size(), trailer) != 0) {
         return "";
      }
      r.erase(0, header.size());
      r.erase(r.end() - trailer.size(), r.end());

      return r;
   }

   struct rsa_key_parser {
      rsa_key_parser(const std::string_view& key) : key(key) {}

      void read_len(uint32_t& len, uint8_t& len_bytes_num) {
         len = len_bytes_num = 0;

         if (!get_next()) {
            return;
         }
         uint8_t len_byte = cur_byte;
         if (len_byte < 0x80) {
            len = len_byte;
            return;
         }
         if (len_byte > 0x80 && len_byte <= 0x84) {
            for (uint8_t i = 0x80; i < len_byte; ++i) {
               if (!get_next()) {
                  len = len_bytes_num = 0;
                  return;
               }
               len = (len << 8) + uint8_t(cur_byte);
               len_bytes_num++;
            }
         }
      }
      
      uint32_t read_len() {
         uint32_t len = 0;
         uint8_t len_bytes_num = 0;
         read_len(len, len_bytes_num);
         return len;
      }

      void read_seq_len(uint32_t& len, uint8_t& len_bytes_num) {
         if (!get_next() || cur_byte != 0x30) {
            return;
         }
         read_len(len, len_bytes_num);
      }

      uint32_t read_seq_len() {
         if (!get_next() || cur_byte != 0x30) {
            return 0;
         }
         return read_len();
      }

      std::string read_oid() {
         if (!get_next() || cur_byte != 0x06) {
            return "";
         }
         uint32_t len = read_len();
         if (len == 0) {
            return "";
         }
         std::string ret{key.substr(pos, len)};
         if (!get_next(len)) {
            return "";
         }
         return ret;
      }

      bool get_null() {
         return get_next() && cur_byte == 0x05 && get_next() && cur_byte == 0x00;
      }

      uint32_t read_bit_str() {
         if (!get_next() || cur_byte != 0x03) {
            return 0;
         }
         uint32_t len = read_len();
         if (!get_next()) {
            return 0;
         }
         return len;
      }

      std::string read_int() {
         if (!get_next() || cur_byte != 0x02) {
            return "";
         }
         uint32_t len = read_len();
         if (len == 0) {
            return "";
         }

         std::string ret{key.substr(pos, len)};
         if (ret[0] == 0x00) {
            ret.erase(0, 1);
         }
         if (!get_next(len)) {
            return "";
         }
         return ret;
      }

     private:
      std::string_view key;
      uint32_t pos = 0;
      uint8_t cur_byte = 0;

      bool get_next(int step = 1) {
         if (pos >= key.size()) {
            return false;
         }
         cur_byte = key[pos];
         if (pos != key.size() - step) {
            if (pos + step >= key.size()) {
               return false;
            }
            pos += step;
         }
         return true;
      }
   };

   static inline int parse_rsa_pubkey(const std::string& pubkey, std::string& mod, std::string& exp) {
      std::string key_stripped = strip_newline(pubkey);
      if (key_stripped.empty()) {
         return -1;
      }

      key_stripped = strip_header_trailer(key_stripped);
      if (key_stripped.empty()) {
         return -1;
      }

      std::string key = base64_decode(key_stripped);
      if (key.size() < 2) { // "0x30**"
         return -1;
      }

      rsa_key_parser parser(key);
      
      uint8_t len_bytes_num = 0;
      uint32_t key_seq_len = 0;
      parser.read_seq_len(key_seq_len, len_bytes_num);
      if (key_seq_len == 0 || key.size() - 2 - len_bytes_num != key_seq_len ) {
         return -1;
      }

      if (parser.read_seq_len() == 0) {
            return -1;
      }

      const unsigned char rsa_oid[] = {0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01};
      std::string oid_chars = parser.read_oid();
      if (oid_chars.size() != sizeof(rsa_oid) || strncmp(oid_chars.data(), (char *)rsa_oid, sizeof(rsa_oid))) {
         return -1;
      }

      if (!parser.get_null() || parser.read_bit_str() == 0 || parser.read_seq_len() == 0) {
         return -1;
      }

      std::string mod_bytes = parser.read_int();
      if (mod_bytes.empty()) {
         return -1;
      }
      mod = to_hex(mod_bytes.data(), mod_bytes.size());

      std::string exp_bytes = parser.read_int();
      if (exp_bytes.empty()) {
         return -1;
      }
      exp = to_hex(exp_bytes.data(), exp_bytes.size());

      return 0;
   }
}
