#pragma once
#include <eosio/kv_base.hpp>
#include <eosio/datastream.hpp>
#include <eosio/varint.hpp>
#include <boost/preprocessor/stringize.hpp>


#include <algorithm>
#include <cctype>
#include <functional>

namespace eosio {
// Ignored by kv_table, maintained for now so contracts do not have to change
static constexpr eosio::name kv_ram = "eosio.kvram"_n;
static constexpr eosio::name kv_disk = "eosio.kvdisk"_n;

template<typename T>
class kv_table;

namespace kv_detail {

   class kv_table_base;

   class kv_index {

   public:

      eosio::name index_name;
      eosio::name table_name;
      eosio::name contract_name;

      full_key to_table_key( const partial_key& k )const{ return full_key( prefix, k ); }

   protected:
      kv_index() = default;

      template <typename KF, typename T>
      kv_index(eosio::name::raw index_name, KF&& kf, T*) : index_name{index_name} {
         key_function = [=](const void* t) {
            return make_key(std::invoke(kf, static_cast<const T*>(t)));
         };
      }

      template<typename T>
      partial_key get_key(const T& inst) const { return key_function(&inst); }
      partial_key get_key_void(const void* ptr) const { return key_function(ptr); }

      void get(const full_key& key, void* ret_val, void (*deserialize)(void*, const void*, std::size_t)) const;

      kv_table_base* tbl;
      partial_key prefix;

   private:
      template<typename T>
      friend class eosio::kv_table;
      friend class kv_table_base;
      friend class iterator_base;

      std::function<partial_key(const void*)> key_function;

      virtual void setup() = 0;
   };

   class kv_table_base {
    protected:
      friend class kv_index;
      friend class iterator_base;
      eosio::name contract_name;
      eosio::name table_name;
      uint64_t db_name;

      eosio::name primary_index_name;

      kv_index* primary_index;
      std::vector<kv_index*> secondary_indices;

      void put(const void* value, void* old_value,
               std::size_t (*get_size)(const void*),
               void (*deserialize)(void*, const void*, std::size_t),
               void (*serialize)(const void*, void*, std::size_t)) {
         uint32_t value_size;

         auto primary_key = primary_index->get_key_void(value);
         auto tbl_key = full_key(make_prefix(table_name, primary_index->index_name), primary_key);

         auto primary_key_found = internal_use_do_not_use::kv_get(contract_name.value, tbl_key.data(), tbl_key.size(), value_size);

         if (primary_key_found) {
            auto free_memory = [value_size](char* buf) { if (detail::max_stack_buffer_size < value_size) free(buf);};
            std::unique_ptr<char, decltype(free_memory)> buffer( (char*)(detail::max_stack_buffer_size < value_size ? malloc(value_size) : alloca(value_size)), free_memory);

            auto copy_size = internal_use_do_not_use::kv_get_data(0, buffer.get(), value_size);
            deserialize(old_value, buffer.get(), copy_size);
         }

         eosio::name payer = contract_name;
         for (const auto& idx : secondary_indices) {
            uint32_t value_size;
            auto sec_tbl_key = full_key(make_prefix(table_name, idx->index_name), idx->get_key_void(value));
            auto sec_found = internal_use_do_not_use::kv_get(contract_name.value, sec_tbl_key.data(), sec_tbl_key.size(), value_size);

            if (!primary_key_found) {
               eosio::check(!sec_found, "Attempted to store an existing secondary index.");
               internal_use_do_not_use::kv_set(contract_name.value, sec_tbl_key.data(), sec_tbl_key.size(), tbl_key.data(), tbl_key.size(), payer.value);
            } else {
               if (sec_found) {
                  auto free_memory = [value_size](char* buf) { if (detail::max_stack_buffer_size < value_size) free(buf);};
                  std::unique_ptr<char, decltype(free_memory)> buffer( (char*)(detail::max_stack_buffer_size < value_size ? malloc(value_size) : alloca(value_size)), free_memory);

                  auto copy_size = internal_use_do_not_use::kv_get_data(0, buffer.get(), value_size);

                  auto res = memcmp(buffer.get(), tbl_key.data(), copy_size);
                  eosio::check(copy_size == tbl_key.size() && res == 0, "Attempted to update an existing secondary index.");

               } else {
                  auto old_sec_key = full_key(make_prefix(table_name, idx->index_name), idx->get_key_void(old_value));
                  internal_use_do_not_use::kv_erase(contract_name.value, old_sec_key.data(), old_sec_key.size());
                  internal_use_do_not_use::kv_set(contract_name.value, sec_tbl_key.data(), sec_tbl_key.size(), tbl_key.data(), tbl_key.size(), payer.value);
               }
            }
         }

         size_t data_size = get_size(value);
         auto free_memory = [data_size](char* buf) { if (detail::max_stack_buffer_size < data_size) free(buf);};
         std::unique_ptr<char, decltype(free_memory)> data_buffer( (char*)(detail::max_stack_buffer_size < data_size ? malloc(data_size) : alloca(data_size)), free_memory);

         serialize(value, data_buffer.get(), data_size);

         internal_use_do_not_use::kv_set(contract_name.value, tbl_key.data(), tbl_key.size(), data_buffer.get(), data_size, payer.value);
      }

