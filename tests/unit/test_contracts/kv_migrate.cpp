#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <vector>

using namespace eosio;
struct address_record_v1 {
    uint64_t    uid;
    uint64_t    addr;  
    uint64_t    state;
    std::vector<uint64_t> deposit;
};

struct address_table_v1 : eosio::kv::table<address_record_v1, "testtable1"_n>
{
    KV_NAMED_INDEX("uid"_n, uid);
    KV_NAMED_INDEX("addr"_n, addr);
    KV_NAMED_INDEX("state"_n, state);
    address_table_v1(name contract_name){
        init(contract_name, uid, addr, state);
    }
};
struct address_record_v2 {
    uint64_t    uid; 
    uint64_t    addr; 
    std::vector<uint64_t> withdraw;
};

struct address_table_v2 : eosio::kv::table<address_record_v2, "testtable2"_n>
{
    KV_NAMED_INDEX("uid"_n, uid);
    KV_NAMED_INDEX("addr"_n, addr);
    address_table_v2(name contract_name){
        init(contract_name, uid, addr);
    }
};
class [[eosio::contract]] kv_migrate : public contract {
    public:
        using contract::contract;

    void migration() {
        address_table_v1 from{"kvtest"_n};
        address_table_v2 to{"kvtest"_n};
        auto it  = std::begin(from.uid);
        auto end = std::end(from.uid);
        int  count = 0;
        while (it != end) {
            auto record = it.value();
            to.put({.uid  = record.uid,
                    .addr  = record.addr,
                    .withdraw = record.deposit});
            eosio::check(record.uid == count, "data corrupted in first index of existing table");
            eosio::check(record.addr == count + 1, "data corrupted in second index of existing table");
            ++ it;
            ++count;
            from.erase(record);
        }
        eosio::check(from.uid.begin() == from.uid.end(), "data cleanup failed");
    }

    void init(){
        address_table_v1 t{"kvtest"_n};
        for (size_t i = 0; i < 15; i++) {
            // this allocates memory from 73kb to 1.1mb
            // so the migration step will move both large (over 1mb) and small(below 1mb)
            // of data in each row of kv table.
            std::vector<uint64_t> dep(9235*(i+1), 7897); 
            t.put({.uid = i, .addr = i + 1, .state = i + 2, .deposit = dep});
        }
    }

    [[eosio::action]] 
    void setup() {
        init(); 
    }

    [[eosio::action]] 
    void setupf() {
        eosio::malloc_enable_free();
        init();
    }

    [[eosio::action]]
    void migratef() {
        eosio::malloc_enable_free();
        migration();
    }
    [[eosio::action]]
    void migrate() {
        migration();
    }
};