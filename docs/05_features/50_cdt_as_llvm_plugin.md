## Background

By version 1.8, CDT was implemented by customizing the LLVM source code. This presented a unique problem as upgrades to LLVM versions required CDT developers to understand and modify low-level source code in LLVM and then make the suitable changes to emit the required WASM code for Taurus. This was substantially difficult when the LLVM architecture and APIs changed, and it became a very large-scale effort to upgrade CDT. To overcome this issue, Taurus-CDT is re-architectured to make EOSIO related functions as Clang plugins such that LLVM upgrades can be independent of CDT and CDT bugs are much easier to fix than the original version. This allows CDT take advantage of the latest security and performance features in LLVM without sacrificing any extra efforts to do the CDT migrations. 

## Plugin Implementation

There are two Clang plugins added in Taurus-CDT,

- eosio_attr: EOSIO attributes are added in `custom_attr.cpp`, e.g. `eosio::contract`, `eosio::table`, and `eosio::read_only` etc.
- eosio_codegen: two frontend plugins `eosio_codegen` and `eosio_abigen` are implemented in `codegen.cpp` and `abigen.hpp` respectively

## Notes

To generate ABI, developers can use the script `$CDT_BUILD_DIR/tests/toolchain-tester/codegen.sh` that calls the `eosio-codegen` binary with the contract code. Further, option `-v` can be added to check the actual Clang commands for the contract compilation, for purposes such as CDT debugging.
