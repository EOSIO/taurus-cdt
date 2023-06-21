#pragma once
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendPluginRegistry.h>

#include <eosio/cdt_abi.hpp>
#include "gen.hpp"
#include "ppcallbacks.hpp"

#include <fstream>
#include <jsoncons/json.hpp>
#include <regex>

using namespace clang;
using namespace eosio::cdt;
using jsoncons::json;
using jsoncons::ojson;

extern std::string output;

namespace eosio { namespace cdt {
   class abigen : public generation_utils {
      std::set<std::string> checked_actions;
   public:
      using generation_utils::generation_utils;
      std::map<clang::QualType, std::string> parsed_typenames;
      std::unordered_map<std::string, clang::QualType> type_lookup;

      clang::SourceManager* source_manager = nullptr;

      bool no_abigen = false;

      static abigen& get() {
         static abigen ag;
         return ag;
      }

      void set_abi_version(int major, int minor) {
         _abi.version_major = major;
         _abi.version_minor = minor;
      }

      void add_typedef( const clang::QualType& t ) {
         abi_typedef ret;
         ret.new_type_name = translate_type( t );
         if (is_builtin_type(ret.new_type_name))
            return;
         auto td = get_type_alias(t);
         if (td.empty())
            return;
         
         ret.type = translate_type(td[0]);
         if (!is_builtin_type(ret.type)) {
            add_type(td[0]);
         }
         _abi.typedefs.insert(ret);
      }

      template<typename T>
      void add_wasm_action(const clang_wrapper::Decl<T>& decl, const std::string& handler) {
         wasm_action ret;
         ret.name = get_action_name(decl);
         ret.handler = handler;
         _abi.wasm_actions.insert(ret);
      }

      template<typename T>
      void add_wasm_notify(const clang_wrapper::Decl<T>& decl, const std::string& handler) {
         wasm_notify ret;
         auto str = get_notify_pair(decl);
         auto pos = str.find("::");
         if (pos == std::string::npos) {
            std::cerr << "Error, the argument of eosio::on_notify attribute should have separator '::'" << std::endl;
            throw;
         }
         ret.contract = str.substr(0, pos);
         ret.name = str.substr(pos+2);
         ret.handler = handler;
         _abi.wasm_notifies.insert(ret);
      }

      template<typename T>
      void add_wasm_entries(const clang_wrapper::Decl<T>& decl) {
         if (const auto* Attr = decl->template getAttr<clang::WebAssemblyExportNameAttr>()) {
            _abi.wasm_entries.insert(Attr->getExportName().str());
         }
      }

      void add_action( const clang::CXXRecordDecl* _decl ) {
         auto decl = clang_wrapper::wrap_decl(_decl);
         abi_action ret;
         auto action_name = decl.getEosioActionAttr()->getName();

         if (!checked_actions.insert(get_action_name(decl)).second)
            if (!suppress_ricardian_warnings)
               CDT_CHECK_WARN(!rcs[get_action_name(decl)].empty(), "abigen_warning", decl->getLocation(), "Action <"+get_action_name(decl)+"> does not have a ricardian contract");

         ret.ricardian_contract = rcs[get_action_name(decl)];

         if (action_name.empty()) {
            validate_name(decl->getName().str(), [&](auto s) { CDT_ERROR("abigen_error", decl->getLocation(), s); });
            ret.name = decl->getName().str();
         }
         else {
            validate_name( action_name.str(), [&](auto s) { CDT_ERROR("abigen_error", decl->getLocation(), s); });
            ret.name = action_name.str();
         }
         ret.type = decl->getName().str();
         _abi.actions.insert(ret);
      }

      std::string get_action_type(const clang::CXXMethodDecl* decl) {
         if (decl->getNumParams() == 1) {
            auto type = decl->getParamDecl(0)->getOriginalType();
            if (type->isReferenceType())
               type=type->getPointeeType();
            auto ctsd = get_template_specialization( type );
            if (ctsd) {
               auto name = ctsd->getQualifiedNameAsString();
               if (name == "eosio::pb") {
                  auto message_type = ctsd->getTemplateArgs()[0].getAsType()->getAsCXXRecordDecl()->getQualifiedNameAsString();
                  message_type = std::regex_replace(message_type, std::regex("::"), "."); // replace '::' -> '.'
                  return "protobuf::" + message_type; 
               }
            }
         }
         return decl->getNameAsString();
      }

      void add_action( const clang::CXXMethodDecl* _decl ) {
         auto decl = clang_wrapper::wrap_decl(_decl);
         abi_action ret;

         auto action_name = decl.getEosioActionAttr()->getName();

         if (!checked_actions.insert(get_action_name(decl)).second)
            if (!suppress_ricardian_warnings)
               CDT_CHECK_WARN(!rcs[get_action_name(decl)].empty(), "abigen_warning", decl->getLocation(), "Action <"+get_action_name(decl)+"> does not have a ricardian contract");

         ret.ricardian_contract = rcs[get_action_name(decl)];

         if (action_name.empty()) {
            validate_name( decl->getNameAsString(), [&](auto s) { CDT_ERROR("abigen_error", decl->getLocation(), s); } );
            ret.name = decl->getNameAsString();
         }
         else {
            validate_name( action_name.str(), [&](auto s) { CDT_ERROR("abigen_error", decl->getLocation(), s); } );
            ret.name = action_name.str();
         }
         ret.type = get_action_type(_decl);
         _abi.actions.insert(ret);
         // TODO
         if (translate_type(decl->getReturnType()) != "void") {
            add_type(decl->getReturnType());
            _abi.action_results.insert({get_action_name(decl), translate_type(decl->getReturnType())});
         }
      }

      void add_tuple(std::string name, const clang::ArrayRef<clang::TemplateArgument>& args) {
         abi_struct tup;
         tup.name = name;
         int i = 0;
         for (const auto& arg : args) {
            tup.fields.push_back( {"field_"+std::to_string(i++), add_then_translate_type(arg.getAsType())} );
         }      
         _abi.structs.insert(tup);
      }

      void add_pair(std::string name,  const clang::TemplateArgumentList& args) {
         abi_struct pair;
         pair.name = name;
         pair.fields.push_back( {"first", add_then_translate_type(args[0].getAsType())} );
         pair.fields.push_back( {"second", add_then_translate_type(args[1].getAsType())} );
         _abi.structs.insert(pair);
      }

      void add_map(std::string name, const clang::TemplateArgumentList& args) {
         abi_struct kv;
         kv.name = name.substr(0, name.length() - 2);
         kv.fields.push_back( {"key", add_then_translate_type(args[0].getAsType())} );
         kv.fields.push_back( {"value", add_then_translate_type(args[1].getAsType())} );
         _abi.structs.insert(kv);
      }

