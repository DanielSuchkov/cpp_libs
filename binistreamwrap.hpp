#pragma once

#include <istream>
#include <vector>
#include <list>
#include <string>
#include <tuple>
#include <type_traits>
#include <exception>
#ifdef QT_VERSION
#   include <QString>
#endif // QT_VERSION

#ifdef _MSC_VER
#   define noexcept
#endif

class ReadingAtEOF : public std::exception
{
public:
    virtual const char *what() const noexcept override final
    {
        return "attempt to read at eof";
    }
};

class BinIStreamWrap
{
public:
    enum class UseExceptions
    {
        yes, no
    };

public:
    /*!
     * \brief BinIStreamWrap
     * \param istr Input stream opend with std::ios::binary flag
     * \param useExceptions UseExceptions::yes if you want that this class notify you about reading at eof using exceptions and UseExceptions::no if no
     */
    explicit BinIStreamWrap(std::istream &istr,
                            UseExceptions useExceptions = UseExceptions::yes) :
        m_istr(istr),
        m_useExceptions(useExceptions == UseExceptions::yes)
    { }

    template <typename T>
    friend BinIStreamWrap &operator >>(BinIStreamWrap &is, T &t)
    {
        // static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
        is.m_istr.read(reinterpret_cast< char *>(&t), sizeof(t));
        if (is.m_istr.eof() && is.m_useExceptions)
        {
            throw ReadingAtEOF();
        }
        return is;
    }

    template <typename T, typename Alloc>
    friend BinIStreamWrap &operator >>(BinIStreamWrap &is, std::vector< T, Alloc > &vec)
    {
        uint64_t size;
        is >> size;
        vec.resize(static_cast< size_t >(size));
        is.m_istr.read(reinterpret_cast< char *>(vec.data()), static_cast< size_t >(size) * sizeof(T));
        if (is.m_istr.eof() && is.m_useExceptions)
        {
            throw ReadingAtEOF();
        }
        return is;
    }

    template <typename T, typename Alloc>
    friend BinIStreamWrap &operator >>(BinIStreamWrap &is, std::list< T, Alloc > &list)
    {
        uint64_t size;
        is >> size;
        list.resize(static_cast< size_t >(size));

        for (auto &el: list)
        {
            is >> el;
        }

        return is;
    }

    template <typename CharT, typename Traits, typename Alloc>
    friend BinIStreamWrap &operator >>(
            BinIStreamWrap                            &is,
            std::basic_string< CharT, Traits, Alloc > &s
            )
    {
        uint64_t size;
        is >> size;
        s.resize(static_cast< size_t >(size));
        is.m_istr.read(reinterpret_cast< char *>(&s.front()),
                       sizeof(CharT) * static_cast< size_t >(size)
                       );

        if (is.m_istr.eof() && is.m_useExceptions)
        {
            throw ReadingAtEOF();
        }
        return is;
    }

#ifdef QT_VERSION
    friend BinIStreamWrap &operator >>(BinIStreamWrap &is, QString &str)
    {
        int32_t size;
        is >> size;
        str.resize(static_cast< int >(size));
        is.m_istr.read(reinterpret_cast< char *>(str.data()),
                       static_cast< size_t >(size) * sizeof(QChar)
                       );
        if (is.m_istr.eof() && is.m_useExceptions)
        {
            throw ReadingAtEOF();
        }
        return is;
    }
#endif // QT_VERSION

    template <typename T>
    friend BinIStreamWrap &operator >>(BinIStreamWrap &is, std::pair< T *, uint64_t > &cArr)
    {
        is >> cArr.second;
        cArr.first = new T[cArr.second];
        is.m_istr.read(reinterpret_cast< char *>(cArr.first),
                       sizeof(T) * cArr.second
                       );
        if (is.m_istr.eof() && is.m_useExceptions)
        {
            throw ReadingAtEOF();
        }
        return is;
    }

    template<std::size_t I = 0, typename... Tp>
    friend auto operator >>(BinIStreamWrap &is, std::tuple<Tp ...> &tpl)
        -> typename std::enable_if< (I < sizeof...(Tp)), BinIStreamWrap &>::type
    {
        is >> std::get<I>(tpl);
        operator>> < I + 1 >(is, tpl);
        return is;
    }

    template<std::size_t I = 0, typename... Tp>
    friend auto operator >>(BinIStreamWrap &is, std::tuple<Tp ...> &)
        -> typename std::enable_if< (I == sizeof...(Tp)), BinIStreamWrap &>::type
    {
        return is;
    }

private:
    std::istream &m_istr;
    bool m_useExceptions;
};

template <typename T, typename Source>
T readVal(Source &stream)
{
    T outVal;
    stream >> outVal;
    return outVal;
}
