///
/// Generated from protoc with zpp-proto-plugin, DO NOT modify
///
#pragma once
#include "google/protobuf/unittest_import_public.pb.h"
#include <zpp_bits.h>
#include <eosio/reflection.hpp>
// @@protoc_insertion_point(includes)
namespace protobuf_unittest_import {
enum ImportEnum : int {
   IMPORT_FOO = 0,
   IMPORT_BAR = 8,
   IMPORT_BAZ = 9
};

enum ImportEnumForMap : int {
   UNKNOWN = 0,
   FOO = 1,
   BAR = 2
};

struct ImportMessage {
   zpp::bits::vint32_t d = {};
   bool operator == (const ImportMessage&) const = default;
   EOSIO_FRIEND_REFLECT(ImportMessage, d)
}; // struct ImportMessage

} // namespace protobuf_unittest_import
