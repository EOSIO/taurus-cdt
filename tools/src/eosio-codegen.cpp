#include <eosio/pb_support.hpp>
#include <eosio/cdt_abi.hpp>
#include <eosio/abi.hpp>
#include <eosio/abimerge.hpp>
#include <eosio/whereami/whereami.hpp>

#include <fstream>
#include <set>
#include <unistd.h>
#include <sys/stat.h>
#include <cxxopts.hpp>
#include <llvm/Support/Program.h>
#include <google/protobuf/compiler/importer.h>

int file_size(const char* filename) {
   struct stat st;
   stat(filename, &st);
   return st.st_size;
}

bool exists(const char* filename) {
   struct stat st;
   return stat(filename, &st) == 0;
}

void generate_eosio_dispatch(const std::string& output, const std::set<wasm_action>& wasm_actions,
                             const std::set<wasm_notify>& wasm_notifies) {
   try {
      std::ofstream ofs(output);
      if (!ofs)
         throw;
      ofs << "#include <cstdint>\n"
          << "#include <eosio/name.hpp>\n"
          << "using namespace eosio::literals;\n" ;
      ofs << "extern \"C\" {\n";
      ofs << "  __attribute__((import_name(\"eosio_assert_code\"))) void eosio_assert_code(uint32_t, uint64_t);";
      ofs << "  void eosio_set_contract_name(uint64_t n);\n";
      for (auto& wa : wasm_actions) {
         ofs << "  void " << wa.handler << "(uint64_t r, uint64_t c);\n";
      }
      for (auto& wn : wasm_notifies) {
         ofs << "  void " << wn.handler << "(uint64_t r, uint64_t c);\n";
      }
      ofs << "  __attribute__((export_name(\"apply\"), visibility(\"default\")))\n";
      ofs << "  void apply(uint64_t r, uint64_t c, uint64_t a) {\n";
      ofs << "    eosio_set_contract_name(r);\n";
      ofs << "    if (c == r) {\n";
      if (wasm_actions.size()) {
         ofs << "      switch (a) {\n";
         for (auto& wa : wasm_actions) {
            ofs << "      case \"" << wa.name << "\"_n.value:\n";
            ofs << "        " << wa.handler << "(r, c);\n";
            ofs << "        break;\n";
         }
         // assert that no action was found when the receiver is not "eosio"
         ofs << "      default:\n"
             << "        if ( r != \"eosio\"_n.value) eosio_assert_code(false, 1);\n"
             << "      }\n";
      }
      ofs << "    } else {\n";
      if (wasm_notifies.size()) {
         std::string action;
         for (auto& wn : wasm_notifies) {
            if (wn.name != action) {
               if (action.empty()) {
                  ofs << "      if (a == \"" << wn.name << "\"_n.value) {\n";
               } else {
                  ofs << "        }\n";
                  ofs << "      }\n";
                  ofs << "      else if (a == \"" << wn.name << "\"_n.value) {\n";
               }
               ofs << "        switch (c) {\n";
               action = wn.name;
            }
            if (wn.contract != "*")
               ofs << "        case \"" << wn.contract << "\"_n.value:\n";
            else
               ofs << "        default:\n";
            ofs << "          " << wn.handler << "(r, c);\n";
            ofs << "          break;\n";
         }
         ofs << "        }\n";
         ofs << "      }\n";
      }
      ofs << "    }\n";
      ofs << "  }\n";
      ofs << "}\n";
      ofs.close();
   } catch (...) {
      std::cerr << "Failed to generate eosio dispatcher\n";
   }
}

std::vector<std::string> split_then_prepend(const std::string& s, char delim, std::string prefix) {
   std::vector<std::string> result;
   std::stringstream        ss(s);
   std::string              item;

   while (std::getline(ss, item, delim)) {
      if (item.size())
         result.push_back(prefix + item);
   }

   return result;
}

std::vector<std::string> desc_files;
std::vector<std::string> input_files;
std::vector<std::string> resource_dirs;
std::vector<std::string> compiler_options;
std::string              protobuf_dir;
std::vector<std::string> protobuf_files;
std::string              contract_name;
std::string              output_dir=".";

std::string abi_version;
int         abi_version_major           = 1;
int         abi_version_minor           = 3;
bool        no_abigen                   = false;
bool        verbose                     = false;
bool        suppress_ricardian_warnings = true;
bool        is_wasm = false;
std::string smart_contract_trace_level;