      void erase(const void* value) {
         uint32_t value_size;

         auto primary_key = primary_index->get_key_void(value);
         auto tbl_key = full_key(make_prefix(table_name, primary_index->index_name), primary_key);
         auto primary_key_found = internal_use_do_not_use::kv_get(contract_name.value, tbl_key.data(), tbl_key.size(), value_size);

         if (!primary_key_found) {
            return;
         }

         for (const auto& idx : secondary_indices) {
            auto sec_tbl_key = full_key(make_prefix(table_name, idx->index_name), idx->get_key_void(value));
            internal_use_do_not_use::kv_erase(contract_name.value, sec_tbl_key.data(), sec_tbl_key.size());
         }

         internal_use_do_not_use::kv_erase(contract_name.value, tbl_key.data(), tbl_key.size());
      }
   };

   inline void kv_index::get(const full_key& key, void* ret_val, void (*deserialize)(void*, const void*, std::size_t)) const {
      uint32_t value_size;
      uint32_t actual_data_size;

      auto success = internal_use_do_not_use::kv_get(contract_name.value, key.data(), key.size(), value_size);
      if (!success) {
         return;
      }

      auto free_memory = [value_size](char* buf) { if (detail::max_stack_buffer_size < value_size) free(buf);};
      std::unique_ptr<char, decltype(free_memory)> buffer( (char*)(detail::max_stack_buffer_size < value_size ? malloc(value_size) : alloca(value_size)), free_memory);
      auto copy_size = internal_use_do_not_use::kv_get_data(0, buffer.get(), value_size);

      bool is_primary = index_name == tbl->primary_index_name;
      if (!is_primary) {
         auto success = internal_use_do_not_use::kv_get(contract_name.value, buffer.get(), copy_size, actual_data_size);
         eosio::check(success, "failure getting primary key");
         auto free_memory = [actual_data_size](char* buf) { if (detail::max_stack_buffer_size < actual_data_size) free(buf);};
         std::unique_ptr<char, decltype(free_memory)> pk_buffer( (char*)(detail::max_stack_buffer_size < actual_data_size ? malloc(actual_data_size) : alloca(actual_data_size)), free_memory);

         auto pk_copy_size = internal_use_do_not_use::kv_get_data(0, pk_buffer.get(), actual_data_size);

         deserialize(ret_val, pk_buffer.get(), pk_copy_size);
      } else {
         deserialize(ret_val, buffer.get(), copy_size);
      }
   }

   class iterator_base {
   public:
      enum class status {
         iterator_ok     = 0,  // Iterator is positioned at a key-value pair
         iterator_erased = -1, // The key-value pair that the iterator used to be positioned at was erased
         iterator_end    = -2, // Iterator is out-of-bounds
      };

      iterator_base() = default;

      iterator_base(uint32_t itr, status itr_stat, const kv_index* index) : itr{itr}, itr_stat{itr_stat}, index{index} {}

      iterator_base(iterator_base&& other) :
         itr(std::exchange(other.itr, 0)),
         itr_stat(std::move(other.itr_stat)),
         index(std::move(other.index))
      {}

      ~iterator_base() {
         if (itr) {
            internal_use_do_not_use::kv_it_destroy(itr);
         }
      }