      void add_struct( const clang::CXXRecordDecl* decl, const std::string& rname="" ) {
         if (is_kv_internal(decl) || is_kv_table(decl)) {
            return;
         }

         abi_struct ret;
         if ( decl->getNumBases() == 1 ) {
            ret.base = get_type(decl->bases_begin()->getType());
            add_type(decl->bases_begin()->getType());
         }
         for ( auto field : decl->fields() ) {
            if ( field->getName() == "transaction_extensions") {
               abi_struct ext;
               ext.name = "extension";
               ext.fields.push_back( {"type", "uint16"} );
               ext.fields.push_back( {"data", "bytes"} );
               ret.fields.push_back( {"transaction_extensions", "extension[]"});
               _abi.structs.insert(ext);
            }
            else {
               auto t           = field->getType();
               auto attrs       = clang_wrapper::wrap_decl(field);
               auto* proto_type = attrs.getEosioTypeAttr();
               if (proto_type) {
                  if(auto it = type_lookup.find(proto_type->getNameAsString()); it != std::end(type_lookup)) {
                     t = it->second;
                  }
               }
               ret.fields.push_back({field->getName().str(), get_type(t)});
               add_type(t);
            }
         }
         if (!rname.empty())
            ret.name = rname;
         else
            ret.name = decl->getName().str();

         const auto res = _abi.structs.insert(ret);
      }

      void add_struct( const clang::CXXMethodDecl* decl ) {
         abi_struct new_struct;
         new_struct.name = decl->getNameAsString();
         for (auto param : decl->parameters() ) {
            auto param_type = param->getType().getNonReferenceType().getUnqualifiedType();
            new_struct.fields.push_back({param->getNameAsString(), get_type(param_type)});
            add_type(param_type);
         }
         _abi.structs.insert(new_struct);
      }

      std::string to_index_type( std::string t ) {
         return "i64";
      }

      void add_table( const clang::CXXRecordDecl* _decl ) {
         auto decl = clang_wrapper::wrap_decl(_decl);

         abi_table t;
         t.type = decl->getNameAsString();
         auto table_name = decl.getEosioTableAttr()->getName();
         if (!table_name.empty()) {
            validate_name( table_name.str(), [&](auto s) { CDT_ERROR("abigen_error", decl->getLocation(), s); } );
            t.name = table_name.str();
         }
         else {
            t.name = t.type;
         }
         ctables.insert(t);
      }

      void add_table( uint64_t name, const clang::CXXRecordDecl* _decl ) {
         auto decl = clang_wrapper::wrap_decl(_decl);
         if (!(decl.isEosioTable() && abigen::is_eosio_contract(decl, get_contract_name())))
            return;

         abi_table t;
         t.type = decl->getNameAsString();
         t.name = name_to_string(name);
         _abi.tables.insert(t);
      }

      void add_kv_map(const clang::ClassTemplateSpecializationDecl* decl) {
          abi_kv_table akt;
          const auto& first_arg  = decl->getTemplateArgs()[0];
          const auto& second_arg = decl->getTemplateArgs()[1];
          const auto& third_arg  = decl->getTemplateArgs()[2];
          const auto& fourth_arg = decl->getTemplateArgs()[3];

          if (first_arg.getKind() != clang::TemplateArgument::ArgKind::Integral)
             CDT_ERROR("abigen_error", decl->getLocation(), "first template argument to KV map is not an integral const");
          if (second_arg.getKind() != clang::TemplateArgument::ArgKind::Type)
             CDT_ERROR("abigen_error", decl->getLocation(), "second template argument to KV map is not a type");
          if (third_arg.getKind() != clang::TemplateArgument::ArgKind::Type)
             CDT_ERROR("abigen_error", decl->getLocation(), "third template argument to KV map is not a type");

          akt.name = name_to_string(first_arg.getAsIntegral().getExtValue());
          akt.type = translate_type(third_arg.getAsType()); // pick the "value" type
          add_type(third_arg.getAsType());
          akt.indices.push_back({name_to_string(fourth_arg.getAsIntegral().getExtValue()),
                                  translate_type(second_arg.getAsType())}); // set the "key" as the index type
          _abi.kv_tables.insert(akt);
      }

      static std::string get_expression_source(clang::Expr* expr) {
         std::string TypeS;
         llvm::raw_string_ostream s(TypeS); 
         clang::LangOptions LangOpts;
         LangOpts.CPlusPlus = true;
         clang::PrintingPolicy Policy(LangOpts);

         expr->printPretty(s, 0, Policy);
         return s.str();
      }

      static std::string get_eosio_name_from_literal(clang::UserDefinedLiteral* udl) {
         std::string index_name;
         const auto child = udl->getRawSubExprs()[0];
         if (const auto ice = dyn_cast<clang::ImplicitCastExpr>(child)) {
            if (const auto dre = dyn_cast<clang::DeclRefExpr>(ice->getSubExpr())) {
               if (const auto fd = dyn_cast<clang::FunctionDecl>(dre->getDecl())) {
                  const auto& templ_pack = fd->getTemplateSpecializationArgs()->get(1).pack_elements();
                  std::transform( templ_pack.begin(), templ_pack.end(), std::back_inserter(index_name), 
                     [](auto elem) { return (char)elem.getAsIntegral().getExtValue(); });
               }
            }
         }
         return index_name;
      }

      static bool is_deprecated(const clang::Decl* decl) {
         for (auto attr : decl->getAttrs()) {
            if (strcmp(attr->getSpelling(), "deprecated") == 0)
               return true;
         }
         return false;
      }

