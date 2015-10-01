# cpp_libs
Some useful non-standard independent C++ functions.

# Requirements
Just any C++11-compatible compiler (and somtimes boost).

# Description
Every header is independent, so you can just copy one of them and use.

## carry.cpp
If you have somewhere in code

    class Foo {
        ...
        void foo();
        ...
    };
    
and want to use it as `std::function<void ()>`, just write

    auto fn = carry(&Foo::foo, foo_ptr);

unlike `std::bind` you don't need to specify all `Foo::foo` arguments.
Note that you can carry as much args as you want.

## narrow_cast.hpp
It guarantee that you won't lose data on narrowing casts.

## iterator.hpp
Some Rust-like iterator functionality. Requires boost.

## binstreamwrap.hpp
Wrapper for `std::(i|o)stream`'s children, that reads/writes binary data. Just for specific `operator <</>>` overloads. There is some built-in overloads for commonly used `std::` types.
Example:

    writing:
    std::ofstream ouf("data.bin", std::ios::binary);
    BinOStreamWrap bouf(ouf);
    std::vector<int32_t> v = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
    bouf << v;
    
    reading:
    std::ifstream inf("data.bin", std::ios::binary);
    BinOStreamWrap binf(inf);
    ... and then use usual stream syntax
    std::vector<int32_t> v;
    binf >> v;
    ... or read_val function
    auto v = read_val<std::vector<int32_t>>(binf);

_Note:_ if you want to read and write files on platforms with different bit depth - you must use exact-width integer types from `cstdint.h`.

## binstreamwrapfwd.hpp
Forward-declarations, nothing else.