int exec_subprogram(std::string prog, const std::vector<std::string>& options, bool show_commands) {
   if (prog.size() && prog[0] != '/') {
      prog =  eosio::cdt::whereami::where() + "/" + prog;
   }
   
#if defined(__APPLE__)
   // in macOS 10.15, if using symbolic link to call "clang++", headers in system include path 
   // (e.g. /usr/local/opt/llvm@13/bin/../include/c++/v1) couldn't be found
   struct stat sb;
   if (lstat(prog.c_str(), &sb) != -1) {
      std::vector<char> buf(sb.st_size + 1);
      ssize_t nbytes = readlink(prog.c_str(), buf.data(), buf.size());
      if (nbytes != -1) {
         prog.assign(buf.data(), nbytes);
      }
   }
#endif

   if (show_commands) { 
      std::cout << prog ;
      for (const auto& s : options) 
         std::cout << " " << s;
      std::cout << "\n";
   }
   
   std::vector<llvm::StringRef> args;
   args.push_back(prog);
   args.insert(args.end(), options.begin(), options.end());
   return  llvm::sys::ExecuteAndWait(prog.c_str(), args);
}


void parse_args(int argc, const char** argv) {

   cxxopts::Options options(argv[0], "Code and ABI generator for EOS contracts");
   std::string cxx_arg; 

   // clang-format off
   options.add_options()
       ("abi-version", "ABI version", cxxopts::value<std::string>())
       ("no-abigen", "diable ABI generation")
       ("cxx", "additional C++ compiler options", cxxopts::value<std::string>(cxx_arg))
       ("I,include", "C++ include directories", cxxopts::value<std::vector<std::string>>())
       ("D,defines", "C macros", cxxopts::value<std::string>())
       ("contract", "contract name", cxxopts::value<std::string>(contract_name))
       ("smart-contract-trace-level", "smart contract trace level", cxxopts::value<std::string>(smart_contract_trace_level))
       ("output-dir", "output dirirectory", cxxopts::value<std::string>(output_dir))
       ("version", "display version")
       ("v,verbose", "verbose output", cxxopts::value<bool>(verbose)->default_value("false"))
       ("protobuf-dir", "the root directory of the specified protobuf files", cxxopts::value<std::string>())
       ("protobuf-files", "protobuf schema files to be used for the contract", cxxopts::value<std::string>())
       ("input", "input files", cxxopts::value<std::vector<std::string>>(input_files))
       ("h,help", "Print usage");

   // clang-format on
   options.parse_positional("input");
   auto result = options.parse(argc, argv);

   if (result.count("help")) {
      std::cout << options.help() << std::endl;
      exit(0);
   }

   if (result.count("version")) {
      std::cout << "eosio-codegen version " << EOSIO_CDT_VERSION << "\n";
      exit(0);
   }

   if (result.count("abi-version")) {
      auto abi_version = result["abi-version"].as<std::string>();
      float tmp;
      abi_version_major = std::stoi(abi_version);
      abi_version_minor = (int)(std::modf(std::stof(abi_version), &tmp) * 10);
   }

   no_abigen = result.count("no-abigen");

   if (result.count("defines")) {
      auto args = split_then_prepend(result["defines"].as<std::string>(), ';', "-D");
      compiler_options.insert(compiler_options.end(), args.begin(), args.end());
   }

   if (result.count("include")) {
      auto includes = result["include"].as<std::vector<std::string>>();
      for (auto i : includes) {
         auto args = split_then_prepend(i, ';', "-I");
         compiler_options.insert(compiler_options.end(), args.begin(), args.end());
      }
   }
   
   if (result.count("cxx") == 0) {
      cxx_arg = getenv("EOSIO_CXX_OPTIONS");
      if (cxx_arg.empty()) {
         std::cerr << "You must expclicitly specify --cxx option or set environment variable EOSIO_CXX_OPTIONS\n";
         exit(1); 
      }
   }
   auto args = split_then_prepend(cxx_arg, ' ', "");
   compiler_options.insert(compiler_options.end(), args.begin(), args.end());
   is_wasm = cxx_arg.find("--target=wasm") !=  std::string::npos;


   if (result.count("include")==0) {
      compiler_options.push_back("-I" + eosio::cdt::whereami::where()+ "/../include");
   }


   if (result.count("contract") == 0)  {
      if (input_files.size() == 1) {
         auto fn = input_files[0];
         fn = fn.substr(fn.rfind('/')+1);
         contract_name = fn.substr(0, fn.rfind('.'));
      }
      else {
         std::cerr << "missing --contract argument\n";
         exit(1);
      }
   }

   if (result.count("protobuf-dir")) {
      protobuf_dir = result["protobuf-dir"].as<std::string>();
   }

   if (result.count("protobuf-files")) {
      protobuf_files = split_then_prepend(result["protobuf-files"].as<std::string>(), ';', "");
   }
}

