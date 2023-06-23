#include <google/protobuf/compiler/plugin.pb.h>
#include <google/protobuf/descriptor.pb.h>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_set>
#include <utility>
#include "src/zpp_options.pb.h"

namespace gpb  = google::protobuf;
namespace gpbc = google::protobuf::compiler;

std::string strip_proto(std::string name) {
   assert(name.size() > 6);
   name.resize(name.size() - 6);
   return name;
}

bool is_keyword(std::string word) {
   static std::unordered_set<std::string> keywords = {
       "NULL",          "alignas",      "alignof",   "and",        "and_eq",
       "asm",           "auto",         "bitand",    "bitor",      "bool",
       "break",         "case",         "catch",     "char",       "class",
       "compl",         "const",        "constexpr", "const_cast", "continue",
       "decltype",      "default",      "delete",    "do",         "double",
       "dynamic_cast",  "else",         "enum",      "explicit",   "export",
       "extern",        "false",        "float",     "for",        "friend",
       "goto",          "if",           "inline",    "int",        "long",
       "mutable",       "namespace",    "new",       "noexcept",   "not",
       "not_eq",        "nullptr",      "operator",  "or",         "or_eq",
       "private",       "protected",    "public",    "register",   "reinterpret_cast",
       "return",        "short",        "signed",    "sizeof",     "static",
       "static_assert", "static_cast",  "struct",    "switch",     "template",
       "this",          "thread_local", "throw",     "true",       "try",
       "typedef",       "typeid",       "typename",  "union",      "unsigned",
       "using",         "virtual",      "void",      "volatile",   "wchar_t",
       "while",         "xor",          "xor_eq"};
   return keywords.count(word);
}

std::string reserve_keyword(const std::string& value) { return is_keyword(value) ? value + "_" : value; }

std::string ClassName(const gpb::Descriptor* descriptor) {
   const gpb::Descriptor* parent = descriptor->containing_type();
   std::string            res;
   if (parent)
      res += ClassName(parent) + "::";
   res += descriptor->name();
   if (descriptor->options().map_entry())
      res += "_DoNotUse";
   return reserve_keyword(res);
}

std::string ClassName(const gpb::EnumDescriptor* enum_descriptor) {
   if (enum_descriptor->containing_type() == nullptr) {
      return reserve_keyword(enum_descriptor->name());
   } else {
      return ClassName(enum_descriptor->containing_type()) + "::" + enum_descriptor->name();
   }
}

bool IsWellKnownMessage(const gpb::FileDescriptor* file) {
   static const std::unordered_set<std::string> well_known_files{
       "google/protobuf/any.proto",
       "google/protobuf/api.proto",
       "google/protobuf/compiler/plugin.proto",
       "google/protobuf/descriptor.proto",
       "google/protobuf/duration.proto",
       "google/protobuf/empty.proto",
       "google/protobuf/field_mask.proto",
       "google/protobuf/source_context.proto",
       "google/protobuf/struct.proto",
       "google/protobuf/timestamp.proto",
       "google/protobuf/type.proto",
       "google/protobuf/wrappers.proto",
   };
   return well_known_files.find(file->name()) != well_known_files.end();
}

// ----------------------------------------------------------------------
// StringReplace()
//    Replace the "old" pattern with the "new" pattern in a string,
//    and append the result to "res".  If replace_all is false,
//    it only replaces the first instance of "old."
// ----------------------------------------------------------------------

void StringReplace(const std::string& s, const std::string& oldsub, const std::string& newsub, bool replace_all,
                   std::string* res) {
   if (oldsub.empty()) {
      res->append(s); // if empty, append the given string.
      return;
   }

   std::string::size_type start_pos = 0;
   std::string::size_type pos;
   do {
      pos = s.find(oldsub, start_pos);
      if (pos == std::string::npos) {
         break;
      }
      res->append(s, start_pos, pos - start_pos);
      res->append(newsub);
      start_pos = pos + oldsub.size(); // start searching again after the "old"
   } while (replace_all);
   res->append(s, start_pos, s.length() - start_pos);
}

// ----------------------------------------------------------------------
// StringReplace()
//    Give me a string and two patterns "old" and "new", and I replace
//    the first instance of "old" in the string with "new", if it
//    exists.  If "global" is true; call this repeatedly until it
//    fails.  RETURN a new string, regardless of whether the replacement
//    happened or not.
// ----------------------------------------------------------------------

std::string StringReplace(const std::string& s, const std::string& oldsub, const std::string& newsub,
                          bool replace_all) {
   std::string ret;
   StringReplace(s, oldsub, newsub, replace_all, &ret);
   return ret;
}

std::string DotsToColons(const std::string& name) { return StringReplace(name, ".", "::", true); }

std::string Namespace(const std::string& package) {
   if (package.empty())
      return "";
   return "::" + DotsToColons(package);
}

