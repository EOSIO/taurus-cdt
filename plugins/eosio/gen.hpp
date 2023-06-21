#pragma once

#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/Expr.h>
#include <clang/Basic/Builtins.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/raw_ostream.h>
#include "utils.hpp"
#include "error_emitter.hpp"
#include <functional>
#include <vector>
#include <string>
#include <map>
#include <regex>
#include <utility>
#include <variant>
#include <iostream>
#include "clang_wrapper.hpp"

using namespace blanc;

namespace eosio { namespace cdt {

struct simple_ricardian_tokenizer {
   simple_ricardian_tokenizer( const std::string& src ) : source(src), index(0) {}
   int eat_ws(int i) {
      if (source[i] != ' ' && source[i] != '\n' && source[i] != '\r' && source[i] != '\t')
         return 0;
      for (; i < source.size(); i++) {
         if (source[i] != ' ' && source[i] != '\n' && source[i] != '\r' && source[i] != '\t')
            break;
      }
      return i - index;
   }

   bool check(const std::string& sub) {
      int i = index;
      int j = 0;
      for (; j < sub.size(); i++, j++) {
         i += eat_ws(i);
         if (sub[j] != source[i])
            return false;
      }
      index = i;
      index += eat_ws(index);
      return true;
   }

   bool is_decl(std::string type) {
      return check("<") && check("h1") && check("class") && check("=")
            && check('\"'+type+'\"') && check(">");
   }

   std::vector<std::string> get_decl(std::string type) {
      if (is_decl(type)) {
         int before, after;
         before = after = index;
         int ws = 0;
         for (; after < source.size(); after++) {
            if (source[after] == '<')
               break;
            if (source[after] != ' ' && source[after] != '\n' && source[after] != '\r' && source[after] != '\t')
               ws = 0;
            else
               ws++;
         }
         index = after;
         if (check("<") && check("/h1") && check(">"))
            return {source.substr(before, after-before-ws)};
      }
      return {};
   }
   std::string get_body(const std::string& type) {
      int i, before;
      i = before = index;
      int ws = 0;
      for (; i < source.size(); i++) {
         index = i;
         if (is_decl(type))
            break;
         if (source[i] != ' ' && source[i] != '\n' && source[i] != '\r' && source[i] != '\t')
            ws = 0;
         else
            ws++;
      }
      index = i;
      return source.substr(before, index-before-ws);
   }

   std::vector<std::pair<std::string, std::string>> parse(const std::string& type) {
      std::vector<std::pair<std::string, std::string>> ret;
      while (index < source.size()) {
         std::vector<std::string> decl = get_decl(type);
         if (!decl.empty()) {
            std::string body = get_body(type);
            ret.push_back(std::make_pair(decl[0], body));
         }
         else
            return {};
      }
      return ret;
   }

   std::string source;
   size_t      index;
};

struct generation_utils {
   std::vector<std::string> resource_dirs;
   std::string contract_name;
   inline static std::string parsed_contract_name = ""; // obtained by parsing methods/records
   bool suppress_ricardian_warnings;
   bool is_wasm = false;

   generation_utils() : resource_dirs({"./"}) {}
   generation_utils( const std::vector<std::string>& paths ) : resource_dirs(paths) {}

   static error_emitter& get_error_emitter() {
      static error_emitter ee;
      return ee;
   }

   inline void set_contract_name( const std::string& cn ) { contract_name = cn; }
   inline std::string get_contract_name()const { return contract_name; }
   static inline std::string get_parsed_contract_name() { return parsed_contract_name; }
   inline void set_resource_dirs( const std::vector<std::string>& rd ) {
      llvm::SmallString<PATH_MAX> cwd;
      auto has_real_path = llvm::sys::fs::real_path("./", cwd, true);
      if (!has_real_path)
         resource_dirs.push_back(cwd.str().str());
      for ( auto res : rd ) {
         llvm::SmallString<PATH_MAX> rp;
         auto has_real_path = llvm::sys::fs::real_path(res, rp, true);
         if (!has_real_path)
            resource_dirs.push_back(rp.str().str());
      }
   }

   template<typename T>
   static inline bool has_eosio_ricardian( const clang_wrapper::Decl<T>& decl ) {
      return decl.hasEosioRicardian();
   }

   template<typename T>
   static inline bool get_eosio_ricardian( const clang_wrapper::Decl<T>& decl ) {
      return decl.getEosioRicardianAttr()->getName();
   }

   template<typename T>
   static inline std::string get_action_name( const clang_wrapper::Decl<T>& decl ) {
      std::string action_name = "";
      auto tmp = decl.getEosioActionAttr()->getNameAsString();
      if (!tmp.empty())
         return tmp;
      return decl->getNameAsString();
   }

   template<typename T>
   static inline std::string get_action_name( T decl ) {
      auto _decl = clang_wrapper::wrap_decl(decl);
      return get_action_name(_decl);
   }

   template<typename T>
   static inline std::string get_notify_pair( const clang_wrapper::Decl<T>& decl ) {
      std::string notify_pair = "";
      auto tmp = decl.getEosioNotifyAttr()->getNameAsString();
      return tmp;
   }

   template<typename T>
   static inline std::string get_notify_pair( T decl ) {
      auto _decl = clang_wrapper::wrap_decl(decl);
      return get_notify_pair(_decl);
   }

