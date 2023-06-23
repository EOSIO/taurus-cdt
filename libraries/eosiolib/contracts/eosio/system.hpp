/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#pragma once
#include <eosio/time.hpp>
#include <eosio/check.hpp>
#include <eosio/fixed_bytes.hpp>
#include <eosio/name.hpp>
#include <eosio/serialize.hpp>
#include <eosio/datastream.hpp>
extern "C" {
   void eosio_malloc_enable_free();
}
namespace eosio {
  namespace internal_use_do_not_use {
    extern "C" {
      __attribute__((import_name("eosio_exit"), noreturn))
      void eosio_exit( int32_t code );

      struct  __attribute__((aligned (16))) capi_checksum256 {
         uint8_t hash[32];
      };

      __attribute__((import_name("is_feature_activated"))) bool is_feature_activated( const capi_checksum256* feature_digest );

      __attribute__((import_name("get_sender"))) uint64_t get_sender();

      __attribute__((import_name("push_event"))) void push_event(const void*, size_t);
    }
  }

  /**
   *  @addtogroup system System
   *  @ingroup contracts
   *  @brief Defines time related functions and eosio_exit
   */

   /**
    *  This method will abort execution of wasm without failing the contract.
    *  This is used to bypass all cleanup / destructors that would normally be
    *  called.
    *
       <html><p><b>
       WARNING: this method will immediately abort execution of wasm code that is on
                the stack and would be executed as the method normally returned.
                Problems can occur with write-caches, RAII, reference counting
                when this method aborts execution of wasm code immediately.
       </b></p></html>
    *  @ingroup system
    *
    *  @param code - the exit code
    *    Example:
    *
    *      @code
    *      eosio_exit(0);
    *      eosio_exit(1);
    *      eosio_exit(2);
    *      eosio_exit(3);
    *      @endcode
   */
   inline void eosio_exit( int32_t code ) {
     internal_use_do_not_use::eosio_exit(code);
   }

   /**
   *  Returns the time in microseconds from 1970 of the current block as a time_point
   *
   *  @ingroup system
   *  @return time in microseconds from 1970 of the current block as a time_point
   */
   time_point current_time_point();

   /**
   *  Returns the time in microseconds from 1970 of the current block as a block_timestamp
   *
   *  @ingroup system
   *  @return time in microseconds from 1970 of the current block as a block_timestamp
   */
   block_timestamp current_block_time();


   /**
    * Check if specified protocol feature has been activated
    *
    * @ingroup system
    * @param feature_digest - digest of the protocol feature
    * @return true if the specified protocol feature has been activated, false otherwise
    */
   inline bool is_feature_activated( const checksum256& feature_digest ) {
      auto feature_digest_data = feature_digest.extract_as_byte_array();
      return internal_use_do_not_use::is_feature_activated(
         reinterpret_cast<const internal_use_do_not_use::capi_checksum256*>( feature_digest_data.data() )
      );
   }

   /**
    * @brief enable access to free() memory.
    * 
    */
   inline void malloc_enable_free() { 
      #ifdef __wasm__
         eosio_malloc_enable_free();
      #endif
   }

   /**
    * Return name of account that sent current inline action
    *
    * @ingroup system
    * @return name of account that sent the current inline action (empty name if not called from inline action)
    */
   inline name get_sender() {
      return name( internal_use_do_not_use::get_sender() );
   }

   struct event_wrapper {
      eosio::name       tag;
      std::string       route;
      std::vector<char> data;

      bool operator == (const event_wrapper& other) const = default;

      EOSLIB_SERIALIZE(event_wrapper, (tag)(route)(data) )
   };

   /**
    * Send event data to host
    * 
    * @ingroup system
    * @param tag   - tag correspond to individual AMQP queue or exchange
    * @param route - route for the event
    * @param data  - payload of the event
    */
   inline void push_event(eosio::name tag, std::string route, const std::vector<char>& data) {
      auto packed = eosio::pack(event_wrapper{tag, route, data});
      return internal_use_do_not_use::push_event(packed.data(), packed.size());
   }

   inline void push_event(std::string route, const std::vector<char>& data) {
      return push_event(eosio::name{"taurus"}, route, data);
   }
}
