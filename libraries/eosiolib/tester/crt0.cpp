
#include <eosio/tester.hpp>
int main(int argc, char** argv);

extern "C" __attribute__((export_name("start"), visibility ("default"))) int start(void (*f)()) {
  std::vector<std::string> args = eosio::get_args();
   char buf[] = "eosio-tester";
   std::vector<char*> argv;
   argv.push_back(buf);
   for(std::string& s : args) {
      argv.push_back(const_cast<char*>(s.c_str()));
   }
   return main(argv.size(), argv.data());
}