---
content_title: Protocol Buffers
---

## Overview

Serialization/Deserialization of data is an important part of many software projects. It defines the format that will be used to transmit/store the data as it flows through several parts of the system. On the original design of CDT there was only one way to serialize this information and that was using our custom library called abieos.

Taurus-CDT introduces a new way to serialize/deserialize this information that makes use of an industry-wide used library called Protocol Buffers (Protobuf).

## Why to use Protobuf

Why Protobuf has certain advantages:

* ID based field encoding to ensure on-chain data and interface stableness. On-chain data history are immutable and we must make sure the formats are strictly controlled with the enforce ID based encoding/decoding.

* Language neutral message format, and extensive high quality libraries for various languages. Less code to write, easier to maintain, faster to evolve. Microservices don’t have to struggle with the sometimes hardcoded serialization.

* Backwards compatibility. Easy to upgrade the message data structure, like removing/adding fields well supported. No need to rely heavily on manual code review for on-chain data upgrading compatibility, to avoid “corrupted” on-chain data.

* Fast. Generated native code from .proto to do serialization/deserialization; compact serialization format. As convenience to use as JSON, without the high deserialization performance cost.

## How to add protobuf support to smart contracts for action parameters/return data

1. Create your proto file with the definition of the data to be serialized, as an example:

```protobuf
syntax = "proto3";
package ms;

message monster {
  enum Color {
    red = 0;
    blue = 1;
    green = 300;
  }

  message vec3 {
    int32 x = 1;
    uint32 y = 2;
    uint64 z = 3;
  }

  message weapon {
    string name = 1;
    sfixed32 damage = 2;
  };

  vec3 pos = 1;
  sfixed32 mana = 2;
  sfixed64 hp = 3;
  string name = 4;
  bytes inventory = 5;
  Color color = 6;
  repeated weapon weapons = 7;
  weapon equipped = 8;
  repeated vec3 path = 9;
  bool boss = 10;
};
```

2. Configure CMake so that your smart contract can make use of the proto file:

```cmake
add_contract(hello hello hello_contract.cpp)

add_library(monster.pb INTERFACE)
target_add_protobuf(monster.pb monster.proto)

contract_use_protobuf(hello monster.pb)
```

3. After the above steps are configured, when building the contract, a hpp file will be generated containing the mapping of the message defined on the proto file to convenient c++ structs that can be used in the contracts. The library used to do this is called zpp-bits, please visit its github page if you need more information about it and the specific types it uses, as an example of the hpp file generated for the proto example above:

```cpp
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
```

4. You can use the above structure on the smart contract code, and its data will be deserialized automatically for action parameters data:

```cpp
...
[[eosio::action]] void createrole(const eosio::pb<Monster>& input);
...
```

## How to add protobuf support to kv tables

Protobuf can also be used as the serializing/deserializing format for kv tables, please stay tuned on more updates about this feature.

## Effects on abi generation

When compiling a contract that makes use of protobuf, a new section will be added to its abi containing the abieos-compatible definition of the protobuf types:

```json
"protobuf_types": {
        "file": [
            {
                "name": "monster/monster.proto",
                "package": "monster",
                "messageType": [
                    {
                        "name": "Monster",
                        "field": [
                            {
                                "name": "Color",
                                "number": 1,
                                "label": "LABEL_OPTIONAL",
                                "type": "TYPE_UINT32"
                            },
                            ...

```

The abieos library was thus modified to support serialization/deserialization of data using this abi definitions.

