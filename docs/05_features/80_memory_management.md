---
content_title: Memory management
---

## Overview

At present, the EOSIO-taurus platform caps transaction execution times at 500ms as part of its block production cycle. All transactions that fall within this boundary are added to a block, whereas any that take longer get discarded. There are, however, circumstances where transactions like migration necessitate a longer execution duration.

In order to facilitate this feature, adjustments to the wasm memory model have been introduced to prevent transaction execution from depleting the wasm VM's memory. Whenever a contract action is submitted to nodeos, a wasm VM with 32MB of memory gets activated. The lifespan of this VM is predicated on the duration established in the `--max-transaction-time` parameter. As soon as this predefined time period lapses, the VM is terminated, resulting in the automatic liberation of memory.

Thanks to the long running transaction feature, the wasm VM can effectively oversee memory allocation, releasing it for subsequent use during prolonged transaction execution. This facilitates transactions to operate for a period exceeding the typical 500ms. This feature is especially valuable during high network traffic, allowing nodeos to process a greater number of transactions by postponing block production and incorporating more transactions within each block.

## Updates to CDT wasm memory model

WebAssembly (Wasm) virtual machine (VM) has a capped heap memory of 32MB. In order to prolong the Wasm VM's lifespan past the standard 500ms, alterations have been made to the memory model.

The said changes involve the introduction of a memory allocation method tailored to enhance performance. When allocating memory less than 1MB, a fixed-size memory is reserved from a small memory table. This allows quick and efficient allocation and deallocation operations with a consistent time complexity of O(1). The scope of the small memory table spans from 1<<6 to 1<<20.

In the event of memory allocations that surpass 1MB, a distinct mechanism is implemented using a large chunk table. This table preserves pointers to memory blocks and their corresponding sizes. If a memory request can be met with the available slots in the large chunk table, the allocation is made accordingly. Once the allocated memory becomes redundant, it's reintroduced to the table for potential future usage.

When the memory request is beyond the capabilities of the slots available in the table, a new request from the Wasm VM is fullfilled, with the resultant memory block stored in the table after its release.

It's important to note that when storing memory chunks in the table, if adjacent memory chunks are identified, they are merged. This merger ensures the storage of larger memory chunks, encouraging more efficient use of memory.

## How to activate memory cleanup in smart contract
Here is a template smart contract on how to activate memory clean up for smart contract action. Include <eosio/system.hpp> to access eosio::malloc_enable_free() and call it at the very beginning of the action so that malloc can keep track of allocated memory. 



```
#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
using namespace eosio;
class [[eosio::contract]] smrtcontract : public contract {
    public:
        using contract::contract;
        [[eosio::action]]
        void action() {
            eosio::malloc_enable_free();
            // rest of smart contract code ...
        }
};
```

# Nodeos config for long running transaction
In nodeos config transaction time should be set such that to let transactions to get executed for extended period of time. Limitation for chain CPU limit should be lifted.

```
--max-transaction-time=60000 // from 445 miliseconds to a minute or more
--override-chain-cpu-limits=true
```

