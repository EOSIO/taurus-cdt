#include <eosio/eosio.hpp>
#include <variant>
#include <string>
#include <vector>


using std::variant;
using std::string;
using std::vector;
using namespace eosio;

struct anyvar;
struct entry;

using any_object = vector<entry>;
using any_array  = vector<anyvar>;

struct anyvar
{
   using value_type = std::variant<int64_t,
                                   any_object,
                                   any_array>;

   template <typename T, typename = typename std::enable_if<std::is_constructible_v<value_type, T>>::type>
   anyvar(const T& v) : _value(v)
   {}

   anyvar() {}

private:
   value_type _value;

   template <typename S>
   friend void to_json(const anyvar& obj, S& stream);
   template <typename S>
   friend void to_bin(const anyvar& obj, S& stream)
   {
      return eosio::to_bin(obj._value, stream);
   }
   template <typename S>
   friend void from_bin(anyvar& obj, S& stream)
   {
      return eosio::from_bin(obj._value, stream);
   }
};

struct entry
{
   string key;
   anyvar value;
};
EOSIO_REFLECT(entry, key, value)

class[[eosio::contract("any_object_any_array")]] any_object_any_array : public contract
{
public:
   using contract::contract;

   struct [[eosio::table]] greeting {
      uint64_t id;
      any_object o;
      any_array a;
      uint64_t primary_key() const { return id; }
   };
   typedef multi_index<"greeting"_n, greeting> greeting_index;

   greeting_index testtab;

   [[eosio::action]] void find(name user) {
      testtab.find(user.value);
   }
};
