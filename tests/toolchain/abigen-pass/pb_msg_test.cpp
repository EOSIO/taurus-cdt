#include <eosio/eosio.hpp>
#include <eosio/from_bin.hpp>
#include <eosio/to_bin.hpp>
#include <zpp_bits.h>

// @@protoc_insertion_point(includes)
namespace test {
struct ActData {
   zpp::bits::vint32_t id = {};
   zpp::bits::vint32_t type = {};
   std::string note = {};
   zpp::bits::vuint64_t account = {};
   std::optional<std::string> optional_string = {};
   std::optional<std::vector<char>> optional_bytes = {};
   using pb_options = std::tuple<
      zpp::bits::pb_map<4,5>,
      zpp::bits::pb_map<5,6>,
      zpp::bits::pb_map<6,7>>;
   bool operator == (const ActData&) const = default;
}; // struct ActData

struct ActResult {
   std::optional<zpp::bits::vint32_t> value = {};
   std::optional<std::string> opt_str_value = {};
   std::optional<std::vector<char>> opt_bytes_value = {};
   bool operator == (const ActResult&) const = default;
}; // struct ActResult

} // namespace test

namespace test {

class [[eosio::contract]] pb_msg_test : public eosio::contract {
 public:
   using contract::contract;

   [[eosio::action]] eosio::pb<ActResult> hi(const eosio::pb<ActData>& msg) { return ActResult{}; }
};
} // namespace test
