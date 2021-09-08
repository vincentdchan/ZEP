//
// Created by Duzhong Chen on 2020/3/26.
//

#include <memory>
#include <gtest/gtest.h>
#include <parser/Parser.hpp>

#include "codegen/CodeGen.h"

using namespace jetpack;
using namespace jetpack::parser;

inline Sp<Module> ParseString(const std::string& src) {
    auto u16src = UStringFromUtf8(src.c_str(), src.size());
    ParserContext::Config config = ParserContext::Config::Default();
    auto ctx = std::make_shared<ParserContext>(-1, u16src, config);
    Parser parser(ctx);
    return parser.ParseModule();
}

inline std::string GenCode(Sp<Module>& mod) {
    CodeGen::Config code_gen_config;
    CodeGen codegen(code_gen_config, nullptr);
    codegen.Traverse(mod);
    return UStringToUtf8(codegen.GetResult().content);
}

TEST(Scope, Collect) {
    UString content(u"var name = 3;");

    ParserContext::Config config = ParserContext::Config::Default();
    auto ctx = std::make_shared<ParserContext>(-1, content, config);
    Parser parser(ctx);

    auto mod = parser.ParseModule();
    mod->scope->ResolveAllSymbols(nullptr);

    EXPECT_EQ(mod->scope->own_variables.size(), 1);
    EXPECT_TRUE(mod->scope->own_variables.find(u"name") != mod->scope->own_variables.end());
}

TEST(Scope, Rename) {
    std::string src = "var name = 3;\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back(u"name", u"new_name");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(mod->scope->own_variables.size(), 1);
    EXPECT_TRUE(mod->scope->own_variables.find(u"name") == mod->scope->own_variables.end());

    EXPECT_EQ(GenCode(mod), "var new_name = 3;\n");
}

TEST(Scope, RenameImportNamespace) {
    std::string src = "import * as name from 'main';\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back(u"name", u"new_name");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(mod->scope->own_variables.size(), 1);
    EXPECT_TRUE(mod->scope->own_variables.find(u"name") == mod->scope->own_variables.end());

    EXPECT_EQ(GenCode(mod), "import * as new_name from 'main';\n");
}

TEST(Scope, RenameFunction1) {
    std::string src = "var name = 3;\n"
                      "function ok() {\n"
                      "  console.log(name);\n"
                      "}\n";

    std::string expected = "var new_name = 3;\n"
                           "function ok() {\n"
                           "  console.log(new_name);\n"
                           "}\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back(u"name", u"new_name");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_TRUE(mod->scope->own_variables.find(u"name") == mod->scope->own_variables.end());

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameFunction2) {
    std::string src = "var name = 3;\n"
                      "function ok() {\n"
                      "  console.log(name);\n"
                      "}\n";

    std::string expected = "var name = 3;\n"
                           "function ok1() {\n"
                           "  console.log(name);\n"
                           "}\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back(u"ok", u"ok1");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameFunction3) {
    std::string src = "var name = 3;\n"
                      "function ok(name) {\n"
                      "  console.log(name);\n"
                      "}\n";

    std::string expected = "var rename = 3;\n"
                           "function ok(name) {\n"
                           "  console.log(name);\n"
                           "}\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back(u"name", u"rename");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameObjectPattern) {
    std::string src = "var { name: other } = obj;\n";

    std::string expected = "var { name: renamed } = obj;\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back(u"other", u"renamed");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameObjectPattern2) {
    std::string src = "var { name: other } = obj;\n";

    std::string expected = "var { name: other } = obj;\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back(u"name", u"ok1");
    EXPECT_FALSE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameObjectPattern3) {
    std::string src = "var { name } = obj;\n";

    std::string expected = "var { name: renamed } = obj;\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back(u"name", u"renamed");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, Cls) {
    std::string src = "const print = 'hello world';\n"
                      "class A {"
                      "  print() {\n"
                      "    console.log(print);\n"
                      "  }\n"
                      "}\n";

    std::string expected = "const renamed = 'hello world';\n"
                           "class A {\n"
                           "  print() {\n"
                           "    console.log(renamed);\n"
                           "  }\n"
                           "}\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back(u"print", u"renamed");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameImport) {
    std::string src = "import { name } from 'main';\n";

    std::string expected = "import { name as renamed } from 'main';\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back(u"name", u"renamed");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameImport3) {
    std::string src = "import { a, fun, ooo } from './a';\n"
                      "\n"
                      "var b = 44444;\n"
                      "\n"
                      "export default a + 3 + b + fun();";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back(u"a", u"p");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_GT(mod->body.size(), 0);
    auto import_decl = std::dynamic_pointer_cast<ImportDeclaration>(mod->body[0]);
    EXPECT_NE(import_decl, nullptr);

    auto first_spec = std::dynamic_pointer_cast<ImportSpecifier>(import_decl->specifiers[0]);
    EXPECT_NE(import_decl, nullptr);

    EXPECT_EQ(first_spec->local->name, u"p");
}

TEST(Scope, RenameImportDefault) {
    std::string src = "import React from 'react';\n";

    std::string expected = "import Angular from 'react';\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back(u"React", u"Angular");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameImport2) {
    std::string src = "import { cc as name } from 'main';\n"
                      "console.log(name);\n";

    std::string expected = "import { cc as renamed } from 'main';\n"
                           "console.log(renamed);\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back(u"name", u"renamed");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameExport1) {
    std::string src = "const name = 3;\n"
                      "export { name as foo };\n";

    std::string expected = "const renamed = 3;\n"
                           "export { renamed as foo };\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back(u"name", u"renamed");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}

TEST(Scope, RenameExport2) {
    std::string src = "const name = 3;\n"
                      "export { name };\n";

    std::string expected = "const renamed = 3;\n"
                           "export { renamed as name };\n";

    auto mod = ParseString(src);
    mod->scope->ResolveAllSymbols(nullptr);

    ModuleScope::ChangeSet changeset;
    changeset.emplace_back(u"name", u"renamed");
    EXPECT_TRUE(mod->scope->BatchRenameSymbols(changeset));

    EXPECT_EQ(GenCode(mod), expected);
}