std::string Namespace(const gpb::FileDescriptor* d) {
   std::string ret = Namespace(d->package());
   if (IsWellKnownMessage(d)) {
      // Written with string concatenation to prevent rewriting of
      // ::google::protobuf.
      ret = StringReplace(ret,
                          "::google::"
                          "protobuf",
                          "::PROTOBUF_NAMESPACE_ID", false);
   }
   return ret;
}

std::string QualifiedFileLevelSymbol(const gpb::FileDescriptor* file, const std::string& name) {
   if (file->package().empty()) {
      return "::" + name;
   }
   return Namespace(file) + "::" + name;
}

std::string QualifiedClassName(const gpb::Descriptor* d) { return QualifiedFileLevelSymbol(d->file(), ClassName(d)); }

std::string QualifiedClassName(const gpb::EnumDescriptor* d) {
   return QualifiedFileLevelSymbol(d->file(), ClassName(d));
}

bool is_recursive(const gpb::Descriptor* containing_descriptor, const gpb::Descriptor* descriptor) {
   while (containing_descriptor != descriptor) {
      if (containing_descriptor == nullptr)
         return false;
      containing_descriptor = containing_descriptor->containing_type();
   }
   return true;
}

std::string full_name_to_cpp_name(std::string fullname) {
   std::string result;
   size_t pos = 0;
   size_t dot_pos;
   while ((dot_pos = fullname.find('.', pos)) != std::string::npos) {
      result += reserve_keyword(fullname.substr(pos, dot_pos - pos));
      result += "::";
      pos = dot_pos + 1;
   }

   result += reserve_keyword(fullname.substr(pos));
   return result;
}

struct zpp_generator {
   gpbc::CodeGeneratorResponse response;
   gpb::DescriptorPool         pool;
   std::stringstream           epilogue_strm;

   struct message_info {
      std::vector<std::pair<uint32_t, uint32_t>>                  pb_map;
      std::vector<uint32_t>                                       unique_ptr_fields;
      std::map<const gpb::Descriptor*, const gpb::Descriptor*>    dependencies;
      std::vector<std::pair<const gpb::Descriptor*, std::string>> messages;
      std::stringstream                                           strm;
   };

   zpp_generator(const gpbc::CodeGeneratorRequest& request) {
      for (int i = 0; i < request.proto_file_size(); ++i) {
         auto file_desc_proto = request.proto_file(i);
         pool.BuildFile(file_desc_proto);
      }
   }

   void generate_enum(const gpb::EnumDescriptor* descriptor, std::stringstream& strm, std::string indent = "") {
      
      auto name = reserve_keyword(descriptor->name());
      strm << indent << "enum " << name << " : int {\n";
      int min_value = descriptor->value(0)->number();
      int max_value = descriptor->value(0)->number();

      if ( descriptor->value_count()) {
         for (int i = 0; i < descriptor->value_count(); ++i) {
            auto value = descriptor->value(i);
            if (i != 0)
               strm << ",\n";
            strm << indent << "   " << reserve_keyword(value->name());
            if (descriptor->options().deprecated())
               strm << " [[deprecated]]";
            strm << " = " << value->number();

            min_value = std::min(min_value,  value->number());
            max_value = std::max(max_value,  value->number());
         }     
      }
      strm << "\n" << indent << "};\n\n";

      if (min_value < -128 || max_value > 128 ||  (max_value-min_value) > UINT16_MAX) {
         epilogue_strm << "\n" << "template <>\n"
                       <<  "struct magic_enum::customize::enum_range<" << full_name_to_cpp_name(descriptor->full_name()) << "> {\n"
                       << "   static constexpr int min = " << min_value << ";\n"
                       << "   static constexpr int max = " << max_value << ";\n"
                       << "};\n";
      }
   }