   inline std::string get_rc_filename() {
      return contract_name+".contracts.md";
   }
   inline std::string get_clauses_filename() {
      return contract_name+".clauses.md";
   }

   inline std::string read_file( const std::string& fname ) {
      for ( auto res : resource_dirs ) {
         if ( llvm::sys::fs::exists( res + "/" + fname ) ) {
            int fd;
            llvm::sys::fs::file_status stat;
            llvm::sys::fs::openFileForRead(res+"/"+fname, fd);
            llvm::sys::fs::status(fd, stat);
            llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> mb =
               llvm::MemoryBuffer::getOpenFile(fd, fname, stat.getSize());
            if (mb)
               return mb.get()->getBuffer().str();
         }
      }
      return {};
   }

   inline void set_suppress_ricardian_warning(bool suppress_ricardian_warnings) {
      this->suppress_ricardian_warnings = suppress_ricardian_warnings;
   }

   inline std::string get_ricardian_clauses() {
      return read_file(get_clauses_filename());
   }
   inline std::string get_ricardian_contracts() {
      return read_file(get_rc_filename());
   }

   inline std::map<std::string, std::string> parse_contracts() {
      std::string contracts = get_ricardian_contracts();
      std::map<std::string, std::string> rcs;
      simple_ricardian_tokenizer srt(contracts);
      if (contracts.empty()) {
         if (!suppress_ricardian_warnings) {
            std::cout << "Warning, empty ricardian clause file\n";
         }
         return rcs;
      }

      auto parsed = srt.parse("contract");
      for (auto cl : parsed) {
         rcs.emplace(std::get<0>(cl), std::get<1>(cl));
      }
      return rcs;
   }

   inline std::vector<std::pair<std::string, std::string>> parse_clauses() {
      std::string clauses = get_ricardian_clauses();
      std::vector<std::pair<std::string, std::string>> clause_pairs;
      simple_ricardian_tokenizer srt(clauses);
      if (clauses.empty()) {
         if (!suppress_ricardian_warnings) {
            std::cout << "Warning, empty ricardian clause file\n";
         }
         return clause_pairs;
      }

      auto parsed = srt.parse("clause");
      for (auto cl : parsed) {
         clause_pairs.emplace_back(std::get<0>(cl), std::get<1>(cl));
      }
      return clause_pairs;
   }

   template<typename T>
   static inline bool is_eosio_contract( const clang_wrapper::Decl<T>& decl, const std::string& cn ) {
      std::string name = "";
      if (auto* r = llvm::dyn_cast<clang::CXXRecordDecl>(*decl)) {
         auto pd = decl.getParent();
         if (decl.isEosioContract()) {
            auto nm = decl.getEosioContractAttr()->getNameAsString();
            name = nm.empty() ? r->getName().str() : nm;
         } else if (pd && pd.isEosioContract()) {
            auto nm = pd.getEosioContractAttr()->getNameAsString();
            name = nm.empty() ? pd->getName().str() : nm;
         }
         parsed_contract_name = name;
         return cn == parsed_contract_name;
      } else if (auto* m = llvm::dyn_cast<clang::CXXMethodDecl>(*decl)) {
         if (decl.isEosioContract()) {
            name = decl.getEosioContractAttr()->getNameAsString();
         } else if (decl.getParent().isEosioContract()) {
            name = decl.getParent().getEosioContractAttr()->getNameAsString();
         }
         if (name.empty()) {
            name = decl.getParent()->getName().str();
         }
         parsed_contract_name = name;
         return cn == parsed_contract_name;
      }
      return false;
   }

   template<typename T>
   static inline bool is_eosio_contract( T decl, const std::string& cn ) {
      auto _decl = clang_wrapper::wrap_decl(decl);
      return is_eosio_contract(_decl, cn);
   }


   inline bool is_write_host_func( const clang::FunctionDecl *func_decl ) {
      static const std::set<std::string> write_host_funcs =
      {
         "set_resource_limits",
         "set_wasm_parameters_packed",
         "set_resource_limit",
         "set_proposed_producers",
         "set_proposed_producers_ex",
         "set_blockchain_parameters_packed",
         "set_parameters_packed",
         "set_kv_parameters_packed",
         "set_privileged",
         "db_store_i64",
         "db_update_i64",
         "db_remove_i64",
         "db_idx64_store",
         "db_idx64_update",
         "db_idx64_remove",
         "db_idx128_store",
         "db_idx128_update",
         "db_idx128_remove",
         "db_idx256_store",
         "db_idx256_update",
         "db_idx256_remove",
         "db_idx_double_store",
         "db_idx_double_update",
         "db_idx_double_remove",
         "db_idx_long_double_store",
         "db_idx_long_double_update",
         "db_idx_long_double_remove",
         "kv_erase",
         "kv_set",
         "send_deferred",
         "send_inline",
         "send_context_free_inline"
      };

      return write_host_funcs.count(func_decl->getNameInfo().getAsString()) >= 1;
   }

   inline bool is_deferred_transaction_func( const std::string& t ) {
      static const std::set<std::string> deferred_transaction_funcs =
      {
         "send_deferred",
      };
      return deferred_transaction_funcs.count(t) >= 1;
   }

   inline bool is_inline_action_func( const std::string& t ) {
      static const std::set<std::string> inline_action_funcs =
      {
         "send_inline",
         "send_context_free_inline"
      };
      return inline_action_funcs.count(t) >= 1;
   }
};
}} // ns eosio::cdt
