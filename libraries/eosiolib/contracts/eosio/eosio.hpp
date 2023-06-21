/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#pragma once
#include <eosio/action.hpp>
#include <eosio/print.hpp>
#include <eosio/key_value.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/dispatcher.hpp>
#include <eosio/contract.hpp>
#include <eosio/map.hpp>
#include <eosio/kv.hpp>

#ifdef __wasm32__
static_assert( sizeof(long) == sizeof(int), "unexpected size difference" );
#endif