      void add_kv_table(const clang::CXXRecordDecl* const decl) {
         if (is_deprecated(decl))
            return;

         auto attrs = clang_wrapper::wrap_decl(decl);
         auto contract = attrs.getAttribute("eosio_contract");
         if (contract && *contract !=  get_contract_name())
            return;

         clang::QualType table_type;
         std::string templ_name;

         for (const auto& base : decl->bases()) {
            if (const auto templ_base = dyn_cast<clang::ClassTemplateSpecializationDecl>(base.getType()->getAsCXXRecordDecl())) {
               std::string base_name = templ_base->getNameAsString();
               if (base_name != "table") 
                  continue;
               const auto templ_val = templ_base->getTemplateArgs()[1].getAsIntegral().getExtValue();
               templ_name = name_to_string(templ_val);
               const auto& templ_type = templ_base->getTemplateArgs()[0];
               table_type = templ_type.getAsType();
               add_type(table_type);
            }
         }

         if (table_type.isNull()) {
            return;
         }

         abi_kv_table t;
         t.type = table_type->getAsCXXRecordDecl()->getNameAsString();
         t.name = templ_name;
         auto it = _abi.kv_tables.find(t);

         if (it != _abi.kv_tables.end() && it->type != t.type) {
            std::cerr << "Error: duplicated table name \"" << templ_name << "\"\n";
            exit(-1);
         }

         const auto get_string_name_from_kv_index = [&](clang::Expr* expr) {
            std::string index_name;
            if (const auto expr_wc = dyn_cast<clang::ExprWithCleanups>(expr)) {
               if (const auto cc_expr = dyn_cast<clang::CXXConstructExpr>(expr_wc->getSubExpr())) {
                  const auto arg = cc_expr->getArg(0)->IgnoreParenImpCasts();

                  if (const auto cfc_expr = dyn_cast<clang::CXXFunctionalCastExpr>(arg)) {
                     if (const auto il_expr = dyn_cast<clang::InitListExpr>(cfc_expr->getSubExpr())) {
                        const auto init = il_expr->getInit(0);
                        if (const auto udl = dyn_cast<clang::UserDefinedLiteral>(init)) {
                           return get_eosio_name_from_literal(udl);
                        }
                     }
                  }
                  else if (const auto udl = dyn_cast<clang::UserDefinedLiteral>(arg)) {
                     return get_eosio_name_from_literal(udl);
                  }
                  else if (auto x = dyn_cast<CXXMemberCallExpr>(arg)){
                     auto callee = dyn_cast<MemberExpr>(x->getCallee());
                     if (callee) {
                        auto udl_expr = dyn_cast<CastExpr>(*callee->child_begin())->IgnoreParenImpCasts();

                        if (auto udl = dyn_cast<clang::UserDefinedLiteral>(udl_expr))
                           return get_eosio_name_from_literal(udl);
                        else if (auto init_expr = dyn_cast<clang::InitListExpr>(*udl_expr->child_begin())) 
                           return get_eosio_name_from_literal( dyn_cast<clang::UserDefinedLiteral>(*init_expr->child_begin()));
                     } 
                  } 
               }    
            }
            
            return index_name;
         };

         for (const auto field : decl->fields()) {
            auto f = field->getInClassInitializer();
            if (f) {
               std::string index_name = get_string_name_from_kv_index(f);

               if (index_name.empty() && source_manager) {
                  std::cerr << "Error: Unable to get index name: " << f->getSourceRange().printToString(*source_manager) << "\n";
                  std::exit(-1);
               }

               std::string idx_type;
               const auto qt = field->getType();
               auto& args = get_template_specialization(qt)->getTemplateArgs();
               auto index_type = args[0];
               if (const auto elab_type = dyn_cast<clang::ElaboratedType>(index_type.getAsType().getTypePtr())) {
                  // This is the macro case
                  const auto decayed_type = elab_type->getNamedType();
                  if (const auto d = dyn_cast<clang::TemplateSpecializationType>(decayed_type)) {
                     const auto& decl_type = d->getArg(0);
                     if (const auto dcl_type = dyn_cast<clang::DecltypeType>(decl_type.getAsType())) {
                        idx_type = get_type_string_from_kv_index_macro_decltype(dcl_type);
                     } 
                  }
               } 
               if (idx_type.empty()) {
                  add_type(index_type.getAsType());
                  idx_type = get_type(index_type.getAsType());
               }

               t.indices.push_back({index_name, idx_type});
            }
         }

         _abi.kv_tables.insert(t);
      }

      void add_kv_caching_singleton(const clang::ClassTemplateSpecializationDecl* decl) {
         auto table_type = decl->getTemplateArgs()[0].getAsType();

         abi_kv_table t;
         t.type  = table_type->getAsCXXRecordDecl()->getNameAsString();
         t.name  = name_to_string(decl->getTemplateArgs()[1].getAsIntegral().getExtValue());
         auto it = _abi.kv_tables.find(t);

         if (it != _abi.kv_tables.end() && it->type != t.type) {
            std::cerr << "Error: duplicated table name \"" << t.name << "\"\n";
            exit(-1);
         }

         add_type(table_type);
         _abi.kv_tables.insert(t);
      }

      void add_clauses( const std::vector<std::pair<std::string, std::string>>& clauses ) {
         for ( auto clp : clauses ) {
            _abi.ricardian_clauses.push_back({std::get<0>(clp), std::get<1>(clp)});
         }
      }

      void add_contracts( const std::map<std::string, std::string>& rc ) {
         rcs = rc;
      }

      void add_variant(std::string name, const clang::ArrayRef<clang::TemplateArgument>& args) {
         abi_variant var;
         
         var.name = name;
         for (auto& arg: args) {
            var.types.push_back(add_then_translate_type(arg.getAsType()));
         }
         _abi.variants.insert(var);
      }

      void  add_enum(const clang::QualType& t ){
         abi_typedef enum_type;
         if(auto ptr = t.getTypePtr()->getAs<clang::EnumType>()){
            auto underlying = ptr->getDecl()->getIntegerType();
            enum_type.new_type_name = get_base_type_name( t );
            enum_type.type = translate_type(underlying);
            _abi.typedefs.insert(enum_type);
         }
      }

      void add_type( const clang::QualType& t) {
         if (evaluated.count(t.getTypePtr()))
            return;
         evaluated.insert(t.getTypePtr());
         auto type = get_ignored_type(t);
         auto n = translate_type(type);

         if (!is_builtin_type(n)) {
            auto ctsd = get_template_specialization(type);
            if (ctsd) {
               auto& args = ctsd->getTemplateArgs(); 
               auto name = ctsd->getQualifiedNameAsString();
               static const std::vector<std::string> one_arg_types = {"std::vector", "std::set", "std::deque", "std::list", "std::optional", "eosio::binary_extension", "eosio::ignore", "std::array"};

               if (std::find(one_arg_types.begin(), one_arg_types.end(), name) != one_arg_types.end()) {
                  auto arg = args[0].getAsType();
                  auto [canonical, non_canonical] = get_type_strings(arg);
                  if (canonical != non_canonical) {
                     _abi.typedefs.insert( {non_canonical, canonical} );
                  }
                  add_type(arg);
                  if ( strchr("]?$", n.back()) == 0) {
                     abi_typedef ret;
                     ret.new_type_name = n;
                     ret.type = get_type_strings(type).first;
                     _abi.typedefs.insert(ret); 
                  }
               }
               else if (name == "std::map")
                  add_map(n, args);
               else if (name == "std::pair")
                  add_pair(n, args);
               else if (name == "std::tuple") 
                  add_tuple(n, args[0].getPackAsArray());
               else if (name == "std::variant") {
                  add_variant(n, args[0].getPackAsArray());
               }
               else if (name != "eosio::pb" && name != "eosio::pb_msg") {
                  add_struct(type.getTypePtr()->getAsCXXRecordDecl(), n);
               } 
            } else if (is_aliasing(type)) {
               add_typedef(type);
            } else if (type.getTypePtr()->isRecordType()) {
               add_struct(type.getTypePtr()->getAsCXXRecordDecl(), n);
            } else if (type->isEnumeralType()) {
               add_enum(type);
            } 
         }
      }

