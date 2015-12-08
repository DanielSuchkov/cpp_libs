#pragma once

#include <istream>
#include <ostream>
#include <vector>
#include <list>
#include <string>
#include <tuple>
#include <cstdint>
#include <type_traits>
#include <exception>
#ifdef QT_VERSION
#   include <QString>
#endif // QT_VERSION
#include "binstreamwrapfwd.hpp"

namespace fcl {
#ifdef _MSC_VER
#   define noexcept
#endif


class ReadingAtEOF : public std::exception
{
public:
    virtual const char *what() const noexcept override final {
        return "attempt to read at eof";
    }
};

#ifdef _MSC_VER
#   undef noexcept
#endif


enum class UseExceptions {
    yes, no
};

namespace details {
    template <size_t I>
    struct reader {
        template<class StreamTy, typename... Tp>
        friend auto read_tuple(BinIStreamWrap<StreamTy> &is, std::tuple<Tp ...> &tpl) {
            is >> std::get<I>(tpl);
            reader< sizeof...(Tp) - I - 1 >::read_tuple(is, tpl);
            return is;
        }
    };

    template <>
    struct reader<0> {
        template<class StreamTy, typename... Tp>
        friend auto read_tuple(BinIStreamWrap<StreamTy> &is, std::tuple<Tp ...> &) {
            return is;
        }
    };
}
//namespace from {
//    struct begin_t {} begin;
//    struct end_t {} end;
//}

template <class StreamTy>
class BinIStreamWrap {
public:
    /*!
     * \brief BinIStreamWrap
     * \param istr Input stream opend with std::ios::binary flag
     * \param useExceptions UseExceptions::yes if you want that this class notify you about reading at eof using exceptions and UseExceptions::no if no
     */
    explicit BinIStreamWrap(StreamTy &istr,
                            UseExceptions useExceptions = UseExceptions::yes)
        : m_istr(istr)
        , m_useExceptions(useExceptions == UseExceptions::yes) {}

    int64_t get_ipos() const {
        return m_istr.tellg();
    }

    void set_ipos(int64_t pos) const {
        m_istr.seekg(pos);
    }

    void goto_iend() {
        m_istr.seekg(0, m_istr.end);
    }

    void goto_ibegin() {
        m_istr.seekg(0, m_istr.beg);
    }

    template <typename T>
    friend BinIStreamWrap &operator >>(BinIStreamWrap &is, T &t) {
        static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
        is.m_istr.read(reinterpret_cast< char *>(&t), sizeof(t));
        if (is.m_istr.eof() && is.m_useExceptions) {
            throw ReadingAtEOF();
        }
        return is;
    }

    template <typename T, typename Alloc>
    friend BinIStreamWrap &operator >>(BinIStreamWrap &is, std::vector< T, Alloc > &vec) {
        uint64_t size;
        is >> size;
        vec.resize(static_cast< size_t >(size));
        is.m_istr.read(reinterpret_cast< char *>(vec.data()), static_cast< size_t >(size) * sizeof(T));
        if (is.m_istr.eof() && is.m_useExceptions) {
            throw ReadingAtEOF();
        }
        return is;
    }

    template <typename T, typename Alloc>
    friend BinIStreamWrap &operator >>(BinIStreamWrap &is, std::list< T, Alloc > &list) {
        uint64_t size;
        is >> size;
        list.resize(static_cast< size_t >(size));

        for (auto &el: list) {
            is >> el;
        }

        return is;
    }

    template <typename CharT, typename Traits, typename Alloc>
    friend BinIStreamWrap &operator >>(
            BinIStreamWrap &is,
            std::basic_string< CharT, Traits, Alloc > &s) {
        uint64_t size;
        is >> size;
        s.resize(static_cast< size_t >(size));
        is.m_istr.read(
            reinterpret_cast< char *>(&s.front()),
            sizeof(CharT) * static_cast< size_t >(size)
        );

        if (is.m_istr.eof() && is.m_useExceptions) {
            throw ReadingAtEOF();
        }
        return is;
    }

#ifdef QT_VERSION
    friend BinIStreamWrap &operator >>(BinIStreamWrap &is, QString &str) {
        int32_t size;
        is >> size;
        str.resize(static_cast< int >(size));
        is.m_istr.read(
            reinterpret_cast< char *>(str.data()),
            static_cast< size_t >(size) * sizeof(QChar)
        );

        if (is.m_istr.eof() && is.m_useExceptions) {
            throw ReadingAtEOF();
        }
        return is;
    }
#endif // QT_VERSION

    template <typename T>
    friend BinIStreamWrap &operator >>(
            BinIStreamWrap &is,
            std::pair< T *, uint64_t > &cArr) {
        is >> cArr.second;
        cArr.first = new T[cArr.second];
        is.m_istr.read(reinterpret_cast< char *>(cArr.first), sizeof(T) * cArr.second);
        if (is.m_istr.eof() && is.m_useExceptions) {
            throw ReadingAtEOF();
        }
        return is;
    }

