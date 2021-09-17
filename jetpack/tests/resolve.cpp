//
// Created by Duzhong Chen on 2020/3/24.
//

#include <gtest/gtest.h>
#include <unordered_set>
#include <sstream>
#include <parser/ParserContext.h>
#include <parser/Parser.hpp>
#include "codegen/CodeGen.h"
#include "UniqueNameGenerator.h"
#include "ModuleResolver.h"

using namespace jetpack;
using namespace jetpack::parser;

/**
 * do not generate duplicate var name
 */
TEST(MinifyNameGenerator, Next) {
    auto gen = MinifyNameGenerator::Make();
    std::unordered_set<std::string> gen_set;

    for (int i = 0; i < 10000; i++) {
        auto next_str_opt = gen->Next("");
        EXPECT_TRUE(next_str_opt.has_value());
//        std::cout << utils::To_UTF8(next_str) << std::endl;
        EXPECT_TRUE(gen_set.find(*next_str_opt) == gen_set.end());
        gen_set.insert(*next_str_opt);
    }
}

inline std::string ReplaceDefault(std::string_view src) {
    auto resolver = std::make_shared<ModuleResolver>();
    auto mod = std::make_shared<ModuleFile>("memory0", -1);
    mod->module_resolver = resolver;

    Config config = Config::Default();
    auto ctx = std::make_shared<ParserContext>(mod->id(), src, config);
    Parser parser(ctx);

    mod->ast = parser.ParseModule();
    resolver->ReplaceExports(mod);

    CodeGenConfig code_gen_config;
    CodeGen codegen(code_gen_config, nullptr);
    codegen.Traverse(*mod->ast);

    return codegen.GetResult().content;
}

TEST(ModuleResolver, HandleExportDefault) {
    std::string src = "export default a = 3;";

    auto result = ReplaceDefault(src);

    EXPECT_EQ(result, "var _default = a = 3;\n");
}

TEST(ModuleResolver, HandleExportDefaultFunction1) {
    std::string src = "export default function() {\n"
                      "}";

    auto result = ReplaceDefault(src);

    EXPECT_EQ(result,
              "function _default() {}\n");
}

TEST(ModuleResolver, HandleExportDefaultFunction2) {
    std::string src = "export default function name() {\n"
                      "}";

    auto result = ReplaceDefault(src);

    EXPECT_EQ(result,
              "function name() {}\n"
              "var _default = name;\n");
}

TEST(ModuleResolver, HandleExportDefaultLiteral) {
    std::string src = "export default 3;\n";

    auto result = ReplaceDefault(src);

    EXPECT_EQ(result,
              "var _default = 3;\n");
}

TEST(ModuleResolver, HandleExportDefaultLiteral2) {
    std::string src = "export default `3`;\n";

    auto result = ReplaceDefault(src);

    EXPECT_EQ(result,
              "var _default = `3`;\n");
}

TEST(ModuleResolver, HandleExportDefaultLiteral3) {
    std::string src = "export default `\n"
                      "aaaabb\n"
                      "ddd`;\n";

    auto result = ReplaceDefault(src);

    EXPECT_EQ(result,
              "var _default = `\n"
              "aaaabb\n"
              "ddd`;\n");
}

TEST(ModuleResolver, SingleMemoryFile) {
    std::string buffer =
            "function hello(world) {\n"
            "  console.log(world);\n"
            "}\n"
            "hello('world');\n";

    bool jsx = true;
    bool minify = true;

    auto resolver = std::make_shared<ModuleResolver>();
    CodeGenConfig codegen_config;
    parser::Config parser_config = parser::Config::Default();

    if (jsx) {
        parser_config.jsx = true;
        parser_config.transpile_jsx = true;
    }

    if (minify) {
        parser_config.constant_folding = true;
        codegen_config.minify = true;
        codegen_config.comments = false;
        resolver->SetNameGenerator(MinifyNameGenerator::Make());
    }

    resolver->SetTraceFile(false);
    resolver->BeginFromEntryString(parser_config, buffer);

    auto final_export_vars = resolver->GetAllExportVars();
    if (minify) {
        resolver->RenameAllInnerScopes();
    }
    resolver->RenameAllRootLevelVariable();

    auto entry_mod = resolver->GetEntryModule();
    CodeGen codegen(codegen_config);
    codegen.Traverse(*entry_mod->ast);

    std::cout << codegen.GetResult().content << std::endl;
}

//TEST(ModuleResolver, HandleExportDefaultLiteral4) {
//    std::string src = "export default /* glsl */`\n"
//                      "#ifdef USE_ALPHAMAP\n"
//                      "\n"
//                      "\tdiffuseColor.a *= texture2D( alphaMap, vUv ).g;\n"
//                      "\n"
//                      "#endif\n"
//                      "`;";
//
//    auto result = ReplaceDefault(src);
//
//    std::cout << result << std::endl;
//}
