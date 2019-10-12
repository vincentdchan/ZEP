//
// Created by Duzhong Chen on 2019/10/10.
//

#pragma once

#include "token.h"
#include <utility>

struct Comment {
    bool multi_line_;
    UString value_;
    std::pair<std::uint32_t, std::uint32_t> range_;
    SourceLocation loc_;

    void* operator new(std::size_t size);
    void operator delete(void* chunk);
};