      std::string add_then_translate_type(const clang::QualType& t) {
         add_type(t);
         return translate_type(t);
      }

      static inline bool is_ignorable( const clang::QualType& type ) {
         auto check = [&](const clang::Type* pt) {
         if (auto tst = llvm::dyn_cast<clang::TemplateSpecializationType>(pt))
            if (auto rt = llvm::dyn_cast<clang::RecordType>(tst->desugar())) {
               auto decl = rt->getDecl();
               return clang_wrapper::wrap_decl(decl).isEosioIgnore();
            }

            return false;
         };

         bool is_ignore = false;
         if ( auto pt = llvm::dyn_cast<clang::ElaboratedType>(type.getTypePtr()) )
            is_ignore = check(pt->desugar().getTypePtr());
         else
            is_ignore = check(type.getTypePtr());
         return is_ignore;
      }

      static inline clang::QualType get_ignored_type( const clang::QualType& type ) {
         if ( !is_ignorable(type) )
            return type;
         auto get = [&](const clang::Type* pt) {
            if (auto tst = llvm::dyn_cast<clang::TemplateSpecializationType>(pt))
               if (auto decl = llvm::dyn_cast<clang::RecordType>(tst->desugar())) {
                  auto _decl = decl->getDecl();
                  return clang_wrapper::wrap_decl(_decl).isEosioIgnore() ? tst->getArg(0).getAsType() : type;
               }
            return type;
         };

         const clang::Type* t;
         if ( auto pt = llvm::dyn_cast<clang::ElaboratedType>(type.getTypePtr()) ) {
            t = pt->desugar().getTypePtr();
         }
         else
            t = type.getTypePtr();
         return get(t);
      }

      inline const clang::ClassTemplateSpecializationDecl* get_template_specialization(clang::QualType type) {
         const clang::Type* pt = type.getTypePtr();
         if (auto et = llvm::dyn_cast<clang::ElaboratedType>(pt))
            pt = et->desugar().getTypePtr();
         if (auto tst = llvm::dyn_cast<clang::TemplateSpecializationType>(pt))
            pt = tst->desugar().getTypePtr();
         if (auto rt = llvm::dyn_cast<clang::RecordType>(pt)) {
            return llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(rt->getDecl());
         }
         return nullptr;
      }

      std::string get_base_type_name( const clang::QualType& type ) {
         clang::QualType newType = type;
         std::string type_str = newType.getNonReferenceType().getAsString();
         return get_base_type_name(type_str);
      }

      // remove the namespace and template parameters from a type name
      std::string get_base_type_name( const std::string& type_str ) {
         int i = type_str.length()-1;
         int template_nested = 0;
         for (; i > 0; i--) {
            if (type_str[i] == '>') ++template_nested;
            if (type_str[i] == '<') --template_nested;
            if (type_str[i] == ':' && template_nested == 0) {
               return type_str.substr(i+1);
            }
            if (type_str[i] == ' ' && template_nested == 0) {
               static const std::string valid_prefixs[] = {"long", "signed", "unsigned" };
               bool valid = false;
               for (auto &s : valid_prefixs) {
                  if (i >= s.length() && type_str.substr(i - s.length(), s.length()) == s &&
                     (i - s.length() == 0 || type_str[i - s.length() - 1] == ' ')) {
                     valid = true;
                     i -= s.length();
                     break;
                  }
               }
               if (valid) continue;
               return type_str.substr(i+1);
            }
         }
         return type_str;
      }

      std::string _translate_type( const clang::QualType& type ) {
         clang::QualType newType = type;
         std::string type_str = newType.getNonReferenceType().getAsString();

         const std::regex base_regex("^zpp::bits::v[us]+int64_t$");
         std::smatch base_match;
         if (std::regex_match(type_str, base_match, base_regex))
            throw std::runtime_error(type_str + " is only supported for protobuf encoding");          
         return _translate_type(get_base_type_name(type_str));
      }

      std::map<std::string, std::string> get_translation_table() {
         std::map<std::string, std::string> result = {
            {"unsigned __int128", "uint128"},
            {"__int128", "int128"},
            {"uint128_t", "uint128"},
            {"int128_t", "int128"},

            {"unsigned long long", "uint64"},
            {"long long", "int64"},
            {"uint64_t", "uint64"},
            {"int64_t", "int64"},

            {"uint32_t", "uint32"},
            {"int32_t", "int32"},

            {"unsigned short", "uint16"},
            {"short", "int16"},
            {"uint16_t", "uint16"},
            {"int16_t", "int16"},

            {"unsigned char", "uint8"},
            {"signed char", "int8"},
            {"char", "int8"},
            {"uint8_t", "uint8"},
            {"byte", "uint8"},
            {"int8_t", "int8"},
            {"_Bool", "bool"},

            {"float",  "float32"},

            {"double", "float64"},
            {"long double", "float128"},

            {"unsigned_int", "varuint32"},
            {"signed_int",   "varint32"},

            {"vint32_t", "varuint32"},
            {"vuint32_t", "varuint32"},
            {"vsint32_t", "varint32"},

            {"string", "string"},
            {"string_view", "string"},

            {"block_timestamp", "block_timestamp_type"},
            {"public_key", "public_key"},
            {"private_key", "private_key"},
            {"signature", "signature"},

            {"capi_name",    "name"},
            {"capi_public_key", "public_key"},
            {"capi_signature", "signature"},
            {"capi_checksum160", "checksum160"},
            {"capi_checksum256", "checksum256"},
            {"capi_checksum512", "checksum512"},
            {"fixed_bytes_20_uint32", "checksum160"},
            {"fixed_bytes_32_uint64", "checksum256"},
            {"fixed_bytes_64_uint64", "checksum512"}
         };

         if (is_wasm) {
            result.insert({{"unsigned long", "uint32"}, {"long", "int32"}, {"unsigned int", "uint32"}, {"int", "int32"}});
         } else {
            std::string num_bits_of_long = std::to_string(sizeof(long) * 8);
            std::string num_bits_of_int  = std::to_string(sizeof(int) * 8);

            result.insert({{"unsigned long", "uint" + num_bits_of_long},
                           {"long", "int" + num_bits_of_long},
                           {"unsigned int", "uint" + num_bits_of_int},
                           {"int", "int" + num_bits_of_int}});
         }

         return result;
      }

      std::string translate_builtin_type( const std::string& t ) {
         static std::map<std::string, std::string> translation_table = get_translation_table();
         auto ret = translation_table.find(t);
         if (ret == translation_table.end())
            return {};
         return ret->second;
      }