      iterator_base& operator=(iterator_base&& other) {
         if (itr) {
            internal_use_do_not_use::kv_it_destroy(itr);
         }
         itr = std::exchange(other.itr, 0);
         itr_stat = std::move(other.itr_stat);
         index = std::move(other.index);
         return *this;
      }

      bool good()const { return itr_stat != status::iterator_end; }

      /**
       * Returns the value that the iterator points to.
       * @ingroup keyvalue
       *
       * @return The value that the iterator points to.
       */
     void value(void* val, void (*deserialize)(void*, const void*, std::size_t)) const {
         using namespace detail;

         eosio::check(itr_stat != status::iterator_end, "Cannot read end iterator");

         uint32_t value_size;
         uint32_t actual_value_size;
         uint32_t actual_data_size;
         uint32_t offset = 0;

         // call once to get the value_size
         internal_use_do_not_use::kv_it_value(itr, 0, (char*)nullptr, 0, value_size);

         auto free_memory = [value_size](char* buf) { if (detail::max_stack_buffer_size < value_size) free(buf);};
         std::unique_ptr<char, decltype(free_memory)> buffer( (char*)(detail::max_stack_buffer_size < value_size ? malloc(value_size) : alloca(value_size)), free_memory);

         auto stat = internal_use_do_not_use::kv_it_value(itr, offset, buffer.get(), value_size, actual_value_size);

         eosio::check(static_cast<status>(stat) == status::iterator_ok, "Error reading value");

         bool is_primary = index->index_name == index->tbl->primary_index_name;
         if (!is_primary) {
            auto success = internal_use_do_not_use::kv_get(index->contract_name.value, buffer.get(), actual_value_size, actual_data_size);
            eosio::check(success, "failure getting primary key in `value()`");

            auto free_memory = [actual_data_size](char* buf) { if (detail::max_stack_buffer_size < actual_data_size) free(buf);};
            std::unique_ptr<char, decltype(free_memory)> pk_buffer( (char*)(detail::max_stack_buffer_size < actual_data_size ? malloc(actual_data_size) : alloca(actual_data_size)), free_memory);

            internal_use_do_not_use::kv_get_data(0, pk_buffer.get(), actual_data_size);

            deserialize(val, pk_buffer.get(), actual_data_size);
         } else {
            deserialize(val, buffer.get(), actual_value_size);
         }
      }

      full_key key() const {
         uint32_t actual_value_size;
         uint32_t value_size;

         // call once to get the value size
         internal_use_do_not_use::kv_it_key(itr, 0, (char*)nullptr, 0, value_size);

         auto free_memory = [value_size](char* buf) { if (detail::max_stack_buffer_size < value_size) free(buf);};
         std::unique_ptr<char, decltype(free_memory)> buffer( (char*)(detail::max_stack_buffer_size < value_size ? malloc(value_size) : alloca(value_size)), free_memory);

         auto stat = internal_use_do_not_use::kv_it_key(itr, 0, buffer.get(), value_size, actual_value_size);

         eosio::check(static_cast<status>(stat) == status::iterator_ok, "Error getting key");

         return full_key{buffer.get(), actual_value_size};
      }

   protected:
      uint32_t itr;
      status itr_stat;

      const kv_index* index;

      int compare(const iterator_base& b) const {
         bool a_is_end = !itr || itr_stat == status::iterator_end;
         bool b_is_end = !b.itr || b.itr_stat == status::iterator_end;
         if (a_is_end && b_is_end) {
            return 0;
         } else if (a_is_end && b.itr) {
            return 1;
         } else if (itr && b_is_end) {
            return -1;
         } else {
            return internal_use_do_not_use::kv_it_compare(itr, b.itr);
         }
      }
   };

}

/**
 * @defgroup keyvalue Key Value Table
 * @ingroup contracts
 *
 * @brief Defines an EOSIO Key Value Table
 * @details EOSIO Key Value API provides a C++ interface to the EOSIO Key Value database.
 * Key Value Tables require 1 primary index, of any type that can be serialized to a binary representation.
 * Key Value Tables support 0 or more secondary index, of any type that can be serialized to a binary representation.
 * Indexes must be a member variable or a member function.
 *
 * @tparam T         - the type of the data stored as the value of the table
  */
template<typename T>
class [[ deprecated ]] kv_table : kv_detail::kv_table_base {

