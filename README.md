# cpp_libs
Some useful non-standard independent C++ functions.

# Requirements
Just any C++11-compatible compiler.

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
