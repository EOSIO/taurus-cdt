#pragma once
#include <vector>
#include <eosio/name.hpp>
#include <eosio/to_key.hpp>

#define EOSIO_CDT_GET_RETURN_T(value_class, index_name) std::decay_t<decltype(std::invoke(&value_class::index_name, std::declval<const value_class*>()))>

/**
 * @brief Macro to define an index.
 * @details In the case where the autogenerated index names created by DEFINE_TABLE are not enough, a user can instead
 * manually define the table and indices. This macro allows users to conveniently define an index without having to specify
 * the index template type, as those can be large/unwieldy to type out.
 *
 * @param index_name    - The index name.
 * @param member_name   - The name of the member pointer used for the index. This also defines the index's C++ variable name.
 */
#define KV_NAMED_INDEX(index_name, member_name)                                                                        \
   index<EOSIO_CDT_GET_RETURN_T(value_type, member_name)> member_name{index_name, &value_type::member_name};

namespace eosio {
   namespace internal_use_do_not_use {
      extern "C" {
         __attribute__((import_name("kv_erase"))) 
         int64_t kv_erase(uint64_t contract, const char* key, uint32_t key_size);

         __attribute__((import_name("kv_set"))) 
         int64_t kv_set(uint64_t contract, const char* key, uint32_t key_size, const char* value, uint32_t value_size, uint64_t payer);

         __attribute__((import_name("kv_get"))) 
         bool kv_get(uint64_t contract, const char* key, uint32_t key_size, uint32_t& value_size);

         __attribute__((import_name("kv_get_data"))) 
         uint32_t kv_get_data(uint32_t offset, char* data, uint32_t data_size);

         __attribute__((import_name("kv_it_create"))) 
         uint32_t kv_it_create(uint64_t contract, const char* prefix, uint32_t size);

         __attribute__((import_name("kv_it_destroy"))) 
         void kv_it_destroy(uint32_t itr);

         __attribute__((import_name("kv_it_status"))) 
         int32_t kv_it_status(uint32_t itr);

         __attribute__((import_name("kv_it_compare"))) 
         int32_t kv_it_compare(uint32_t itr_a, uint32_t itr_b);

         __attribute__((import_name("kv_it_key_compare"))) 
         int32_t kv_it_key_compare(uint32_t itr, const char* key, uint32_t size);

         __attribute__((import_name("kv_it_move_to_end"))) 
         int32_t kv_it_move_to_end(uint32_t itr);

         __attribute__((import_name("kv_it_next"))) 
         int32_t kv_it_next(uint32_t itr, uint32_t& found_key_size = (uint32_t&)std::move(uint32_t(0)), uint32_t& found_value_size = (uint32_t&)std::move(uint32_t(0)));

         __attribute__((import_name("kv_it_prev"))) 
         int32_t kv_it_prev(uint32_t itr, uint32_t& found_key_size = (uint32_t&)std::move(uint32_t(0)), uint32_t& found_value_size = (uint32_t&)std::move(uint32_t(0)));

         __attribute__((import_name("kv_it_lower_bound"))) 
         int32_t kv_it_lower_bound(uint32_t itr, const char* key, uint32_t size, uint32_t& found_key_size = (uint32_t&)std::move(uint32_t(0)), uint32_t& found_value_size = (uint32_t&)std::move(uint32_t(0)));

         __attribute__((import_name("kv_it_key"))) 
         int32_t kv_it_key(uint32_t itr, uint32_t offset, char* dest, uint32_t size, uint32_t& actual_size);

         __attribute__((import_name("kv_it_value"))) 
         int32_t kv_it_value(uint32_t itr, uint32_t offset, char* dest, uint32_t size, uint32_t& actual_size);
      }
   }

namespace detail {
   constexpr inline size_t max_stack_buffer_size = 512;
}

/**
 * The key_type struct is used to store the binary representation of a key.
 */
struct key_type : protected std::vector<char> {
   key_type() = default;

   explicit key_type(std::vector<char>&& v) : std::vector<char>(v) {}

   explicit key_type(char* str, size_t size) : std::vector<char>(str, str+size) {}

   key_type operator+(const key_type& b) const {
      key_type ret = *this;
      ret += b;
      return ret;
   }

   key_type& operator+=(const key_type& b) {
      this->insert(this->end(), b.begin(), b.end());
      return *this;
   }


   std::string to_hex() const {
      const char* hex_characters = "0123456789abcdef";

      uint32_t buffer_size = 2 * size();
      check(buffer_size >= size(), "length passed into printhex is too large");

      auto free_memory = [buffer_size](char* buf) { if (detail::max_stack_buffer_size < buffer_size) free(buf);};
      std::unique_ptr<char, decltype(free_memory)> buffer( (char*)(detail::max_stack_buffer_size < buffer_size ? malloc(buffer_size) : alloca(buffer_size)), free_memory);

      char* b = buffer.get();
      const uint8_t* d = reinterpret_cast<const uint8_t*>(data());
      for(uint32_t i = 0; i < size(); ++i) {
         *b = hex_characters[d[i] >> 4];
         ++b;
         *b = hex_characters[d[i] & 0x0f];
         ++b;
      }

      std::string ret{buffer.get(), buffer_size};

      return ret;
   }

   using std::vector<char>::data;
   using std::vector<char>::size;
   using std::vector<char>::resize;
   using std::vector<char>::operator[];
};

struct partial_key : public key_type {
   using key_type::key_type;
};

struct full_key : public key_type {
   using key_type::key_type;

   full_key(const partial_key& a, const partial_key& b) {
      *this += a + b;
   }

   static full_key from_hex( const std::string_view& str ) {
      full_key out;

      check( str.size() % 2 == 0, "invalid hex string length" );
      out.reserve( str.size() / 2 );

      auto start = str.data();
      auto end   = start + str.size();
      for(const char* p = start; p != end; p+=2 ) {
          auto hic = p[0];
          auto lowc = p[1];

          uint8_t hi  = hic  <= '9' ? hic-'0' : 10+(hic-'a');
          uint8_t low = lowc <= '9' ? lowc-'0' : 10+(lowc-'a');

          out.push_back( char((hi << 4) | low) );
      }

      return out;
   }
};

/* @cond PRIVATE */
template <typename T>
inline partial_key make_key(T&& t) {
   return partial_key(convert_to_key(std::forward<T>(t)));
}
inline partial_key make_key(partial_key&& t) {
   return std::move(t);
}
inline partial_key make_key(partial_key& t) {
   return t;
}
inline partial_key make_key(const partial_key& t) {
   return t;
}

inline partial_key make_prefix(eosio::name table_name, eosio::name index_name, uint8_t status = 1) {
   return make_key(std::make_tuple(status, table_name, index_name));
}

/* @endcond */

// This is the "best" way to document a function that does not technically exist using Doxygen.
#if EOSIO_CDT_DOXYGEN
/**
 * @brief A function for converting types to the appropriate binary representation for the EOSIO Key Value database.
 * @details The CDT provides implementations of this function for many of the common primitives and for structs/tuples.
 * If sticking with standard types, contract developers should not need to interact with this function.
 * If doing something more advanced, contract developers may need to provide their own implementation for a special type.
 */
template <typename T>
inline partial_key make_key(T val) {
   return {};
}
#endif
}