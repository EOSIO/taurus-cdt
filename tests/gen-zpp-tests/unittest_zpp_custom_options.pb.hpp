///
/// Generated from protoc with zpp-proto-plugin, DO NOT modify
///
#pragma once
#include <zpp_bits.h>
#include <eosio/reflection.hpp>
// @@protoc_insertion_point(includes)
namespace zpp_option_unittest {
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

struct TestAllType {
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

   std::optional<zpp::bits::vint32_t> optional_int32 = {};
   std::optional<zpp::bits::vint64_t> optional_int64 = {};
   std::optional<zpp::bits::vuint32_t> optional_uint32 = {};
   std::optional<zpp::bits::vuint64_t> optional_uint64 = {};
   std::optional<zpp::bits::vsint32_t> optional_sint32 = {};
   std::optional<zpp::bits::vsint64_t> optional_sint64 = {};
   std::optional<uint32_t> optional_fixed32 = {};
   std::optional<uint64_t> optional_fixed64 = {};
   std::optional<int32_t> optional_sfixed32 = {};
   std::optional<int64_t> optional_sfixed64 = {};
   std::optional<float> optional_float = {};
   std::optional<double> optional_double = {};
   std::optional<bool> optional_bool = {};
   std::optional<std::string> optional_string = {};
   std::optional<std::vector<char>> optional_bytes = {};
   std::optional<NestedMessage> optional_nested_message = {};
   std::optional<::zpp_option_unittest::ForeignMessage> optional_foreign_message = {};
   std::optional<NestedEnum> optional_nested_enum = {};
   std::optional<::zpp_option_unittest::ForeignEnum> optional_foreign_enum = {};
   eosio::name remap_type = {};
   std::optional<eosio::name> optional_remap_type = {};
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
   std::vector<::zpp_option_unittest::ForeignMessage> repeated_foreign_message = {};
   std::vector<NestedEnum> repeated_nested_enum = {};
   std::vector<::zpp_option_unittest::ForeignEnum> repeated_foreign_enum = {};
   using pb_options = std::tuple<
      zpp::bits::pb_map<16,18>,
      zpp::bits::pb_map<17,19>,
      zpp::bits::pb_map<18,21>,
      zpp::bits::pb_map<19,22>,
      zpp::bits::pb_map<20,25>,
      zpp::bits::pb_map<21,26>,
      zpp::bits::pb_map<22,31>,
      zpp::bits::pb_map<23,32>,
      zpp::bits::pb_map<24,33>,
      zpp::bits::pb_map<25,34>,
      zpp::bits::pb_map<26,35>,
      zpp::bits::pb_map<27,36>,
      zpp::bits::pb_map<28,37>,
      zpp::bits::pb_map<29,38>,
      zpp::bits::pb_map<30,39>,
      zpp::bits::pb_map<31,40>,
      zpp::bits::pb_map<32,41>,
      zpp::bits::pb_map<33,42>,
      zpp::bits::pb_map<34,43>,
      zpp::bits::pb_map<35,44>,
      zpp::bits::pb_map<36,45>,
      zpp::bits::pb_map<37,48>,
      zpp::bits::pb_map<38,49>,
      zpp::bits::pb_map<39,51>,
      zpp::bits::pb_map<40,52>>;
   bool operator == (const TestAllType&) const = default;
   EOSIO_FRIEND_REFLECT(TestAllType, optional_int32, optional_int64, optional_uint32, optional_uint64, optional_sint32, optional_sint64, optional_fixed32, optional_fixed64, optional_sfixed32, optional_sfixed64, optional_float, optional_double, optional_bool, optional_string, optional_bytes, optional_nested_message, optional_foreign_message, optional_nested_enum, optional_foreign_enum, remap_type, optional_remap_type, repeated_int32, repeated_int64, repeated_uint32, repeated_uint64, repeated_sint32, repeated_sint64, repeated_fixed32, repeated_fixed64, repeated_sfixed32, repeated_sfixed64, repeated_float, repeated_double, repeated_bool, repeated_string, repeated_bytes, repeated_nested_message, repeated_foreign_message, repeated_nested_enum, repeated_foreign_enum)
}; // struct TestAllType

} // namespace zpp_option_unittest