      std::string _translate_type( const std::string& t ) 
      {
         auto r = translate_builtin_type(t);
         return r.empty() ? t : r;
      }

      inline bool is_builtin_type( const clang::QualType& type ) {
         return translate_builtin_type(get_base_type_name(type.getNonReferenceType().getAsString())).size();
      }

      static std::string to_string(llvm::APSInt value) {
         llvm::SmallString<0> s;
         value.toString(s, 10);
         return static_cast<std::string>(s);
      }

      std::string get_template_arg_name(const clang::TemplateArgument& arg, bool canonical) {
         if (arg.getKind() == clang::TemplateArgument::ArgKind::Type) {
            auto itr = parsed_typenames.find(arg.getAsType().getCanonicalType());
            if (itr != parsed_typenames.end()) {
               return itr->second;
            }
            auto x = get_type_strings(arg.getAsType());
            return canonical ? x.first : x.second;
         } else if (arg.getKind() == clang::TemplateArgument::ArgKind::Integral) {
            return to_string(arg.getAsIntegral());
         } else if (arg.getKind() == clang::TemplateArgument::ArgKind::Expression) {
            if (auto ce = llvm::dyn_cast<clang::CastExpr>(arg.getAsExpr())) {
               auto il = llvm::dyn_cast<clang::IntegerLiteral>(ce->getSubExpr());
               return std::to_string(il->getValue().getLimitedValue());
            }
         }
         CDT_INTERNAL_ERROR("Tried to get a non-existent template argument");
         __builtin_unreachable();
      }

      /// @brief Get the string representations of a type
      ///
      /// Sometime, the string represention can have a non-canonical form so that it does not contains any special characters.
      /// For example, given the type `std::vector<int32_t>`, the canonical form of the type would be  `int32[]`. However, it 
      /// contains special characters which cannot be directly embeded inside another type to form the string represantation of a composite 
      /// type like `std::pair<std::vector<int32_t>, bool>`. In this case we need a non-canonical form which only contains the 
      /// alphanumeric characters. In the case of `std::vector<int32_t>`, the non-canonical form of the type would be 'int32_array'.
      /// Therefore, the result for `std::vector<int32_t>` would be { "int32[]", "int32_array"}. 
      ///
      /// For the types where no non-canonical representations exist, it returns the pair of their canonical representation; i.e. 
      /// the result for `int32_t` would be  `{ "int32", "int32"}`. 
      ///   
      /// @param type The type requested
      ///
      /// @return The pair of the canonical and the non-canonical  string representation. 
      ///
      inline std::pair<std::string, std::string> get_type_strings(const clang::QualType& type) {

         static const std::vector<std::string> sequence_types{"std::vector", "std::set", "std::deque", "std::list"};

         auto ctsd = get_template_specialization(type);

         if (ctsd) {
            auto  name    = ctsd->getQualifiedNameAsString();
            auto& args    = ctsd->getTemplateArgs();
            auto arg_name = [&](int i, bool canonical) { return get_template_arg_name(args[i], canonical); };

            if (name == "eosio::ignore") {
               auto ret = arg_name(0, true);
               return {ret, ret};
            } else if (name == "std::basic_string"){
               return {"string", "string"};
            } else if (name == "eosio::binary_extension") {
               auto ret = arg_name(0, false);
               return {ret + "$", ret + "_S"};
            } else if (name == "eosio::pb") {
               auto message_type = ctsd->getTemplateArgs()[0].getAsType()->getAsCXXRecordDecl()->getQualifiedNameAsString();
               message_type = std::regex_replace(message_type, std::regex("::"), "."); // replace '::' -> '.'
               pb_types.insert(message_type);
               message_type = "protobuf::" + message_type; 
               return {message_type, message_type};
            } else if (name == "eosio::pb_msg") {
               return {"bytes", "bytes"};
            } else if (name == "std::optional") {
               auto ret = arg_name(0, false);
               return {ret + "?", "optional_" + ret};
            } else if (name == "std::pair") {
               auto ret = "pair_" + arg_name(0, false) + "_" + arg_name(1, false);
               return {ret, ret};
            } else if (name == "std::map") {
               auto ret = arg_name(0, false) + "_" + arg_name(1, false);
               return {"pair_" + ret + "[]", "map_" + ret};
            } else if (name == "std::tuple" || name == "std::variant") {
               auto result = ctsd->getNameAsString();
               for (const auto& pack_elem : args[0].getPackAsArray()) {
                  result += "_";
                  auto element_name = translate_type(pack_elem.getAsType());
                  if ( strchr("]?$", element_name.back()) != 0)
                     element_name = get_type_strings(pack_elem.getAsType()).second;
                  result += element_name;
               }
               return {result, result + "_E"};
            } else if (std::find(sequence_types.begin(), sequence_types.end(), name) != sequence_types.end()) {
               auto element_type = arg_name(0, false);
               if (element_type == "int8")
                  return {"bytes", "bytes"};
               else
                  return {element_type + "[]", element_type + "_array"};
            } else if (name == "std::array") {
               auto size         = arg_name(1, false);
               auto element_type = arg_name(0, false);
               return {element_type + "[" + size + "]", element_type + "_array" + size};
            } else {
               std::string ret = ctsd->getNameAsString();
               for (int i = 0; i < args.size(); ++i) {
                  ret += "_";
                  ret += arg_name(i, false);
               }
               ret = _translate_type(ret);
               return {ret, ret};
            }
         }
         auto ret = _translate_type(type);
         return {ret, ret};
      }

      std::string translate_type(clang::QualType type) {
         auto itr = parsed_typenames.find(type.getCanonicalType());
         if (itr != parsed_typenames.end()) {
            return itr->second;
         }
         return get_type_strings(type).first;
      }

      inline bool is_builtin_type( const std::string& t ) {
         static const std::set<std::string> builtins =
         {
            "bool",
            "int8",
            "uint8",
            "int16",
            "uint16",
            "int32",
            "uint32",
            "int64",
            "uint64",
            "int128",
            "uint128",
            "varint32",
            "varuint32",
            "float32",
            "float64",
            "float128",
            "time_point",
            "time_point_sec",
            "name",
            "bytes",
            "string",
            "block_timestamp_type",
            "name",
            "checksum160",
            "checksum256",
            "checksum512",
            "public_key",
            "private_key",
            "signature",
            "symbol",
            "symbol_code",
            "asset",
            "extended_asset"
         };
         return builtins.count(_translate_type(t)) >= 1;
      }

      inline bool is_reserved( const std::string& t ) {
         return t.find("__") != std::string::npos;
      }

      inline std::string get_type( const clang::QualType& t ) {
         return translate_type(get_ignored_type(t));
      }