   using kv_index = kv_detail::kv_index;

   class base_iterator : public kv_detail::iterator_base {
   public:
      using iterator_base::iterator_base;
      /**
       * Returns the value that the iterator points to.
       * @ingroup keyvalue
       *
       * @return The value that the iterator points to.
       */
      T value() const {
         T val;
         iterator_base::value(&val, &kv_table::deserialize_fun);
         return val;
      }
   };

   class iterator : public base_iterator {
      using base_iterator::itr;
      using base_iterator::itr_stat;
      using base_iterator::index;

   public:
      using status = typename base_iterator::status;

      iterator() = default;

      iterator(uint32_t itr, status itr_stat, const kv_index* index) : base_iterator{itr, itr_stat, index} {}

      iterator(iterator&& other) : base_iterator{std::move(other)} {}

      iterator& operator=(iterator&& other) {
         if (itr) {
            internal_use_do_not_use::kv_it_destroy(itr);
         }
         itr = std::exchange(other.itr, 0);
         itr_stat = std::move(other.itr_stat);
         index = std::move(other.index);
         return *this;
      }

      iterator& operator++() {
         eosio::check(itr_stat != status::iterator_end, "cannot increment end iterator");
         itr_stat = static_cast<status>(internal_use_do_not_use::kv_it_next(itr));
         return *this;
      }

      iterator& operator--() {
         if (!itr) {
            itr = internal_use_do_not_use::kv_it_create(index->contract_name.value, index->prefix.data(), index->prefix.size());
         }
         itr_stat = static_cast<status>(internal_use_do_not_use::kv_it_prev(itr));
         eosio::check(itr_stat != status::iterator_end, "decremented past the beginning");
         return *this;
      }

      int32_t key_compare(const full_key& kt) const {
         if (itr == 0 || itr_stat == status::iterator_end) {
            return 1;
         } else {
            return internal_use_do_not_use::kv_it_key_compare(itr, kt.data(), kt.size());
         }
      }

      bool operator==(const iterator& b) const {
         return base_iterator::compare(b) == 0;
      }

      bool operator!=(const iterator& b) const {
         return base_iterator::compare(b) != 0;
      }

      bool operator<(const iterator& b) const {
         return base_iterator::compare(b) < 0;
      }

      bool operator<=(const iterator& b) const {
         return base_iterator::compare(b) <= 0;
      }

      bool operator>(const iterator& b) const {
         return base_iterator::compare(b) > 0;
      }

      bool operator>=(const iterator& b) const {
         return base_iterator::compare(b) >= 0;
      }
   };

   class reverse_iterator : public base_iterator {
      using base_iterator::itr;
      using base_iterator::itr_stat;
      using base_iterator::index;

   public:
      using status = typename base_iterator::status;

      reverse_iterator() = default;

      reverse_iterator(uint32_t itr, status itr_stat, const kv_index* index) : base_iterator{itr, itr_stat, index} {}

      reverse_iterator(reverse_iterator&& other) : base_iterator{std::move(other)} {}

      reverse_iterator& operator=(reverse_iterator&& other) {
         if (itr) {
            internal_use_do_not_use::kv_it_destroy(itr);
         }
         itr = std::exchange(other.itr, 0);
         itr_stat = std::move(other.itr_stat);
         index = std::move(other.index);
         return *this;
      }

      reverse_iterator& operator++() {
         eosio::check(itr_stat != status::iterator_end, "incremented past the end");
         itr_stat = static_cast<status>(internal_use_do_not_use::kv_it_prev(itr));
         return *this;
      }

      reverse_iterator& operator--() {
         if (!itr) {
            itr = internal_use_do_not_use::kv_it_create(index->contract_name.value, index->prefix.data(), index->prefix.size());
            itr_stat = static_cast<status>(internal_use_do_not_use::kv_it_lower_bound(itr, "", 0));
         }
         itr_stat = static_cast<status>(internal_use_do_not_use::kv_it_next(itr));
         eosio::check(itr_stat != status::iterator_end, "decremented past the beginning");
         return *this;
      }

