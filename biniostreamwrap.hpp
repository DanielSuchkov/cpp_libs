#pragma once

#include <istream>

#include "binistreamwrap.hpp"
#include "binostreamwrap.hpp"


class BinIOStreamWrap
        : public BinIStreamWrap,
          public BinOStreamWrap
{
public:
    BinIOStreamWrap(std::iostream &iostr, UseExceptions useExceptions = UseExceptions::yes) :
        BinIStreamWrap(iostr, useExceptions),
        BinOStreamWrap(iostr)
    { }
};