void gen_actions(std::string input) {
   std::string                 _output = output_dir + "/" + input.substr(input.rfind('/')+1);
   std::vector<std::string> local_args = compiler_options;
   local_args.push_back("-Wno-unknown-attributes");
   local_args.push_back("-fsyntax-only");
   local_args.emplace_back("-fplugin="+eosio::cdt::whereami::where()+"/eosio_attrs" SHARED_LIB_SUFFIX);
   local_args.push_back("-fplugin=" + eosio::cdt::whereami::where() + "/eosio_codegen" + SHARED_LIB_SUFFIX);

   std::string codegen_opts;
   codegen_opts += "output=" + _output;

   if (contract_name.size()) {
      codegen_opts += ",contract=" + contract_name;
   }
   if (smart_contract_trace_level.size()) {
      codegen_opts += ",smart-contract-trace-level=" + smart_contract_trace_level;
   }      

   local_args.push_back("-Xclang");
   local_args.push_back("-plugin-arg-eosio_codegen");
   local_args.push_back("-Xclang");
   local_args.push_back(codegen_opts);

   std::string abigen_opts;
   if (contract_name.size()) {
      abigen_opts += "contract=" + contract_name;
   }
   if (abigen_opts.size())
      abigen_opts += ",";

   if (abi_version.size()) {
      abigen_opts += "abi_version=" + abi_version;
   } else if (no_abigen) {
      abigen_opts += "no_abigen";
   } else {
      abigen_opts += "abi_version=1.3";
   }

   if (suppress_ricardian_warnings) {
      if (abigen_opts.size())
         abigen_opts += ",";
      abigen_opts += "suppress_ricardian_warnings";
   }
   if (resource_dirs.size()) {
      for (const auto& r : resource_dirs) {
         if (abigen_opts.size())
            abigen_opts += ",";
         abigen_opts += "R=" + r;
      }
   }

   if (is_wasm) {
      abigen_opts += ",is_wasm=true";
   }

   local_args.push_back("-Xclang");
   local_args.push_back("-plugin-arg-eosio_abigen");
   local_args.push_back("-Xclang");
   local_args.push_back(abigen_opts);
   local_args.push_back("-c");
   local_args.push_back(input);

   if (auto ret = exec_subprogram("clang++", local_args, verbose)) {
      exit(ret);
   }

   auto desc_file = _output + ".desc";

   if (exists(desc_file.c_str())) {
      desc_files.push_back(desc_file);
   }
}

namespace gpb = google::protobuf;
namespace gpbc = google::protobuf::compiler;
class ErrorCollector : public gpbc::MultiFileErrorCollector {
  public:
    ErrorCollector() {}
    ~ErrorCollector() {}

    // implements ErrorCollector ---------------------------------------
    void AddError(const std::string& filename, int line, int column, const std::string& message) override {
        std::cerr << "error filename " << filename << " (" << line << ", " << column << ")  message " << message
                  << std::endl;
    }
};