    template<typename... Tp>
    friend BinIStreamWrap & operator >>(BinIStreamWrap &is, std::tuple<Tp ...> &tpl) {
        details::reader<sizeof...(Tp)>::read_tuple(is, tpl);
        return is;
    }

private:
    StreamTy &m_istr;
    bool m_useExceptions;
};

template <typename T, typename Source>
T read_val(Source &stream) {
    T outVal;
    stream >> outVal;
    return outVal;
}

template <class StreamTy>
auto make_bin_istream(StreamTy &stream) {
    return BinIStreamWrap<StreamTy>(stream);
}

namespace details {
    template <size_t I>
    struct writer {
        template<class StreamTy, typename... Tp>
        auto write_tuple(BinOStreamWrap<StreamTy> &os, const std::tuple< Tp ...> &tpl) {
            os << std::get<I>(tpl);
            writer<sizeof...(Tp) - (I + 1)>::write_tuple(os, tpl);
            return os;
        }
    };

    template <>
    struct writer<0> {
        template<class StreamTy, typename... Tp>
        auto write_tuple(BinOStreamWrap<StreamTy> &os, const std::tuple< Tp ...> &) {
            return os;
        }
    };
}

template <class StreamTy>
class BinOStreamWrap
{
public:
    explicit BinOStreamWrap(StreamTy &ostr)
        : m_ostr(ostr) {}

    int64_t get_opos() const {
        return m_ostr.tellp();
    }

    void set_opos(int64_t pos) {
        m_ostr.seekp(pos);
    }

    void goto_oend() {
        m_ostr.seekp(0, m_ostr.end);
    }

    void goto_obegin() {
        m_ostr.seekp(0, m_ostr.beg);
    }

    template <typename T>
    friend BinOStreamWrap &operator <<(BinOStreamWrap &os, const T &t) {
        static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
        os.m_ostr.write(reinterpret_cast< const char *>(&t), sizeof(t));
        return os;
    }

    template <typename T, typename Alloc>
    friend BinOStreamWrap &operator <<(      BinOStreamWrap         &os,
                                       const std::vector< T, Alloc > &vec) {
        const auto size = static_cast< uint64_t >(vec.size());
        os << size;
        os.m_ostr.write(reinterpret_cast< const char *>(vec.data()),
                        sizeof(T) * static_cast< size_t >(size)
                        );
        return os;
    }

    template <typename T, typename Alloc>
    friend BinOStreamWrap &operator <<(
            BinOStreamWrap &os,
            const std::list< T, Alloc > &list) {
        const auto size = static_cast< uint64_t >(list.size());
        os << size;
        for (auto &el: list) {
            os << el;
        }

        return os;
    }

    template <typename CharT, typename Traits, typename Alloc>
    friend BinOStreamWrap &operator <<(
            BinOStreamWrap &os,
            const std::basic_string< CharT, Traits, Alloc > &s) {
        const auto size = static_cast< uint64_t >(s.size());
        os << size;
        os.m_ostr.write(s.data(), sizeof(CharT) * static_cast< size_t >(size));
        return os;
    }

#ifdef QT_VERSION
    friend BinOStreamWrap &operator <<(BinOStreamWrap &os, const QString &str) {
        int32_t size = str.size();
        os << size;
        os.m_ostr.write(
            reinterpret_cast< const char *>(str.data()),
            static_cast< size_t >(size) * sizeof(QChar)
        );
        return os;
    }
#endif // QT_VERSION

    template <typename T>
    friend BinOStreamWrap &operator <<(      BinOStreamWrap          &os,
                                       const std::pair< T *, uint64_t > &cArr) {
        os << cArr.second;
        os.m_ostr.write(
            reinterpret_cast< const char *>(cArr.first),
            sizeof(T) * cArr.second
        );
        return os;
    }

    template <std::size_t I = 0, typename... Tp>
    friend auto operator <<(BinOStreamWrap &os, const std::tuple< Tp ...> &tpl)
        -> typename std::enable_if< I < sizeof...(Tp), BinOStreamWrap & >::type {
        os << std::get<I>(tpl);
        details::writer<sizeof...(Tp)>::write_tuple(os, tpl);
        return os;
    }

    template <std::size_t I = 0, typename... Tp>
    friend auto operator <<(BinOStreamWrap &os, const std::tuple< Tp ...> &)
        -> typename std::enable_if< I == sizeof...(Tp), BinOStreamWrap & >::type {
        return os;
    }

private:
    StreamTy &m_ostr;
};

template <class StreamTy>
auto make_bin_ostream(StreamTy &stream) {
    return BinIStreamWrap<StreamTy>(stream);
}

template <class StreamTy>
class BinIOStreamWrap
        : public BinIStreamWrap<StreamTy>
        , public BinOStreamWrap<StreamTy>
{
public:
    BinIOStreamWrap(StreamTy &iostr, UseExceptions useExceptions = UseExceptions::yes)
        : BinIStreamWrap<StreamTy>(iostr, useExceptions)
        , BinOStreamWrap<StreamTy>(iostr) {}

    void set_pos(int64_t pos) {
        this->set_ipos(pos); // just 'cause
    }

    int64_t get_pos() const {
        return this->get_opos(); // to be fair
    }
};

template <class StreamTy>
auto make_bin_iostream(StreamTy &stream) {
    return BinIOStreamWrap<StreamTy>(stream);
}

} // namespace fcl
