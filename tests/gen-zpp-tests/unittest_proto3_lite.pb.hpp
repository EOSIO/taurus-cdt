///
/// Generated from protoc with zpp-proto-plugin, DO NOT modify
///
#pragma once
#include "google/protobuf/unittest_import.pb.h"
#include <zpp_bits.h>
#include <eosio/reflection.hpp>
// @@protoc_insertion_point(includes)
namespace proto3_lite_unittest {
enum ForeignEnum : int {
   FOREIGN_ZERO = 0,
   FOREIGN_FOO = 4,
   FOREIGN_BAR = 5,
   FOREIGN_BAZ = 6
};

struct ForeignMessage {
   zpp::bits::vint32_t c = {};
   bool operator == (const ForeignMessage&) const = default;
   EOSIO_FRIEND_REFLECT(ForeignMessage, c)
}; // struct ForeignMessage

struct TestAllTypes {
   enum NestedEnum : int {
      ZERO = 0,
      FOO = 1,
      BAR = 2,
      BAZ = 3,
      NEG = -1
   };

   struct NestedMessage {
      zpp::bits::vint32_t bb = {};
      bool operator == (const NestedMessage&) const = default;
      EOSIO_FRIEND_REFLECT(NestedMessage, bb)
   }; // struct NestedMessage