int main(int argc, const char** argv) {

   std::set<wasm_action> wasm_actions;
   std::set<wasm_notify> wasm_notifies;
   bool                  dispatcher_was_found = false;

   parse_args(argc, argv);

   try {

      for (auto input : input_files) {
         gen_actions(input);
      }

      ojson abi;
      std::set<std::string> referenced_pb_types;

      for (const auto& desc_name : desc_files) {
         if (exists(desc_name.c_str())) {
            if (file_size(desc_name.c_str())==0)
               continue;
            std::ifstream ifs(desc_name);
            auto          desc = ojson::parse(ifs);
            ifs.close();

            abi = ABIMerger(abi, abi_version_major, abi_version_minor).merge(desc);

            for (auto wa : desc["wasm_actions"].array_range()) {
               wasm_action act;
               act.name    = wa["name"].as_string();
               act.handler = wa["handler"].as_string();
               wasm_actions.insert(act);
            }
            for (auto wn : desc["wasm_notifies"].array_range()) {
               wasm_notify noti;
               noti.contract = wn["contract"].as_string();
               noti.name     = wn["name"].as_string();
               noti.handler  = wn["handler"].as_string();
               wasm_notifies.insert(noti);
            }

            for (auto pb_type: desc["pb_types"].array_range()) {
               referenced_pb_types.insert(pb_type.as_string());
            }

            if (!dispatcher_was_found) {
               for (auto we : desc["wasm_entries"].array_range()) {
                  auto name = we.as_string();
                  if (name == "apply") {
                     dispatcher_was_found = true;
                     break;
                  }
               }
            }
         }
      }

      if (!no_abigen) {
         if (abi.empty()) {
            std::cerr << "abigen error\n";
            return -1;
         }
         else {
            std::string filename = output_dir + "/" + contract_name + ".abi";
            std::ofstream ofs(filename);
            if (!ofs) {
               std::cerr << "cannot open " + filename + "\n";
               return -1;
            }
            jsoncons::json_options opts;
            opts.spaces_around_comma(jsoncons::spaces_option::no_spaces);

            if (protobuf_files.size()) {
               gpbc::DiskSourceTree source_tree;
               source_tree.MapPath("", protobuf_dir);
               source_tree.MapPath("", eosio::cdt::whereami::where() + "/../include");

               ErrorCollector err_collector;
               gpbc::Importer importer(&source_tree,&err_collector);

               for (auto proto_file : protobuf_files) {
                  importer.Import(proto_file.c_str());
               }

               auto pool = importer.pool();

               for (auto type : referenced_pb_types) {
                  if (!pool->FindMessageTypeByName(type)) {
                     std::cerr << "unable to find the definition of the protobuf type: '" << type << "',\n"
                                 "please make sure the correspoinding protobuf file is correctly specified\n";
                     return -1;
                  }
               }
            
               eosio::gpb::FileDescriptorSet fds;
               for (auto proto_file : protobuf_files) {
                  auto descriptor = pool->FindFileByName(proto_file);
                  auto file = fds.add_file();
                  descriptor->CopyTo(file);

                  // if the dependency contains the item "zpp_options.proto", remove it because
                  // the dependency doesn't contain any useful information for encoding/decoding. 
                  int zpp_options_index = -1;
                  // find item which contains a string ends with "zpp_options.proto"
                  for (int i = 0; i < file->dependency_size(); ++i) {
                     auto item = file->dependency(i);
                     const char* zpp_options_proto_string = "zpp_options.proto";
                     const auto zpp_options_proto_string_len = strlen(zpp_options_proto_string);
                     auto pos = item.find(zpp_options_proto_string);
                     if (pos != std::string::npos && (item.size()-pos) == zpp_options_proto_string_len && (pos == 0 || item[pos-1]=='/')){
                        zpp_options_index = i;
                        break;
                     }
                  }
                  if (zpp_options_index >= 0) {
                     // remove the found item
                     auto last_index = file->dependency_size()-1;
                     if (zpp_options_index != last_index) {
                        *file->mutable_dependency(zpp_options_index) = file->dependency(last_index);
                     }
                     file->mutable_dependency()->RemoveLast();
                  }
               }

               std::string protobuf_types;
               auto status = google::protobuf::util::MessageToJsonString(fds, &protobuf_types, google::protobuf::util::JsonPrintOptions{});

               abi["protobuf_types"] = ojson::parse(protobuf_types);
            } else if (referenced_pb_types.size()) {
               std::cerr << "protobuf types are used but no protobuf file are specifified for contract " << contract_name 
                         << ", please use `conctract_use_protobuf()` cmake function to specify the protobuf files it depends on\n";
                     return -1;
            }

            std::stringstream abi_json;
            abi_json << pretty_print(abi, opts);
            ofs << abi_json.str();
            ofs.close();

            // verify the correctness of abi by construct an abi object
            eosio::abi{eosio::abi_def::json_to_bin(abi_json.str())};
         }
      }

      auto main_file = output_dir + "/" + contract_name + ".dispatch.cpp";
      generate_eosio_dispatch(main_file, wasm_actions, wasm_notifies);
      return 0;
   } catch (std::runtime_error& err) {
      std::cerr << err.what() << '\n';
      return -1;
   }
}