   std::string field_type_name(const gpb::Descriptor* containing_descriptor, const gpb::FieldDescriptor* descriptor,
                               std::map<const gpb::Descriptor*, const gpb::Descriptor*>& dependencies) {

      std::string result;
      bool can_be_optional = true;

      switch (descriptor->type()) {
      case gpb::FieldDescriptor::TYPE_INT32:
         result = "zpp::bits::vint32_t";
         break;
      case gpb::FieldDescriptor::TYPE_INT64:
         result = "zpp::bits::vint64_t";
         break;
      case gpb::FieldDescriptor::TYPE_UINT32:
         result = "zpp::bits::vuint32_t";
         break;
      case gpb::FieldDescriptor::TYPE_UINT64:
         result = "zpp::bits::vuint64_t";
         break;
      case gpb::FieldDescriptor::TYPE_SINT32:
         result = "zpp::bits::vsint32_t";
         break;
      case gpb::FieldDescriptor::TYPE_SINT64:
         result = "zpp::bits::vsint64_t";
         break;
      case gpb::FieldDescriptor::TYPE_FIXED32:
         result = "uint32_t";
         break;
      case gpb::FieldDescriptor::TYPE_FIXED64:
         result = "uint64_t";
         break;
      case gpb::FieldDescriptor::TYPE_SFIXED32:
         result = "int32_t";
         break;
      case gpb::FieldDescriptor::TYPE_SFIXED64:
         result = "int64_t";
         break;
      case gpb::FieldDescriptor::TYPE_FLOAT:
         result = "float";
         break;
      case gpb::FieldDescriptor::TYPE_DOUBLE:
         result = "double";
         break;
      case gpb::FieldDescriptor::TYPE_BOOL:
         result = "bool";
         break;
      case gpb::FieldDescriptor::TYPE_ENUM: {
         auto field = descriptor->enum_type();
         result     = (containing_descriptor == field->containing_type()) ? reserve_keyword(field->name())
                                                                          : QualifiedClassName(field);
      } break;
      case gpb::FieldDescriptor::TYPE_STRING:
         result = "std::string";
         break;
      case gpb::FieldDescriptor::TYPE_BYTES:
         result = "std::vector<char>";
         break;
      case gpb::FieldDescriptor::TYPE_MESSAGE: {
         auto field = descriptor->message_type();
         if (descriptor->is_map()) {
            can_be_optional = false;
            result = "std::map<" + field_type_name(containing_descriptor, field->map_key(), dependencies) + "," +
                     field_type_name(containing_descriptor, field->map_value(), dependencies) + ">";
         } else {
            bool nested_type = containing_descriptor == field->containing_type();
            bool recursive = is_recursive(containing_descriptor, field);
            if (nested_type) {
               result = reserve_keyword(field->name());
            } else {
               result = QualifiedClassName(field);
               if (!recursive) {
                  dependencies.try_emplace(field, containing_descriptor);
               }
            }
            if (recursive && !descriptor->is_repeated()) {
               result = "std::unique_ptr<" + result + ">";
               can_be_optional = false;
            }
         }
      } break;
      default:
         assert(false);
      }

      if (descriptor->options().HasExtension(zpp_type)) {
         result = descriptor->options().GetExtension(zpp_type);
         can_be_optional = true;
      }

      if (descriptor->is_repeated()) {
         result = "std::vector<" + result + ">";
         if (descriptor->is_packable() && !descriptor->is_packed()) {
            response.set_error("unpacked repeated field is not supported yet.");
         }
      } else if (can_be_optional && descriptor->options().HasExtension(zpp_optional) && descriptor->options().GetExtension(zpp_optional)) {
         result = "std::optional<" + result + ">";
      }
      return result;
   }

   void generate_field(const gpb::Descriptor* containing_descriptor, const gpb::FieldDescriptor* descriptor,
                       size_t index, message_info& info, std::string indent) {
      auto field_type = field_type_name(containing_descriptor, descriptor, info.dependencies);
      if (index + 1 != descriptor->number()) {
         info.pb_map.emplace_back(index + 1, descriptor->number());
      }
      if (field_type.find("std::unique_ptr<") == 0)
         info.unique_ptr_fields.push_back(index);

      std::string attribute;
      if (descriptor->options().deprecated())
         attribute = "[[deprecated]] ";

      info.strm << indent << "   " << attribute << field_type << " " << reserve_keyword(descriptor->name())
                << " = {};\n";
   }

