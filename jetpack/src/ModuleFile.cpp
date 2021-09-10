//
// Created by Duzhong Chen on 2021/3/25.
//

#include "ModuleFile.h"
#include "ModuleResolver.h"
#include "Benchmark.h"

namespace jetpack {

    ModuleFile::ModuleFile(const std::string& path, int32_t id): path_(path), id_(id) {
        mapping_collector_ = std::make_shared<MappingCollector>();
    }

    void ModuleFile::RenameInnerScopes(RenamerCollection& renamer) {
        std::vector<Sp<MinifyNameGenerator>> result;
        result.reserve(ast->scope->children.size());
        for (auto child : ast->scope->children) {
            result.push_back(RenameInnerScopes(*child, renamer.idLogger.get()));
        }

        auto final = MinifyNameGenerator::Merge(result);
        renamer.PushGenerator(final);
    }

    Sp<MinifyNameGenerator> ModuleFile::RenameInnerScopes(Scope &scope, UnresolvedNameCollector* idLogger) {
        std::vector<Sp<MinifyNameGenerator>> temp;
        temp.reserve(scope.children.size());

        for (auto child : scope.children) {
            temp.push_back(RenameInnerScopes(*child, idLogger));
        }

        std::vector<std::tuple<std::string, std::string>> renames;
        auto renamer = MinifyNameGenerator::Merge(temp);

        for (auto& variable : scope.own_variables) {
            auto new_opt = renamer->Next(variable.first);
            if (new_opt.has_value()) {
                renames.emplace_back(variable.first, *new_opt);
            }
        }

        scope.BatchRenameSymbols(renames);

        return renamer;
    }

    UString ModuleFile::GetModuleVarName() const {
        std::string tmp = "mod_" + std::to_string(id());
        return UStringFromUtf8(tmp.c_str(), tmp.size());
    }

    bool ModuleFile::GetSource(WorkerError& error) {
        J_ASSERT(provider);
        benchmark::BenchMarker marker(benchmark::BENCH_READING_IO);
        auto result =  provider->resolve(*this, Path());
        if (unlikely(result.HasError())) {
            marker.Submit();

            error = *result.error;

            return false;
        } else {
            std::string tmp;
            result.value.swap(tmp);
            src_content = StringWithMapping::Make(std::move(tmp));
        }
        marker.Submit();
        return true;
    }


}