      inline std::string get_type_alias_string( const clang::QualType& t ) {
         if (auto dt = llvm::dyn_cast<clang::TypedefType>(t.getTypePtr()))
            return get_type(dt->desugar());
         else if (auto dt = llvm::dyn_cast<clang::ElaboratedType>(t.getTypePtr()))
            return get_type_alias_string(dt->desugar());
         return get_type(t);
      }

      inline std::vector<clang::QualType> get_type_alias( const clang::QualType& t ) {
         if (auto dt = llvm::dyn_cast<clang::TypedefType>(t.getTypePtr()))
            return {dt->desugar()};
         else if (auto dt = llvm::dyn_cast<clang::ElaboratedType>(t.getTypePtr()))
            return get_type_alias(dt->desugar());
         return {};
      }

      inline bool is_aliasing( const clang::QualType& t ) {
         if (is_builtin_type(t))
            return false;
         if (get_base_type_name(t).find("<") != std::string::npos) return false;
         return get_base_type_name(t).compare(get_type_alias_string(t)) != 0;
      }

      inline bool is_kv_map(const clang::CXXRecordDecl* decl) {
         return decl->getQualifiedNameAsString().find("eosio::kv::map<") != std::string::npos;
      }

      inline bool is_kv_table(const clang::CXXRecordDecl* decl) {
         if (decl->hasDefinition()) {
            for (const auto& base : decl->bases()) {
               auto type = base.getType().getAsString();
               if (type.find("eosio::kv::table<") == 0 ) {
                  return true;
               }
            }
         }
         return false;
      }

      inline bool is_kv_internal(const clang::CXXRecordDecl* decl) {
         const std::set<std::string> internal_types {
            "table",
            "table_base",
            "index",
            "index_base"
         };

         const auto fqn = decl->getQualifiedNameAsString();

         const auto in_kv_namespace = fqn.find("eosio::kv") != std::string::npos;
         const bool is_internal = internal_types.count(decl->getNameAsString());

         return in_kv_namespace && is_internal;
      }

      ojson struct_to_json( const abi_struct& s ) {
         ojson o;
         o["name"] = s.name;
         o["base"] = s.base;
         o["fields"] = ojson::array();
         for ( auto field : s.fields ) {
            ojson f;
            f["name"] = field.name;
            f["type"] = field.type;
            o["fields"].push_back(f);
         }
         return o;
      }

      ojson variant_to_json( const abi_variant& v ) {
         ojson o;
         o["name"] = v.name;
         o["types"] = ojson::array();
         for ( auto ty : v.types ) {
            o["types"].push_back( ty );
         }
         return o;
      }

      ojson typedef_to_json( const abi_typedef& t ) {
         ojson o;
         o["new_type_name"] = t.new_type_name;
         o["type"]          = t.type;
         return o;
      }

      ojson action_to_json( const abi_action& a ) {
         ojson o;
         o["name"] = a.name;
         o["type"] = a.type;
         o["ricardian_contract"] = a.ricardian_contract;
         return o;
      }

      ojson clause_to_json( const abi_ricardian_clause_pair& clause ) {
         ojson o;
         o["id"] = clause.id;
         o["body"] = clause.body;
         return o;
      }

      ojson table_to_json( const abi_table& t ) {
         ojson o;
         o["name"] = t.name;
         o["type"] = t.type;
         o["index_type"] = "i64";
         o["key_names"] = ojson::array();
         o["key_types"] = ojson::array();
         return o;
      }

      ojson wasm_action_to_json(const wasm_action& a) {
         ojson o;
         o["name"] = a.name;
         o["handler"] = a.handler;
         return o;
      }

      ojson wasm_notify_to_json(const wasm_notify& n) {
         ojson o;
         o["contract"] = n.contract;
         o["name"] = n.name;
         o["handler"] = n.handler;
         return o;
      }

      std::pair<std::string, ojson> kv_table_to_json( const abi_kv_table& t ) {
         ojson o;
         o["type"] = t.type;
         auto indices = ojson::object();
         for (int i = 0; i < t.indices.size(); ++i) {
            auto idx = t.indices[i];
            if (i == 0) {
               ojson oj;
               oj["name"] = idx.name;
               oj["type"] = idx.type;
               o["primary_index"] = oj;
            } else {
               ojson o;
               o["type"] = idx.type;
               indices.insert_or_assign(idx.name, o);
            }
         }
         if (t.indices.empty()) {
            o["primary_index"]= ojson::object();
         }
         o["secondary_indices"] = indices;
         return {t.name, o};
      }

      ojson action_result_to_json( const abi_action_result& result ) {
         ojson o;
         o["name"] = result.name;
         o["result_type"] = result.type;
         return o;
      }

      bool is_empty() {
         std::set<abi_table> set_of_tables;
         for ( auto t : ctables ) {
            bool has_multi_index = false;
            for ( auto u : _abi.tables ) {
               if (t.type == u.type) {
                  has_multi_index = true;
                  break;
               }
               set_of_tables.insert(u);
            }
            if (!has_multi_index)
               set_of_tables.insert(t);
         }
         for ( auto t : _abi.tables ) {
            set_of_tables.insert(t);
         }

         return _abi.structs.empty() && _abi.typedefs.empty() && _abi.actions.empty() && set_of_tables.empty() && _abi.ricardian_clauses.empty() && _abi.variants.empty();
      }

