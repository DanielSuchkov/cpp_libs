#pragma once

#include <ostream>
#include <vector>
#include <list>
#include <string>
#include <tuple>
#include <cstdint>
#include <type_traits>
#ifdef QT_VERSION
#   include <QString>
#endif // QT_VERSION

#ifdef _MSC_VER
#   define noexcept
#endif

class BinOStreamWrap
{
public:
    explicit BinOStreamWrap(std::ostream &ostr) :
        m_ostr(ostr)
    { }

    template <typename T>
    friend BinOStreamWrap &operator <<(BinOStreamWrap &os, const T &t)
    {
        // static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
        os.m_ostr.write(reinterpret_cast< const char *>(&t), sizeof(t));
        return os;
    }

    template <typename T, typename Alloc>
    friend BinOStreamWrap &operator <<(      BinOStreamWrap         &os,
                                       const std::vector< T, Alloc > &vec)
    {
        const auto size = static_cast< uint64_t >(vec.size());
        os << size;
        os.m_ostr.write(reinterpret_cast< const char *>(vec.data()),
                        sizeof(T) * static_cast< size_t >(size)
                        );
        return os;
    }

    template <typename T, typename Alloc>
    friend BinOStreamWrap &operator <<(      BinOStreamWrap       &os,
                                      const std::list< T, Alloc > &list)
    {
        const auto size = static_cast< uint64_t >(list.size());
        os << size;
        for (auto &el: list)
        {
            os << el;
        }

        return os;
    }

    template <typename CharT, typename Traits, typename Alloc>
    friend BinOStreamWrap &operator <<(
                  BinOStreamWrap                            &os,
            const std::basic_string< CharT, Traits, Alloc > &s
            )
    {
        const auto size = static_cast< uint64_t >(s.size());
        os << size;
        os.m_ostr.write(s.data(), sizeof(CharT) * static_cast< size_t >(size));
        return os;
    }

#ifdef QT_VERSION
    friend BinOStreamWrap &operator <<(BinOStreamWrap &os, const QString &str)
    {
        int32_t size = str.size();
        os << size;
        os.m_ostr.write(reinterpret_cast< const char *>(str.data()),
                        static_cast< size_t >(size) * sizeof(QChar)
                        );
        return os;
    }

    template <typename T, typename SizeT>
    friend BinOStreamWrap &operator <<(      BinOStreamWrap          &os,
                                       const std::pair< T *, SizeT > &cArr)
    {
        os.m_ostr.write(reinterpret_cast< const char *>(cArr.first),
                        sizeof(T) * cArr.second
                        );
        return os;
    }
#endif // QT_VERSION

    template <std::size_t I = 0, typename... Tp>
    friend auto operator <<(BinOStreamWrap &os, const std::tuple< Tp ...> &tpl)
        -> typename std::enable_if< I < sizeof...(Tp), BinOStreamWrap & >::type
    {
        os << std::get<I>(tpl);
        operator<< <I + 1>(os, tpl);
        return os;
    }

    template <std::size_t I = 0, typename... Tp>
    friend auto operator <<(BinOStreamWrap &os, const std::tuple< Tp ...> &)
        -> typename std::enable_if< I == sizeof...(Tp), BinOStreamWrap & >::type
    {
        return os;
    }

private:
    std::ostream &m_ostr;
};
