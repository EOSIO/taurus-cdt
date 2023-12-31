/**
 *  @file datastream.hpp
 *  @copyright defined in eos/LICENSE
 */
#pragma once
#include <eosio/check.hpp>
#include <eosio/varint.hpp>
#include <eosio/to_bin.hpp>
#include <eosio/from_bin.hpp>
#include <eosio/serialize.hpp>

#include <list>
#include <queue>
#include <vector>
#include <array>
#include <set>
#include <map>
#include <string>
#include <optional>
#include <variant>
#include <experimental/type_traits>

#include <boost/mp11/tuple.hpp>
#include <boost/pfr.hpp>
#include <zpp_bits.h>
#include <magic_enum.hpp>

namespace eosio {

/**
 * @defgroup datastream Data Stream
 * @ingroup core
 * @brief Defines data stream for reading and writing data in the form of bytes
 */

/**
 *  A data stream for reading and writing data in the form of bytes
 *
 *  @tparam T - Type of the datastream buffer
 */
template<typename T>
class datastream {
   public:
      /**
       * Construct a new datastream object
       *
       * @details Construct a new datastream object given the size of the buffer and start position of the buffer
       * @param start - The start position of the buffer
       * @param s - The size of the buffer
       */
      datastream( T start, size_t s )
      :_start(start),_pos(start),_end(start+s){}

      void check_available(size_t size) {
         check(size <= size_t(_end - _pos), convert_stream_error(stream_error::overrun));
      }

     /**
      *  Skips a specified number of bytes from this stream
      *
      *  @param s - The number of bytes to skip
      */
      inline void skip( size_t s ){
         _pos += s;
         check(_pos <= _end, convert_stream_error(stream_error::overrun));
      }

     /**
      *  Reads a specified number of bytes from the stream into a buffer
      *
      *  @param d - The pointer to the destination buffer
      *  @param s - the number of bytes to read
      *  @return true
      */
      inline bool read( char* d, size_t s) {
        eosio::check( size_t(_end - _pos) >= (size_t)s, "read" );
        memcpy( d, _pos, s );
        _pos += s;
        return true;
      }

      void read_reuse_storage(const char*& result, size_t size) {
         check( size <= size_t(_end-_pos), convert_stream_error(stream_error::overrun) );
         result = _pos;
         _pos += size;
      }

     /**
      *  Writes a specified number of bytes into the stream from a buffer
      *
      *  @param d - The pointer to the source buffer
      *  @param s - The number of bytes to write
      *  @return true
      */
      inline bool write( const char* d, size_t s) {
        eosio::check( _end - _pos >= (int32_t)s, "write" );
        memcpy( (void*)_pos, d, s );
        _pos += s;
        return true;
      }

     /**
      *  Writes a byte into the stream
      *
      *  @brief Writes a byte into the stream
      *  @param c byte to write
      *  @return true
      */
      inline bool put(char c) {
        eosio::check( _pos < _end, "put" );
        *_pos = c;
        ++_pos;
        return true;
      }

     inline bool write(char c) { return put(c); }

     /**
      *  Reads a byte from the stream
      *
      *  @brief Reads a byte from the stream
      *  @param c - The reference to destination byte
      *  @return true
      */
      inline bool get( unsigned char& c) { return get( *(char*)&c); }

     /**
      *  Reads a byte from the stream
      *
      *  @brief Reads a byte from the stream
      *  @param c - The reference to destination byte
      *  @return true
      */
      inline bool get( char& c )
      {
        eosio::check( _pos < _end, "get" );
        c = *_pos;
        ++_pos;
        return true;
      }

     /**
      *  Retrieves the current position of the stream
      *
      *  @brief Retrieves the current position of the stream
      *  @return T - The current position of the stream
      */
      T pos()const { return _pos; }
      T get_pos()const { return _pos; }
      inline bool valid()const { return _pos <= _end && _pos >= _start;  }