      ojson to_json() {
         ojson o;
         o["version"]     = _abi.version_string();
         o["structs"]     = ojson::array();
         auto remove_suffix = [&]( std::string name ) {
            int i = name.length()-1;
            for (; i >= 0; i--)
               if ( name[i] != '[' && name[i] != ']' && name[i] != '?' && name[i] != '$' )
                  break;
            return name.substr(0,i+1);
         };

         std::set<abi_table> set_of_tables;
         for ( auto t : ctables ) {
            bool has_multi_index = false;
            for ( auto u : _abi.tables ) {
               if (t.type == u.type) {
                  has_multi_index = true;
                  break;
               }
               set_of_tables.insert(u);
            }
            if (!has_multi_index)
               set_of_tables.insert(t);
         }
         for ( auto t : _abi.tables ) {
            set_of_tables.insert(t);
         }

         std::function<std::string(const std::string&)> get_root_name;
         get_root_name = [&] (const std::string& name) {
            for (auto td : _abi.typedefs)
               if (remove_suffix(name) == td.new_type_name)
                  return get_root_name(td.type);
            return name;
         };

         auto validate_struct = [&]( abi_struct as ) {
            if ( is_builtin_type(as.name) )
               return false;
            if ( is_reserved(as.name) ) {
               return false;
            }
            for ( auto& s : _abi.structs ) {
               for ( auto f : s.fields ) {
                  if (as.name == remove_suffix(f.type))
                     return true;
               }
               for ( auto& v : _abi.variants ) {
                  for ( auto vt : v.types ) {
                     if (as.name == remove_suffix(vt))
                        return true;
                  }
               }
               if (get_root_name(s.base) == as.name)
                  return true;
            }
            for ( auto& v : _abi.variants ) {
               for ( auto vt : v.types ) {
                  if (as.name == remove_suffix(vt))
                     return true;
               }
            }
            for ( auto& a : _abi.actions ) {
               if (as.name == a.type)
                  return true;
            }
            for( auto& t : set_of_tables ) {
               if (as.name == t.type)
                  return true;
            }
            for ( const auto& t : _abi.kv_tables ) {
               if (as.name == t.type)
                  return true;
            }
            for( auto& td : _abi.typedefs ) {
               if (as.name == remove_suffix(td.type))
                  return true;
            }
            for( auto& ar : _abi.action_results ) {
               if (as.name == ar.type)
                  return true;
            }
            for (auto& t: _abi.kv_tables){
               if (t.type == as.name)
                  return true;
               for (auto& idx: t.indices) {
                  if (idx.type == as.name)
                     return true;
               }
            } 
            return false;
         };

         auto validate_types = [&]( abi_typedef td ) {
            if (td.new_type_name == td.type)
               return false;
            for ( auto& as : _abi.structs )
               if (validate_struct(as)) {
                  for ( auto f : as.fields )
                     if ( remove_suffix(f.type) == td.new_type_name )
                        return true;
                  if (as.base == td.new_type_name)
                     return true;
               }

            for ( auto& v : _abi.variants ) {
               for ( auto& vt : v.types ) {
                  if ( remove_suffix(vt) == td.new_type_name )
                     return true;
               }
            }
            for ( auto& t : _abi.tables )
               if ( t.type == td.new_type_name )
                  return true;
            for ( auto& a : _abi.actions )
               if ( a.type == td.new_type_name )
                  return true;
            for ( auto& _td : _abi.typedefs )
               if ( remove_suffix(_td.type) == td.new_type_name )
                  return true;
            for ( auto& ar : _abi.action_results ) {
               if ( ar.type == td.new_type_name )
                  return true;
            }
            for (auto& t: _abi.kv_tables){
               if (t.type == td.new_type_name)
                  return true;
               for (auto& idx: t.indices) {
                  if (idx.type == td.new_type_name)
                     return true;
               }
            } 

            return false;
         };

         for ( auto& s : _abi.structs ) {
            const auto res = validate_struct(s);
            if (res)
               o["structs"].push_back(struct_to_json(s));
         }
         o["types"]       = ojson::array();
         for ( auto& t : _abi.typedefs ) {
            if (validate_types(t))
               o["types"].push_back(typedef_to_json( t ));
         }
         o["actions"]     = ojson::array();
         for ( auto& a : _abi.actions ) {
            o["actions"].push_back(action_to_json( a ));
         }
         o["tables"]     = ojson::array();
         for ( auto& t : set_of_tables ) {
            o["tables"].push_back(table_to_json( t ));
         }
         if (_abi.version_major == 1 && _abi.version_minor >= 2) {
            o["kv_tables"]  = ojson::object();
            for ( const auto& t : _abi.kv_tables ) {
               auto kv_table = kv_table_to_json(t);
               o["kv_tables"].insert_or_assign(kv_table.first, kv_table.second);
            }
         }
         o["ricardian_clauses"]  = ojson::array();
         for ( auto& rc : _abi.ricardian_clauses ) {
            o["ricardian_clauses"].push_back(clause_to_json( rc ));
         }
         o["variants"]   = ojson::array();
         for ( auto& v : _abi.variants ) {
            o["variants"].push_back(variant_to_json( v ));
         }
         o["abi_extensions"]     = ojson::array();
         if (_abi.version_major == 1 && _abi.version_minor >= 2) {
            o["action_results"]  = ojson::array();
            for ( auto& ar : _abi.action_results ) {
               o["action_results"].push_back(action_result_to_json( ar ));
            }
         }
         return o;
      }

      ojson to_json_debug() {
         auto o = to_json();
         o["wasm_actions"] = ojson::array();
         for (auto& a : _abi.wasm_actions) {
            o["wasm_actions"].push_back(wasm_action_to_json(a));
         }
         o["wasm_notifies"] = ojson::array();
         for (auto& n : _abi.wasm_notifies) {
            o["wasm_notifies"].push_back(wasm_notify_to_json(n));
         }
         o["wasm_entries"] = ojson::array();
         for (auto& e : _abi.wasm_entries) {
            o["wasm_entries"].push_back(e);
         }
         o["pb_types"] = ojson::array();
         for (auto& e : pb_types)
            o["pb_types"].push_back(e);
         return o;
      }

      private:
         abi                                   _abi;
         std::set<abi_table>                   ctables;
         std::map<std::string, std::string>    rcs;
         std::set<const clang::Type*>          evaluated;
         std::set<std::string>                 pb_types;

         std::string get_type_string_from_kv_index_macro_decltype(const clang::DecltypeType* decl) {
            if (const auto ref_type = dyn_cast<clang::LValueReferenceType>(decl->desugar())) {
               return get_type(ref_type->getPointeeType());
            } else { 
               auto ttype = llvm::dyn_cast<clang::TemplateSpecializationType>(decl->desugar());
               auto etype = llvm::dyn_cast<clang::ElaboratedType>(ttype->getAliasedType());
               auto typedeftype = llvm::dyn_cast<clang::TypedefType>(etype->desugar());
               auto stype = llvm::dyn_cast<clang::SubstTemplateTypeParmType>(typedeftype->desugar());
               return get_type(stype->getReplacementType());
            }
         }

   };

   class eosio_abigen_visitor : public RecursiveASTVisitor<eosio_abigen_visitor>, public generation_utils {
      private:
         bool has_added_clauses = false;
         abigen& ag = abigen::get();

      public:
         explicit eosio_abigen_visitor(CompilerInstance *CI) {
            get_error_emitter().set_compiler_instance(CI);
         }

         bool shouldVisitTemplateInstantiations() const {
            return true;
         }

         bool VisitFunctionDecl(const FunctionDecl * decl) {
            // extract the ABI type name for a given C++ type from the definition of `constexpr const char* get_type_name()` functions.
            if (decl->getNameAsString() == "get_type_name" && decl->hasBody() && decl->hasOneParamOrDefaultArgs()) {
               if (auto stmt = llvm::dyn_cast<clang::CompoundStmt>(decl->getBody())) {
                  if (auto return_stmt = llvm::dyn_cast<clang::ReturnStmt>(stmt->body_front()) ) {
                     if (auto return_val = llvm::dyn_cast<clang::CastExpr>(return_stmt->getRetValue())) {
                        if (auto literal = llvm::dyn_cast<clang::StringLiteral>(return_val->getSubExpr())) {
                           std::string abi_name {literal->getString()};
                           if (auto ns_pos = abi_name.rfind("::"); ns_pos != std::string::npos) {
                              abi_name = abi_name.substr(ns_pos + 2);
                           }
                           ag.parsed_typenames.emplace(decl->getParamDecl(0)->getType()->getPointeeType().getCanonicalType(), abi_name);
                        }
                     }
                  }
               }
            }
            return true;
         }

