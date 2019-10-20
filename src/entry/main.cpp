#include <iostream>
#include <fstream>
#include <string>

#include <boost/program_options.hpp>

#include "../codegen/codegen.h"
#include "../parser/parser.hpp"
#include "../dumper/ast_to_json.h"
#include "../artery.h"

namespace po = boost::program_options;

using namespace parser;

#define OPT_ENTRY "entry"
#define OPT_PRETTY_PRINT "pretty-print"
#define OPT_TOLERANT "tolerant"

int main(int argc, char** argv) {
    std::string entry_file;

    po::options_description desc("Allowed options");
    desc.add_options()
        (OPT_ENTRY, po::value<std::string>(&entry_file)->required(), "entry file of bundler")
        (OPT_PRETTY_PRINT, "print the code generated by bundler")
        (OPT_TOLERANT, "tolerant parsing error")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    try {
        auto src = make_shared<UString>();

        (*src) = Artery::ReadFileStream(entry_file);

        ParserCommon::Config config = ParserCommon::Config::Default();
        config.tolerant = vm.count(OPT_TOLERANT);

        Parser parser(src, config);
        auto script = parser.ParseScript();

        if (vm.count(OPT_PRETTY_PRINT)) {
            CodeGen codegen;
            codegen.TraverseNode(script);
        } else {
            auto json_result = dumper::AstToJson::Dump(script);
            std::cout << json_result.dump(2) << std::endl;
        }
    } catch (ParseError& err) {
        std::cerr << err.ErrorMessage() << std::endl;
        return 1;
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
