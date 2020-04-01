# Jetpack

`jetpack.js` is an extremely fast js bundler and minifier.

`jetpack.js` a well designed tool. It's modulize into parser and bundler.
The parser can be used as a library independently.

- [Why](#why)
- [Features](#Features)
  - [Parser](#parser)
  - [Bundler](#bundler)
- [Usage](#usage)
- [Use the parser as a library](#use-the-parser-as-a-library)
  - [Example](#example)
- [Performance](#performance)
- [Architecture](#architecture)
- [Compatibility](#compatibility)
- [Platform](#platform)

# Why

Several months ago, I wrote a js parser in C++. Many parsers embedded in other js engine have a lot of dependencies on their runtime. It's very hard to separate them. So I decided to write my own parser. After I finished, I found my parser is almost 10x faster than the common parsers running on Node.js (without heating). But I didn't move on after that. This year(2020), Evan Wallace's project [esbuild](https://github.com/evanw/esbuild/) inspires me. So I decided to write a bundler in C++. I want to know if Golang's performance could beat C++.

# Features

## Parser

- Can be used standalone
- Implemented in C++ with excellent performance
- Full support for ECMAScript 2017([ECMA-262 8th Edition](http://www.ecma-international.org/publications/standards/Ecma-262.htm))
- JSON output of sensible [syntax tree](https://github.com/estree/estree/blob/master/es5.md) format as standardized by [ESTree project](https://github.com/estree/estree)
- Experimental support for [JSX](https://facebook.github.io/jsx/), a syntax extension for [React](https://facebook.github.io/react/)
- Syntax node location (index-based and line-column)
- Friendly error message

## Bundler

- Module resolution.
- Bundle a ES project into a single file.
- Scope hoisting.
- Minify the code.
- Sourcemap generation(WIP).
- Dead Code Elimination(WIP).

# Usage

Use command line to bundle a js module.
```shell script
jetpack main.js --out bundle.js
```

Help command:

```shell script
jetpack --help
Jetpack command line
Usage:
  Jetpack [OPTION...] positional parameters

      --tolerant            tolerant parsing error
      --jsx                 support jsx syntax
      --help                produce help message
      --analyze-module arg  analyze a module and print result
      --no-trace            do not trace ref file when analyze module
      --minify              minify the code
      --out arg             output filename of bundle
```

# Use the parser as a library

jetpack.js is built with CMake, so it can be
easily integrated to your project.

```cmake
add_subdirectory(js-parser)
target_include_directories(${PROJECT_NAME} ./js-parser/src)
target_link_libraries(${PROJECT_NAME} PUBLIC js-parser)
```

## Example

```cpp
Parser parser(src, config);
ParserContext::Config config = ParserContext::Config::Default();
auto src = std::make_shared<UString>();
(*src) = ModuleResolver::ReadFileStream(mf->path);
auto ctx = std::make_shared<ParserContext>(src, config);
Parser parser(ctx);

auto script = parser.ParseScript();
// or
auto module = parser.ParseModule();

```

# Performance

ZEP(release version)'s parsing speed would be nearly 1x faster than
other ES parsers implemented in ES(Running on Node.js).

With the power of jemalloc,
Jetpack's parser's performance is equal to other ES parsers implemented in Rust.

# Architecture

![](./images/Rocket-Bundle-Arch.png)

The code are well commented, please read the code.

# Compatibility

The json output of ZEP would as same as esprima. So I think maybe ZEP can be
a faster alternative to some ES parsers.

And the WASM version is in the roadmap. The web version of ZEP would be released ASAP.

# Platform

`jetpack.js` supports all popular system including:

- macOS
- Windows
- Linux
