//
// Created by Duzhong Chen on 2021/3/30.
//

#pragma once

#include <string>
#include <cstdlib>
#include <cstring>
#include "parser/Config.h"
#include "codegen/CodeGenConfig.h"
#include "utils/JetFlags.h"

enum class JetpackFlag {
    Jsx = 0x1,
    ConstantFolding = 0x2,
    Minify = 0x100,
    TraceFile = 0x10000,
    Sourcemap = 0x20000,
    Library = 0x40000,
    Profile = 0x1000000,
    ProfileMalloc = 0x2000000,
};

JET_DECLARE_FLAGS(JetpackFlags, JetpackFlag)

namespace jetpack { namespace simple_api {

    int AnalyzeModule(const std::string& path,
                      JetpackFlags flags,
                      const std::string& base_path="");

    int BundleModule(const std::string& path,
                     const std::string& out_path,
                     JetpackFlags flags,
                     const std::string& base_path="");

    std::string ParseAndCodeGen(std::string_view content,
                                const jetpack::parser::Config& config,
                                const jetpack::CodeGenConfig& code_gen_config);

    int HandleCommandLine(int argc, char** argv);

}}