      int32_t key_compare(const full_key& kt) const {
         if (itr == 0 || itr_stat == status::iterator_end) {
            return 1;
         } else {
            return internal_use_do_not_use::kv_it_key_compare(itr, kt.data(), kt.size());
         }
      }

      bool operator==(const reverse_iterator& b) const {
         return base_iterator::compare(b) == 0;
      }

      bool operator!=(const reverse_iterator& b) const {
         return base_iterator::compare(b) != 0;
      }

      bool operator<(const reverse_iterator& b) const {
         return base_iterator::compare(b) < 0;
      }

      bool operator<=(const reverse_iterator& b) const {
         return base_iterator::compare(b) <= 0;
      }

      bool operator>(const reverse_iterator& b) const {
         return base_iterator::compare(b) > 0;
      }

      bool operator>=(const reverse_iterator& b) const {
         return base_iterator::compare(b) >= 0;
      }
   };

public:

   using iterator = kv_table::iterator;
   using value_type = T;

   /**
    * @ingroup keyvalue
    *
    * @brief Defines an index on an EOSIO Key Value Table
    * @details A Key Value Index allows a user of the table to search based on a given field.
    * The only restrictions on that field are that it is serializable to a binary representation sortable by the KV intrinsics.
    * Convenience functions exist to handle most of the primitive types as well as some more complex types, and are
    * used automatically where possible.
    *
    * @tparam K - The type of the key used in the index.
    */
   template <typename K>
   class index : public kv_index {
   public:
      using iterator = kv_table::iterator;
      using kv_table<T>::kv_index::tbl;
      using kv_table<T>::kv_index::table_name;
      using kv_table<T>::kv_index::contract_name;
      using kv_table<T>::kv_index::index_name;
      using kv_table<T>::kv_index::prefix;

      template <typename KF>
      index(eosio::name::raw name, KF&& kf) : kv_index{name, kf, (T*)nullptr} {
         static_assert(std::is_same_v<K, std::remove_cv_t<std::decay_t<decltype(std::invoke(kf, std::declval<const T*>()))>>>,
               "Make sure the variable/function passed to the constructor returns the same type as the template parameter.");
      }

      /**
       * Search for an existing object in a table by the index, using the given key.
       * @ingroup keyvalue
       *
       * @param key - The key to search for.
       * @return An iterator to the found object OR the `end` iterator if the given key was not found.
       */
      iterator find(const K& key) const {
         full_key t_key{prefix, make_key(key)};

         return find(t_key);
      }

      iterator find(const partial_key& key) const {
         return find(full_key(prefix, key));
      }

      iterator find(const full_key& key) const {
         uint32_t itr = internal_use_do_not_use::kv_it_create(contract_name.value, prefix.data(), prefix.size());
         int32_t itr_stat = internal_use_do_not_use::kv_it_lower_bound(itr, key.data(), key.size());

         auto cmp = internal_use_do_not_use::kv_it_key_compare(itr, key.data(), key.size());

         if (cmp != 0) {
            internal_use_do_not_use::kv_it_destroy(itr);
            return end();
         }

         return {itr, static_cast<typename iterator::status>(itr_stat), this};
      }

      /**
       * Check if a given key exists in the index.
       * @ingroup keyvalue
       *
       * @param key - The key to check for.
       * @return If the key exists or not.
       */
      bool exists(const K& key) const {
         full_key t_key{prefix, make_key(key)};
         return exists(t_key);
      }

      bool exists(const full_key& key) const {
         uint32_t value_size;
         return internal_use_do_not_use::kv_get(contract_name.value, key.data(), key.size(), value_size);
      }

      /**
       * Get the value for an existing object in a table by the index, using the given key.
       * @ingroup keyvalue
       *
       * @param key - The key to search for.
       * @return The value corresponding to the key.
       */
      T operator[](const K& key) const {
         return operator[](make_key(key));
      }

      T operator[](const partial_key& key) const {
         return operator[](full_key(prefix, key));
      }

      T operator[](const full_key& key) const {
         auto opt = get(key);
         eosio::check(opt.has_value(), __FILE_NAME__ ":" BOOST_PP_STRINGIZE(__LINE__) " Key not found in `[]`");
         return *opt;
      }

