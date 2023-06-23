/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#pragma once
#include "reflect.hpp"
#include <eosio/abieos_name.hpp>

/// @cond IMPLEMENTATIONS

namespace eosio {
   namespace internal_use_do_not_use {
      extern "C" {
         __attribute__((import_name("printn"))) void printn(uint64_t);
      }
   }

   inline void print(name obj) {
      internal_use_do_not_use::printn(obj.value);
   }

}

using namespace eosio::literals;

/// @endcond