         virtual bool VisitCXXMethodDecl(clang::CXXMethodDecl* _decl) {
            auto decl = clang_wrapper::wrap_decl(_decl);
            if (!has_added_clauses) {
               ag.add_clauses(ag.parse_clauses());
               ag.add_contracts(ag.parse_contracts());
               has_added_clauses = true;
            }

            if (decl.isEosioAction() && ag.is_eosio_contract(decl, ag.get_contract_name())) {
               ag.add_struct(*decl);
               ag.add_action(*decl);
               for (auto param : decl->parameters()) {
                  ag.add_type( param->getType() );
               }
            }
            return true;
         }
         virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl* _decl) {
            auto qualified_name = std::string{};
            auto ss             = llvm::raw_string_ostream{qualified_name};
            _decl->printQualifiedName(ss);
            // Exclude "system" types.
            const auto namespaces = std::vector<const char*>{
               "std::", "boost::", "eosio::", "rapidjson::", "zpp::", "magic_enum::", "bluegrass::"};
            auto exclude          = false;
            for (const auto& ns : namespaces) {
               if (std::strncmp(ns, qualified_name.c_str(), std::strlen(ns)) == 0) {
                  exclude = true;
                  break;
               }
            }
            if (auto it = ag.type_lookup.find(qualified_name); it == std::end(ag.type_lookup) && !exclude) {
               ag.type_lookup.emplace(qualified_name, clang::QualType{_decl->getTypeForDecl(), 0});
            }

            auto decl = clang_wrapper::wrap_decl(_decl);
            if (!has_added_clauses) {
               ag.add_clauses(ag.parse_clauses());
               ag.add_contracts(ag.parse_contracts());
               has_added_clauses = true;
            }
            bool is_action = decl.isEosioAction();
            bool is_table = decl.isEosioTable();
            bool is_kv_table = ag.is_kv_table(_decl);

            if (((is_action || is_table) && ag.is_eosio_contract(decl, ag.get_contract_name())) || is_kv_table ) {
               ag.add_struct(_decl);
               if (is_action)
                  ag.add_action(_decl);
               else if (is_kv_table)
                  ag.add_kv_table(_decl);
               else if (is_table)
                  ag.add_table(_decl);
               for (auto field : _decl->fields()) {
                  ag.add_type( field->getType() );
               }
            } else if (auto  x = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(_decl) ; x && x->getName() == "kv_caching_singleton") {
               ag.add_kv_caching_singleton(x);
            }
            return true;
         }

         bool VisitClassTemplateSpecializationDecl(const clang::ClassTemplateSpecializationDecl* d){
            if (d->getName() == "multi_index") {
               ag.add_table(
                   d->getTemplateArgs()[0].getAsIntegral().getExtValue(),
                   (clang::CXXRecordDecl*)((clang::RecordType*)d->getTemplateArgs()[1].getAsType().getTypePtr())
                       ->getDecl());
            } else if (d->getName() == "map") {
               auto decl = clang_wrapper::wrap_decl(d->getSpecializedTemplate()->getTemplatedDecl());
               if (decl.isEosioTable())
                  ag.add_kv_map(d);
            } 
            return true;
         }
   };

   class eosio_abigen_consumer : public ASTConsumer {
      private:
         eosio_abigen_visitor *visitor;
         std::string main_file;
         CompilerInstance* ci;

      public:
         explicit eosio_abigen_consumer(CompilerInstance *CI, std::string file)
            : visitor(new eosio_abigen_visitor(CI)), main_file(file), ci(CI) { }

         virtual void HandleTranslationUnit(ASTContext &Context) {
            if (abigen::get().no_abigen) {
               return;
            }
            auto& src_mgr = Context.getSourceManager();
            abigen::get().source_manager = &src_mgr;
            auto& f_mgr = src_mgr.getFileManager();
            auto main_fe = f_mgr.getFile(main_file);
            if (main_fe) {
               auto fid = src_mgr.getOrCreateFileID(*f_mgr.getFile(main_file), SrcMgr::CharacteristicKind::C_User);
               visitor->TraverseDecl(Context.getTranslationUnitDecl());

               std::ofstream ofs (output + ".desc");
               if (!ofs) throw;
               if (!abigen::get().is_empty()) ofs << pretty_print(abigen::get().to_json_debug());
               ofs.close();
            }
         }
   };

   class eosio_abigen_frontend_action : public PluginASTAction {
      public:
         std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
            CI.getPreprocessor().addPPCallbacks(std::make_unique<eosio_ppcallbacks>(CI.getSourceManager(), file.str()));
            return std::make_unique<eosio_abigen_consumer>(&CI, file.str());
         }

         bool ParseArgs(const CompilerInstance& CI, const std::vector<std::string>& args) override {
            if (args.empty())
               return true;

            std::vector<std::string> resource_dirs;
            for (const auto& arg : tokenize(args[0])) {
               if (eosio::cdt::starts_with(arg, "contract=")) {
                  abigen::get().set_contract_name(arg.substr(arg.find("=")+1));
               } else if (eosio::cdt::starts_with(arg, "abi_version=")) {
                  auto str = arg.substr(arg.find("=")+1);
                  float tmp;
                  int abi_version_major = std::stoi(str);
                  int abi_version_minor = (int)(std::modf(std::stof(str), &tmp) * 10);
                  abigen::get().set_abi_version(abi_version_major, abi_version_minor);
               } else if (arg == "no_abigen") {
                  abigen::get().no_abigen = true;
               } else if (arg == "suppress_ricardian_warnings") {
                  abigen::get().set_suppress_ricardian_warning(true);
               } else if (eosio::cdt::starts_with(arg, "R=")) {
                  resource_dirs.emplace_back(arg.substr(arg.find("=")+1));
               } else if  (eosio::cdt::starts_with(arg, "is_wasm=")) {
                  abigen::get().is_wasm = arg.substr(arg.find("=")+1) == "true"; 
               } else {
                  return false;
               }
            }
            if (resource_dirs.size()) {
               abigen::get().set_resource_dirs(resource_dirs);
            }
            return true;
         }

         ActionType getActionType() override {
            return ReplaceAction;
         }
   };
}} // ns eosio::cdt
