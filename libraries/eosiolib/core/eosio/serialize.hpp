#pragma once

#include <boost/preprocessor/seq/for_each.hpp>
#include <eosio/reflection.hpp>
#include <utility>
#include <concepts>

#define EOSLIB_REFLECT_MEMBER_OP( r, STRUCT, elem ) \
  EOSIO_REFLECT_MEMBER(STRUCT, elem);

/**
 *  @defgroup serialize Serialize
 *  @ingroup core
 *  @brief Defines C++ API to serialize and deserialize object
 */

/**
 *  Defines serialization and deserialization for a class
 *
 *  @ingroup serialize
 *  @param TYPE - the class to have its serialization and deserialization defined
 *  @param MEMBERS - a sequence of member names.  (field1)(field2)(field3)
 */
#define EOSLIB_SERIALIZE( STRUCT, MEMBERS )                             \
  friend constexpr const char* get_type_name(STRUCT*) { return #STRUCT; } \
  template <typename F>                                                 \
  friend constexpr void eosio_for_each_field(STRUCT*, F f) {            \
    BOOST_PP_SEQ_FOR_EACH(EOSLIB_REFLECT_MEMBER_OP, ~, MEMBERS);        \
  }

/**
 *  Defines serialization and deserialization for a class which inherits from other classes that
 *  have their serialization and deserialization defined
 *
 *  @ingroup serialize
 *  @param TYPE - the class to have its serialization and deserialization defined
 *  @param BASE - a sequence of base class names (basea)(baseb)(basec)
 *  @param MEMBERS - a sequence of member names.  (field1)(field2)(field3)
 */

#define EOSLIB_SERIALIZE_DERIVED( STRUCT, BASE, MEMBERS )                \
  friend constexpr const char* get_type_name(STRUCT*) { return #STRUCT; } \
  template <typename F>                                                 \
  friend constexpr void eosio_for_each_field(STRUCT*, F f) {            \
    eosio_for_each_field((BASE*)nullptr, f);                            \
    BOOST_PP_SEQ_FOR_EACH(EOSLIB_REFLECT_MEMBER_OP, ~, MEMBERS);        \
  }


namespace eosio {
  template <typename T>
    struct pb : T {
    using pb_message_type = T;
    pb() = default;
    pb(const T& v) : T(v) {}
    pb(T&& v) : T(std::move(v)) {}
    pb(const pb<T>& v) = default;
    pb(pb<T>&& v) = default;

    pb<T>& operator = (const pb<T>& v) = default;
    pb<T>& operator = (pb<T>&& v) = default;

    template <typename ...Args>
    pb(Args&& ...args): T(std::forward<Args>(args)...) {}

    bool operator == (const pb<T>&) const = default;
  };

  template <typename T>
  const pb<T>& to_pb(const T& v) {
    return static_cast<const pb<T>&>(v);
  }

  template <typename T>
  T& from_pb(pb<T>& v) {
    return v;
  }

  template <typename T>
  const T& from_pb(const pb<T>& v) {
    return v;
  }

  template <typename T>
  T& from_pb(std::tuple<pb<T>>& v) {
    return std::get<0>(v);
  }

  template <typename T>
  const T& from_pb(const std::tuple<pb<T>>& v) {
    return std::get<0>(v);
  }
}