     /**
      *  Sets the position within the current stream
      *
      *  @brief Sets the position within the current stream
      *  @param p - The offset relative to the origin
      *  @return true if p is within the range
      *  @return false if p is not within the rawnge
      */
      inline bool seekp(size_t p) { _pos = _start + p; return _pos <= _end; }

     /**
      *  Gets the position within the current stream
      *
      *  @brief Gets the position within the current stream
      *  @return p - The position within the current stream
      */
      inline size_t tellp()const      { return size_t(_pos - _start); }

     /**
      *  Returns the number of remaining bytes that can be read/skipped
      *
      *  @brief Returns the number of remaining bytes that can be read/skipped
      *  @return size_t - The number of remaining bytes
      */
      inline size_t remaining()const  { return _end - _pos; }
    private:
      /**
       * The start position of the buffer
       *
       * @brief The start position of the buffer
       */
      T _start;
      /**
       * The current position of the buffer
       *
       * @brief The current position of the buffer
       */
      T _pos;
      /**
       * The end position of the buffer
       *
       * @brief The end position of the buffer
       */
      T _end;
};

/**
 * Specialization of datastream used to help determine the final size of a serialized value
 */
template<>
class datastream<size_t> {
   public:
      /**
       * Construct a new specialized datastream object given the initial size
       *
       * @param init_size - The initial size
       */
     datastream( size_t init_size = 0):_size(init_size){}

     /**
      *  Increment the size by s. This behaves the same as write( const char* ,size_t s ).
      *
      *  @param s - The amount of size to increase
      *  @return true
      */
     inline bool skip( size_t s ) { _size += s; return true; }

     /**
      *  Increment the size by s. This behaves the same as skip( size_t s )
      *
      *  @param s - The amount of size to increase
      *  @return true
      */
     inline bool write( const char* ,size_t s )  { _size += s; return true; }

     /**
      *  Increment the size by one
      *
      *  @return true
      */
     inline bool put(char) { ++_size; return true; }

     inline bool write(char c) { return put(c); }

     /**
      *  Check validity. It's always valid
      *
      *  @return true
      */
     inline bool     valid()const                     { return true;              }

     /**
      * Set new size
      *
      * @brief Set new size
      * @param p - The new size
      * @return true
      */
     inline bool     seekp(size_t p)                  { _size = p;  return true;  }

     /**
      * Get the size
      *
      * @return size_t - The size
      */
     inline size_t   tellp()const                     { return _size;             }

     /**
      * Always returns 0
      *
      * @return size_t - 0
      */
     inline size_t   remaining()const                 { return 0;                 }
  private:
     /**
      * The size used to determine the final size of a serialized value.
      */
     size_t _size;
};

namespace _datastream_detail {
   /**
    * Check if type T is a pointer
    *
    * @brief Check if type T is a pointer
    * @tparam T - The type to be checked
    * @return true if T is a pointer
    * @return false otherwise
    */
   template<typename T>
   constexpr bool is_pointer() {
      return std::is_pointer<T>::value ||
             std::is_null_pointer<T>::value ||
             std::is_member_pointer<T>::value;
   }

   /**
    * Check if type T is a primitive type
    *
    * @brief Check if type T is a primitive type
    * @tparam T - The type to be checked
    * @return true if T is a primitive type
    * @return false otherwise
    */
   template<typename T>
   constexpr bool is_primitive() {
      return std::is_arithmetic<T>::value ||
             std::is_enum<T>::value;
   }

   /*
    * Check if type T is a specialization of datastream
    *
    * @brief Check if type T is a datastream
    * @tparam T - The type to be checked
    */
   template<typename T>
   struct is_datastream { static constexpr bool value = false; };
   template<typename T>
   struct is_datastream<datastream<T>> { static constexpr bool value = true; };

