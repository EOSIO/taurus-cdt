# EOSIO-Taurus CDT (Contract Development Toolkit)

EOSIO-Taurs CDT is a toolchain for WebAssembly (WASM) and a set of tools to facilitate smart contract development for the [EOSIO-Taurus platform](https://github.com/EOSIO/taurus-node). In addition to being a general purpose WebAssembly toolchain, EOSIO-Taurus specific optimizations are available to support building EOSIO-Taurus smart contracts. This EOSIO-Taurus CDT repository is fork from the [eosio.cdt](https://github.com/EOSIO/eosio.cdt) repository.

EOSIO-Taurus CDT added features for supporting enterprise application requirements, simplifying smart contract developing, and improving the development efficiency.

- [Native smart contract code generation](./docs/05_features/40_native-tester-compilation.md), to enable single step smart contract debugging with debuggers.
- [Protocol Buffers support](./docs/05_features/70_protocol_buffers.md), to support Protocol Buffers for data serialization/deserialization and data structure specification in ABIs.
- [Smart contract memory management support](./docs/05_features/80_memory_management.md), to support large scale smart contract actions.
- [Built as an LLVM plugin](./docs/05_features/50_cdt_as_llvm_plugin.md), to have the most currently available optimizations and analyses from LLVM.
- [Libraries support for standard ECDSA and RSA keys](./docs/05_features/60_crypto_algorithms.md), to support enterprise application integration.

## Building and Installation

Please follow the [building and installation doc](./docs/02_installation/index.md).

## License

[MIT](./LICENSE)

## Important

See [LICENSE](./LICENSE) for copyright and license terms.

All repositories and other materials are provided subject to the terms of this [IMPORTANT](https://github.com/EOSIO/taurus-node/blob/develop/IMPORTANT.md) notice and you must familiarize yourself with its terms.  The notice contains important information, limitations and restrictions relating to our software, publications, trademarks, third-party resources, and forward-looking statements.  By accessing any of our repositories and other materials, you accept and agree to the terms of the notice.