      /**
       * Get the value for an existing object in a table by the index, using the given key.
       * @ingroup keyvalue
       *
       * @param key - The key to search for.
       * @return A std::optional of the value corresponding to the key.
       */
      std::optional<T> get(const K& key) const {
         return get(make_key(key));
      }

      std::optional<T> get(const partial_key& k ) const {
         return get(full_key(prefix, k));
      }

      std::optional<T> get(const full_key& k ) const {
         std::optional<T> ret_val;
         kv_index::get(k, &ret_val, &deserialize_optional_fun);
         return ret_val;
      }

      /**
       * Returns an iterator to the object with the lowest key (by this index) in the table.
       * @ingroup keyvalue
       *
       * @return An iterator to the object with the lowest key (by this index) in the table.
       */
      iterator begin() const {
         uint32_t itr = internal_use_do_not_use::kv_it_create(contract_name.value, prefix.data(), prefix.size());
         int32_t itr_stat = internal_use_do_not_use::kv_it_lower_bound(itr, "", 0);

         return {itr, static_cast<typename iterator::status>(itr_stat), this};
      }

      /**
       * Returns an iterator pointing past the end. It does not point to any element, therefore `value` should not be called on it.
       * @ingroup keyvalue
       *
       * @return An iterator pointing past the end.
       */
      iterator end() const {
         return {0, iterator::status::iterator_end, this};
      }

      /**
       * Returns a reverse iterator to the object with the highest key (by this index) in the table.
       * @ingroup keyvalue
       *
       * @return A reverse iterator to the object with the highest key (by this index) in the table.
       */
      reverse_iterator rbegin() const {
         uint32_t itr = internal_use_do_not_use::kv_it_create(contract_name.value, prefix.data(), prefix.size());
         int32_t itr_stat = internal_use_do_not_use::kv_it_prev(itr);

         return {itr, static_cast<typename iterator::status>(itr_stat), this};
      }

      /**
       * Returns a reverse iterator pointing past the beginning. It does not point to any element, therefore `value` should not be called on it.
       * @ingroup keyvalue
       *
       * @return A reverse iterator pointing past the beginning.
       */
      reverse_iterator rend() const {
         return {0, iterator::status::iterator_end, this};
      }

      /**
       * Returns an iterator pointing to the element with the lowest key greater than or equal to the given key.
       * @ingroup keyvalue
       *
       * @return An iterator pointing to the element with the lowest key greater than or equal to the given key.
       */
      iterator lower_bound(const K& key) const {
         return lower_bound(make_key(key));
      }

      iterator lower_bound(const partial_key& k ) const {
         full_key key{prefix, k};
         return lower_bound(key);
      }

      iterator lower_bound(const full_key& key ) const {
         uint32_t itr = internal_use_do_not_use::kv_it_create(contract_name.value, prefix.data(), prefix.size());
         int32_t itr_stat = internal_use_do_not_use::kv_it_lower_bound(itr, key.data(), key.size());

         return {itr, static_cast<typename iterator::status>(itr_stat), this};
      }

      /**
       * Returns an iterator pointing to the first element greater than the given key.
       * @ingroup keyvalue
       *
       * @return An iterator pointing to the first element greater than the given key.
       */
      iterator upper_bound(const K& key) const {
         return upper_bound(make_key(key));
      }

      iterator upper_bound(const partial_key& k ) const {
         full_key key{prefix, k};
         return upper_bound(key);
      }

      iterator upper_bound(const full_key& key) const {
         auto it = lower_bound(key);

         auto cmp = it.key_compare(key);
         if (cmp == 0) {
            ++it;
         }

         return it;
      }

      /**
       * Returns a vector of objects that fall between the specifed range. The range is inclusive, exclusive.
       * @ingroup keyvalue
       *
       * @param begin - The beginning of the range (inclusive).
       * @param end - The end of the range (exclusive).
       * @return A vector containing all the objects that fall between the range.
       */
      std::vector<T> range(const K& b, const K& e) const {
         return range(make_key(b), make_key(e));
      }

      std::vector<T> range(const partial_key& b_key, const partial_key& e_key) const {
         full_key b{prefix, b_key};
         full_key e{prefix, e_key};
         return range(b, e);
      }

