#pragma once

#include <eosio/abieos_asset.hpp>
#include <eosio/print.hpp>

namespace eosio {

inline void print(asset obj) {
   print(obj.to_string());
}

inline void print(extended_asset obj) {
   print(obj.to_string());
}

}