   void generate_message(const gpb::Descriptor* descriptor, message_info& info, std::string indent = "") {
      std::string message_name = reserve_keyword(descriptor->name());

      if (descriptor->oneof_decl_count()) {
         response.set_error("oneof field is not supported yet.");
         return;
      }

      auto& strm = info.strm;

      strm << indent << "struct " << message_name << " {\n";

      for (size_t i = 0; i < descriptor->enum_type_count(); ++i)
         generate_enum(descriptor->enum_type(i), strm, indent + "   ");

      for (size_t i = 0; i < descriptor->nested_type_count(); ++i) {
         generate_message(descriptor->nested_type(i), info.dependencies, info.messages, indent + "   ");
      }

      for (auto [_, msg] : info.messages) {
         strm << msg;
      }

      for (size_t i = 0; i < descriptor->field_count(); ++i) {
         generate_field(descriptor, descriptor->field(i), i, info, indent);
      }

      if (info.pb_map.size()) {
         strm << indent << "   using pb_options = std::tuple<\n";
         int i = 0;
         for (auto [index, field_number] : info.pb_map) {
            if (i++ != 0)
               strm << ",\n";
            strm << indent << "      zpp::bits::pb_map<" << index << "," << field_number << ">";
         }
         strm << ">;\n";
      }

      if (descriptor->field_count()) {
         if (info.unique_ptr_fields.size() == 0)
            strm << indent << "   bool operator == (const " << message_name << "&) const = default;\n";
         else {
            strm << indent << "   bool operator == (const " << message_name << "& other) const {\n";
            auto itr = info.unique_ptr_fields.begin();
            for (int i = 0; i < descriptor->field_count(); ++i) {
               if (i != 0 ) strm << " &&\n";
               if (itr == info.unique_ptr_fields.end() || i < *itr )
                  strm << indent << "      " << reserve_keyword(descriptor->field(i)->name()) << " == other." << reserve_keyword(descriptor->field(i)->name());
               else {
                  strm << indent << "      ((" << reserve_keyword(descriptor->field(i)->name()) << " == other." << reserve_keyword(descriptor->field(i)->name()) 
                       << ") || (*" << reserve_keyword(descriptor->field(i)->name()) << " == *other." << reserve_keyword(descriptor->field(i)->name()) << "))";
                  itr++;
               }
            }
            strm << "\n" << indent << "   };\n";
         }

         strm << indent << "   EOSIO_FRIEND_REFLECT(" << message_name ;
         for (size_t i = 0; i < descriptor->field_count(); ++i) 
            strm << ", " << reserve_keyword(descriptor->field(i)->name());
         strm << ")\n";
      }
      strm << indent << "}; // struct " << message_name << "\n\n";
   }

   void generate_message(const gpb::Descriptor*                                       descriptor,
                         std::map<const gpb::Descriptor*, const gpb::Descriptor*>&    dependencies,
                         std::vector<std::pair<const gpb::Descriptor*, std::string>>& messages, std::string indent) {
      message_info info;
      generate_message(descriptor, info, indent);
      auto depended_itr = dependencies.find(descriptor);
      auto insertion_point =
          depended_itr != dependencies.end()
              ? std::find_if(messages.begin(), messages.end(),
                             [depended = depended_itr->second](auto e) { return e.first == depended; })
              : messages.end();
      messages.insert(insertion_point, std::make_pair(descriptor, info.strm.str()));

      for (auto [d, _] : info.dependencies)
         dependencies.try_emplace(d, descriptor);
   }

   void generate_file(const gpb::FileDescriptor* proto_file, gpbc::CodeGeneratorResponse_File* generated) {
      if (proto_file->syntax() != gpb::FileDescriptor::SYNTAX_PROTO3) {
         response.set_error(proto_file->name() + ": only syntax 3 is supported");
         return;
      }
      auto name = proto_file->name();
      // replace ".proto" with ".pb.hpp"
      name = strip_proto(name) + ".pb.hpp";

      generated->set_name(name);

      std::stringstream strm;
      strm << "///\n"
           << "/// Generated from protoc with zpp-proto-plugin, DO NOT modify\n"
           << "///\n"
           << "#pragma once\n";

      for (int i = 0; i < proto_file->dependency_count(); i++) {
         const gpb::FileDescriptor* dep = proto_file->dependency(i);
         if ( dep->package().find("google.") == 0 )
            continue;
         if (dep->enum_type_count()> 0 || dep->message_type_count() > 0)
            strm << "#include \"" << strip_proto(dep->name()) << ".pb.h\"\n";
      }

      strm << "#include <zpp_bits.h>\n"
           << "#include <eosio/reflection.hpp>\n"
           << "// @@protoc_insertion_point(includes)\n";

      std::string indent = "";
      std::string ns     = DotsToColons(proto_file->package());
      if (ns.size()) {
         strm << "namespace " << ns << " {\n";
      }

      for (int i = 0; i < proto_file->enum_type_count(); ++i)
         generate_enum(proto_file->enum_type(i), strm, indent);

      std::map<const gpb::Descriptor*, const gpb::Descriptor*>    dependencies;
      std::vector<std::pair<const gpb::Descriptor*, std::string>> messages;

      for (int i = 0; i < proto_file->message_type_count() && !response.has_error(); ++i) {
         generate_message(proto_file->message_type(i), dependencies, messages, indent);
      }

      for (auto [_, msg] : messages) {
         strm << msg;
      }

      if (ns.size()) {
         strm << "} // namespace " << ns << "\n";
      }

      strm  << epilogue_strm.str();
      generated->set_content(strm.str());
   }

   void generate_file(std::string filename) { generate_file(pool.FindFileByName(filename), response.add_file()); }
};

int main(int argc, const char** argv) {
   gpbc::CodeGeneratorRequest request;
   request.ParseFromIstream(&std::cin);

   zpp_generator generator(request);

   for (int i = 0; i < request.file_to_generate_size(); ++i) {
      generator.generate_file(request.file_to_generate(i));
   }

   generator.response.SerializePartialToOstream(&std::cout);
   return 0;
}