      std::vector<T> range(const full_key& b_key, const full_key& e_key) const {
         std::vector<T> return_values;

         for(auto itr = lower_bound(b_key), end_itr = lower_bound(e_key); itr < end_itr; ++itr) {
            return_values.push_back(itr.value());
         }

         return return_values;
      }

      void setup() override {
         prefix = make_prefix(table_name, index_name);
      }
   };

   /**
    * @ingroup keyvalue
    * Puts a value into the table. If the value already exists, it updates the existing entry.
    * The key is determined from the defined primary index.
    * If the put attempts to store over an existing secondary index, the transaction will be aborted.
    *
    * @param value - The entry to be stored in the table.
    */
   void put(const T& value) {
      T old_value;
      kv_table_base::put(&value, &old_value, &get_size_fun, &deserialize_fun, &serialize_fun);
   }

   static void deserialize_optional_fun(void* value, const void* buffer, std::size_t buffer_size) {
      static_cast<std::optional<T>*>(value)->emplace();
      return kv_table::deserialize(**static_cast<std::optional<T>*>(value), buffer, buffer_size);
   }
   static void deserialize_fun(void* value, const void* buffer, std::size_t buffer_size) {
      return kv_table::deserialize(*static_cast<T*>(value), buffer, buffer_size);
   }
   static void serialize_fun(const void* value, void* buffer, std::size_t buffer_size) {
      return kv_table::serialize(*static_cast<const T*>(value), buffer, buffer_size);
   }
   static std::size_t get_size_fun(const void* value) {
      return kv_table::get_size(*static_cast<const T*>(value));
   }

   /**
    * Removes a value from the table.
    * @ingroup keyvalue
    *
    * @param key - The key of the value to be removed.
    */
   void erase(const T& value) {
      kv_table_base::erase(&value);
   }

protected:
   kv_table() = default;

   template <typename I>
   void setup_indices(I& index) {
      kv_index* idx = &index;
      idx->contract_name = contract_name;
      idx->table_name = table_name;
      idx->tbl = this;

      idx->setup();
      secondary_indices.push_back(idx);
   }

   template <typename PrimaryIndex, typename... SecondaryIndices>
   void init(eosio::name contract, eosio::name table, eosio::name db, PrimaryIndex& prim_index, SecondaryIndices&... indices) {
      validate_types(prim_index);
      (validate_types(indices), ...);

      contract_name = contract;
      table_name = table;
      db_name = db.value;

      primary_index = &prim_index;
      primary_index->contract_name = contract_name;
      primary_index->table_name = table_name;
      primary_index->tbl = this;

      primary_index->setup();

      primary_index_name = primary_index->index_name;

      (setup_indices(indices), ...);
   }

private:

   constexpr void validate_types() {}

   template <typename Type>
   constexpr void validate_types(Type& t) {
      constexpr bool is_kv_index = std::is_base_of_v<kv_index, std::decay_t<Type>>;
      static_assert(is_kv_index, "Incorrect type passed to init. Must be a reference to an index.");
   }

   template <typename V>
   static void serialize(const V& value, void* buffer, size_t size) {
      datastream<char*> ds((char*)buffer, size);
      unsigned_int i{0};
      ds << i;
      ds << value;
   }

   template <typename... Vs>
   static void serialize(const std::variant<Vs...>& value, void* buffer, size_t size) {
      datastream<char*> ds((char*)buffer, size);
      ds << value;
   }

   template <typename V>
   static void deserialize(V& value, const void* buffer, size_t size) {
      unsigned_int idx;
      datastream<const char*> ds((const char*)buffer, size);

      ds >> idx;
      eosio::check(idx==unsigned_int(0), "there was an error deserializing this value.");
      ds >> value;
   }

   template <typename... Vs>
   static void deserialize(std::variant<Vs...>& value, const void* buffer, size_t size) {
      datastream<const char*> ds((const char*)buffer, size);
      ds >> value;
   }

   template <typename V>
   static size_t get_size(const V& value) {
      auto size = pack_size(value);
      return size + 1;
   }

   template <typename... Vs>
   static size_t get_size(const std::variant<Vs...>& value) {
      auto size = pack_size(value);
      return size;
   }
};
} // eosio