   zpp::bits::vint32_t optional_int32 = {};
   zpp::bits::vint64_t optional_int64 = {};
   zpp::bits::vuint32_t optional_uint32 = {};
   zpp::bits::vuint64_t optional_uint64 = {};
   zpp::bits::vsint32_t optional_sint32 = {};
   zpp::bits::vsint64_t optional_sint64 = {};
   uint32_t optional_fixed32 = {};
   uint64_t optional_fixed64 = {};
   int32_t optional_sfixed32 = {};
   int64_t optional_sfixed64 = {};
   float optional_float = {};
   double optional_double = {};
   bool optional_bool = {};
   std::string optional_string = {};
   std::vector<char> optional_bytes = {};
   NestedMessage optional_nested_message = {};
   ::proto3_lite_unittest::ForeignMessage optional_foreign_message = {};
   ::protobuf_unittest_import::ImportMessage optional_import_message = {};
   NestedEnum optional_nested_enum = {};
   ::proto3_lite_unittest::ForeignEnum optional_foreign_enum = {};
   std::string optional_string_piece = {};
   std::string optional_cord = {};
   ::protobuf_unittest_import::PublicImportMessage optional_public_import_message = {};
   NestedMessage optional_lazy_message = {};
   std::vector<zpp::bits::vint32_t> repeated_int32 = {};
   std::vector<zpp::bits::vint64_t> repeated_int64 = {};
   std::vector<zpp::bits::vuint32_t> repeated_uint32 = {};
   std::vector<zpp::bits::vuint64_t> repeated_uint64 = {};
   std::vector<zpp::bits::vsint32_t> repeated_sint32 = {};
   std::vector<zpp::bits::vsint64_t> repeated_sint64 = {};
   std::vector<uint32_t> repeated_fixed32 = {};
   std::vector<uint64_t> repeated_fixed64 = {};
   std::vector<int32_t> repeated_sfixed32 = {};
   std::vector<int64_t> repeated_sfixed64 = {};
   std::vector<float> repeated_float = {};
   std::vector<double> repeated_double = {};
   std::vector<bool> repeated_bool = {};
   std::vector<std::string> repeated_string = {};
   std::vector<std::vector<char>> repeated_bytes = {};
   std::vector<NestedMessage> repeated_nested_message = {};
   std::vector<::proto3_lite_unittest::ForeignMessage> repeated_foreign_message = {};
   std::vector<::protobuf_unittest_import::ImportMessage> repeated_import_message = {};
   std::vector<NestedEnum> repeated_nested_enum = {};
   std::vector<::proto3_lite_unittest::ForeignEnum> repeated_foreign_enum = {};
   std::vector<std::string> repeated_string_piece = {};
   std::vector<std::string> repeated_cord = {};
   std::vector<NestedMessage> repeated_lazy_message = {};
   using pb_options = std::tuple<
      zpp::bits::pb_map<16,18>,
      zpp::bits::pb_map<17,19>,
      zpp::bits::pb_map<18,20>,
      zpp::bits::pb_map<19,21>,
      zpp::bits::pb_map<20,22>,
      zpp::bits::pb_map<21,24>,
      zpp::bits::pb_map<22,25>,
      zpp::bits::pb_map<23,26>,
      zpp::bits::pb_map<24,27>,
      zpp::bits::pb_map<25,31>,
      zpp::bits::pb_map<26,32>,
      zpp::bits::pb_map<27,33>,
      zpp::bits::pb_map<28,34>,
      zpp::bits::pb_map<29,35>,
      zpp::bits::pb_map<30,36>,
      zpp::bits::pb_map<31,37>,
      zpp::bits::pb_map<32,38>,
      zpp::bits::pb_map<33,39>,
      zpp::bits::pb_map<34,40>,
      zpp::bits::pb_map<35,41>,
      zpp::bits::pb_map<36,42>,
      zpp::bits::pb_map<37,43>,
      zpp::bits::pb_map<38,44>,
      zpp::bits::pb_map<39,45>,
      zpp::bits::pb_map<40,48>,
      zpp::bits::pb_map<41,49>,
      zpp::bits::pb_map<42,50>,
      zpp::bits::pb_map<43,51>,
      zpp::bits::pb_map<44,52>,
      zpp::bits::pb_map<45,54>,
      zpp::bits::pb_map<46,55>,
      zpp::bits::pb_map<47,57>>;
   bool operator == (const TestAllTypes&) const = default;
   EOSIO_FRIEND_REFLECT(TestAllTypes, optional_int32, optional_int64, optional_uint32, optional_uint64, optional_sint32, optional_sint64, optional_fixed32, optional_fixed64, optional_sfixed32, optional_sfixed64, optional_float, optional_double, optional_bool, optional_string, optional_bytes, optional_nested_message, optional_foreign_message, optional_import_message, optional_nested_enum, optional_foreign_enum, optional_string_piece, optional_cord, optional_public_import_message, optional_lazy_message, repeated_int32, repeated_int64, repeated_uint32, repeated_uint64, repeated_sint32, repeated_sint64, repeated_fixed32, repeated_fixed64, repeated_sfixed32, repeated_sfixed64, repeated_float, repeated_double, repeated_bool, repeated_string, repeated_bytes, repeated_nested_message, repeated_foreign_message, repeated_import_message, repeated_nested_enum, repeated_foreign_enum, repeated_string_piece, repeated_cord, repeated_lazy_message)
}; // struct TestAllTypes

struct TestPackedTypes {
   std::vector<zpp::bits::vint32_t> packed_int32 = {};
   std::vector<zpp::bits::vint64_t> packed_int64 = {};
   std::vector<zpp::bits::vuint32_t> packed_uint32 = {};
   std::vector<zpp::bits::vuint64_t> packed_uint64 = {};
   std::vector<zpp::bits::vsint32_t> packed_sint32 = {};
   std::vector<zpp::bits::vsint64_t> packed_sint64 = {};
   std::vector<uint32_t> packed_fixed32 = {};
   std::vector<uint64_t> packed_fixed64 = {};
   std::vector<int32_t> packed_sfixed32 = {};
   std::vector<int64_t> packed_sfixed64 = {};
   std::vector<float> packed_float = {};
   std::vector<double> packed_double = {};
   std::vector<bool> packed_bool = {};
   std::vector<::proto3_lite_unittest::ForeignEnum> packed_enum = {};
   using pb_options = std::tuple<
      zpp::bits::pb_map<1,90>,
      zpp::bits::pb_map<2,91>,
      zpp::bits::pb_map<3,92>,
      zpp::bits::pb_map<4,93>,
      zpp::bits::pb_map<5,94>,
      zpp::bits::pb_map<6,95>,
      zpp::bits::pb_map<7,96>,
      zpp::bits::pb_map<8,97>,
      zpp::bits::pb_map<9,98>,
      zpp::bits::pb_map<10,99>,
      zpp::bits::pb_map<11,100>,
      zpp::bits::pb_map<12,101>,
      zpp::bits::pb_map<13,102>,
      zpp::bits::pb_map<14,103>>;
   bool operator == (const TestPackedTypes&) const = default;
   EOSIO_FRIEND_REFLECT(TestPackedTypes, packed_int32, packed_int64, packed_uint32, packed_uint64, packed_sint32, packed_sint64, packed_fixed32, packed_fixed64, packed_sfixed32, packed_sfixed64, packed_float, packed_double, packed_bool, packed_enum)
}; // struct TestPackedTypes

struct NestedTestAllTypes {
   std::unique_ptr<::proto3_lite_unittest::NestedTestAllTypes> child = {};
   ::proto3_lite_unittest::TestAllTypes payload = {};
   bool operator == (const NestedTestAllTypes& other) const {
      ((child == other.child) || (*child == *other.child)) &&
      payload == other.payload
   };
   EOSIO_FRIEND_REFLECT(NestedTestAllTypes, child, payload)
}; // struct NestedTestAllTypes

struct TestEmptyMessage {
}; // struct TestEmptyMessage

} // namespace proto3_lite_unittest
