///
/// Generated from protoc with zpp-proto-plugin, DO NOT modify
///
#pragma once
#include <zpp_bits.h>
#include <eosio/reflection.hpp>
// @@protoc_insertion_point(includes)
namespace ms {
struct monster {
   enum Color : int {
      red = 0,
      blue = 1,
      green = 300
   };

   struct vec3 {
      zpp::bits::vint32_t x = {};
      zpp::bits::vuint32_t y = {};
      zpp::bits::vuint64_t z = {};
      bool operator == (const vec3&) const = default;
      EOSIO_FRIEND_REFLECT(vec3, x, y, z)
   }; // struct vec3

   struct weapon {
      std::string name = {};
      int32_t damage = {};
      bool operator == (const weapon&) const = default;
      EOSIO_FRIEND_REFLECT(weapon, name, damage)
   }; // struct weapon

   vec3 pos = {};
   int32_t mana = {};
   int64_t hp = {};
   std::string name = {};
   std::vector<char> inventory = {};
   Color color = {};
   std::vector<weapon> weapons = {};
   weapon equipped = {};
   std::vector<vec3> path = {};
   bool boss = {};
   bool operator == (const monster&) const = default;
   EOSIO_FRIEND_REFLECT(monster, pos, mana, hp, name, inventory, color, weapons, equipped, path, boss)
}; // struct monster

} // namespace ms

template <>
struct magic_enum::customize::enum_range<ms::monster::Color> {
   static constexpr int min = 0;
   static constexpr int max = 300;
};
