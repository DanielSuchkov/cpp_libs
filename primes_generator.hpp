#pragma once
#include <vector>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <cmath>
#include <tuple>

template <typename num_type = uint32_t>
class primes_generator {
    static_assert(std::is_unsigned<num_type>::value, "num_type have to be unsigned");

public:
    using range_t = std::pair<num_type, num_type>;
    using inner_storage_t = std::vector<num_type>;

public:
    primes_generator(range_t range)
        : m_range(range) {
        m_innerStg = this->gen_primes(range);
    }

    auto begin() -> typename inner_storage_t::iterator { return m_innerStg.begin(); }
    auto end() -> typename inner_storage_t::iterator { return m_innerStg.end(); }
    auto begin() const -> typename inner_storage_t::const_iterator { return m_innerStg.cbegin(); }
    auto end() const -> typename inner_storage_t::const_iterator { return m_innerStg.cend(); }

private:
    inner_storage_t m_innerStg;
    range_t m_range;

private:
    static std::vector<num_type> gen_primes(range_t range) {
        std::vector<bool> is_prime(range.second, true);
        const num_type real_upper_bound =
            static_cast<num_type>(std::sqrt(static_cast<double>(range.second)) + 1);

        for (num_type i = 2; i <= real_upper_bound; ++i) {
            if (is_prime[i]) {
                for (auto j = i * 2; j <= range.second; j += i) {
                    is_prime[j] = false;
                }
            }
        }

        std::vector<num_type> result;

        for (num_type i = range.first; i < range.second; ++i) {
            if (is_prime[i]) {
                result.push_back(i);
            }
        }

        return result;
    }
};