   namespace operator_detection {
      // These overloads will be ambiguous with the operator in eosio, unless
      // the user has provided an operator that is a better match.
      template<typename DataStream, typename T, std::enable_if_t<_datastream_detail::is_datastream<DataStream>::value>* = nullptr>
      DataStream& operator>>( DataStream& ds, T& v );
      template<typename DataStream, typename T, std::enable_if_t<_datastream_detail::is_datastream<DataStream>::value>* = nullptr>
      DataStream& operator<<( DataStream& ds, const T& v );
      template<typename DataStream, typename T>
      using require_specialized_right_shift = std::void_t<decltype(std::declval<DataStream&>() >> std::declval<T&>())>;
      template<typename DataStream, typename T>
      using require_specialized_left_shift = std::void_t<decltype(std::declval<DataStream&>() << std::declval<const T&>())>;
   }
   // Going through enable_if/is_detected is necessary for reasons that I don't entirely understand.
   template<typename DataStream, typename T>
   using require_specialized_right_shift = std::enable_if_t<std::experimental::is_detected_v<operator_detection::require_specialized_right_shift, DataStream, T>>;
   template<typename DataStream, typename T>
   using require_specialized_left_shift = std::enable_if_t<std::experimental::is_detected_v<operator_detection::require_specialized_left_shift, DataStream, T>>;
}

/**
 *  Serialize a class
 *
 *  @brief Serialize a class
 *  @param ds - The stream to write
 *  @param v - The value to serialize
 *  @tparam DataStream - Type of datastream
 *  @tparam T - Type of class
 *  @return DataStream& - Reference to the datastream
 */
template<typename DataStream, typename T, std::enable_if_t<_datastream_detail::is_datastream<DataStream>::value>* = nullptr>
DataStream& operator<<( DataStream& ds, const T& v ) {
   to_bin(v, ds);
   return ds;
}

// Backwards compatibility: allow user defined datastream operators to work with from_bin
template<typename DataStream, typename T, _datastream_detail::require_specialized_left_shift<datastream<DataStream>,T>* = nullptr>
void to_bin( const T& v, datastream<DataStream>& ds ) {
   ds << v;
}

/**
 *  Deserialize a class
 *
 *  @brief Deserialize a class
 *  @param ds - The stream to read
 *  @param v - The destination for deserialized value
 *  @tparam DataStream - Type of datastream
 *  @tparam T - Type of class
 *  @return DataStream& - Reference to the datastream
 */
template<typename DataStream, typename T, std::enable_if_t<_datastream_detail::is_datastream<DataStream>::value>* = nullptr>
DataStream& operator>>( DataStream& ds, T& v ) {
   if constexpr ( requires { from_pb(v); }) {
      zpp::bits::in in(std::span(ds.pos(), ds.remaining()), zpp::bits::size_varint{}, zpp::bits::protobuf{});
      check(std::errc{} == in(from_pb(v)), "protobuf deserialization failure");
   } else {
      from_bin(v, ds);
   }
   return ds;
}

template<typename DataStream, typename T, _datastream_detail::require_specialized_right_shift<datastream<DataStream>,T>* = nullptr>
void from_bin( T& v, datastream<DataStream>& ds ) {
   ds >> v;
}

/**
 * Unpack data inside a fixed size buffer as T
 *
 * @ingroup datastream
 * @brief Unpack data inside a fixed size buffer as T
 * @tparam T - Type of the unpacked data
 * @param buffer - Pointer to the buffer
 * @param len - Length of the buffer
 * @return T - The unpacked data
 */
template<typename T>
T unpack( const char* buffer, size_t len ) {
   T result;
   datastream<const char*> ds(buffer,len);
   ds >> result;
   return result;
}

/**
 * Unpack data inside a fixed size buffer as T
 *
 * @ingroup datastream
 * @tparam T - Type of the unpacked data
 * @param res - Variable to fill with the unpacking
 * @param buffer - Pointer to the buffer
 * @param len - Length of the buffer
 * @return T - The unpacked data
 */
template<typename T>
void unpack( T& res, const char* buffer, size_t len ) {
   datastream<const char*> ds(buffer,len);
   ds >> res;
}

/**
 * Unpack data inside a variable size buffer as T
 *
 * @ingroup datastream
 * @brief Unpack data inside a variable size buffer as T
 * @tparam T - Type of the unpacked data
 * @param bytes - Buffer
 * @return T - The unpacked data
 */
template<typename T>
T unpack( const std::vector<char>& bytes ) {
   return unpack<T>( bytes.data(), bytes.size() );
}

/**
 * Get the size of the packed data
 *
 * @ingroup datastream
 * @brief Get the size of the packed data
 * @tparam T - Type of the data to be packed
 * @param value - Data to be packed
 * @return size_t - Size of the packed data
 */
template<typename T>
size_t pack_size( const T& value ) {
  datastream<size_t> ps;
  ps << value;
  return ps.tellp();
}

/**
 * Get packed data
 *
 * @ingroup datastream
 * @brief Get packed data
 * @tparam T - Type of the data to be packed
 * @param value - Data to be packed
 * @return bytes - The packed data
 */
template<typename T>
std::vector<char> pack( const T& v )  {
   std::vector<char> result;
   if constexpr ( requires { from_pb(v); }) {
      zpp::bits::out  out(result, zpp::bits::size_varint{}, zpp::bits::protobuf{});
      check(std::errc{} == out(from_pb(v)), "protobuf serialization failure");
   } else {
      result.resize(pack_size(v));
      datastream<char*> ds( result.data(), result.size() );
      ds << v;      
   }
   return result;
}

template <typename T, zpp::bits::varint_encoding Encoding, typename S>
void from_json(zpp::bits::varint<T, Encoding>& obj, S& stream) {
   from_json(obj.value, stream);
}

template <typename T, zpp::bits::varint_encoding Encoding, typename S>
void to_json(zpp::bits::varint<T, Encoding> obj, S& stream) {
   to_json(obj.value, stream);
}

template <typename S>
void from_bin(zpp::bits::vuint32_t& obj, S& stream) {
   varuint32_from_bin(obj.value, stream);
}

template <typename S>
void to_bin(zpp::bits::vuint32_t obj, S& stream) {
   varuint32_to_bin(obj.value, stream);
}

template <typename S>
void from_bin(zpp::bits::vuint64_t& obj, S& stream) {
   varuint64_from_bin(obj.value, stream);
}

template <typename S>
void to_bin(zpp::bits::vuint64_t obj, S& stream) {
   varuint64_to_bin(obj.value, stream);
}

template <typename S>
void from_bin(zpp::bits::vint32_t& obj, S& stream) {
   uint32_t v;
   varuint32_from_bin(v, stream);
   obj.value = v;
}

template <typename S>
void to_bin(zpp::bits::vint32_t obj, S& stream) {
   varuint32_to_bin((uint32_t)obj.value, stream);
}

template <typename S>
void from_bin(zpp::bits::vint64_t& obj, S& stream) {
   uint64_t v;
   varuint64_from_bin(v, stream);
   obj.value = v;
}

template <typename S>
void to_bin(zpp::bits::vint64_t obj, S& stream) {
   varuint64_to_bin((uint64_t)obj.value, stream);
}

template <typename S>
void from_bin(zpp::bits::vsint32_t& obj, S& stream) {
   varint32_from_bin(obj.value, stream);
}

template <typename S>
void to_bin(zpp::bits::vsint32_t obj, S& stream) {
   varint32_to_bin(obj.value, stream);
}

template <typename E, typename S>
requires std::is_enum_v<E>
void from_json(E& obj, S& stream) {
   auto name = stream.get_string();
   auto v = magic_enum::enum_cast<E>(name);
   eosio::check(v.has_value(), std::string("invalid enumeration value ") + std::string{name});
   obj = v.value();
}

template <typename E, typename S>
requires std::is_enum_v<E>
void to_json(E obj, S& stream) {
   std::string_view name = magic_enum::enum_name(obj);
   to_json(name, stream);
}

}

