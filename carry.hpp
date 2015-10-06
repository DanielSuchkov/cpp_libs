#pragma once
#include <iostream>
#include <functional>
#include <type_traits>


template<int...> struct int_sequence {};

template<int N, int... Is> struct make_int_sequence
    : make_int_sequence<N-1, N-1, Is...> {};

template<int... Is> struct make_int_sequence<0, Is...>
    : int_sequence<Is...> {};

template<int>
struct placeholder_template {};

namespace std {
    template<int N>
    struct is_placeholder< placeholder_template<N> >
        : integral_constant<int, N+1>
    {};
}

template<typename T>
struct memfun_type
{
    using type = void;
};

template<typename Ret, typename Class, typename... Args>
struct memfun_type<Ret(Class::*)(Args...) const>
{
    using type = std::function<Ret(Args...)>;
};

template<typename Ret, typename Class, typename... Args>
struct memfun_type<Ret(Class::*)(Args...)>
{
    using type = std::function<Ret(Args...)>;
};

template<typename F>
auto FFL(F const &func)
    -> typename memfun_type<decltype(&F::operator())>::type
{
    return func;
}

// Do not use this!
template<typename Fn, typename ...Some, int ...Is>
auto carry_(const Fn &fn, int_sequence<Is...>, Some &&...arg)
    -> decltype(std::bind(fn, std::forward<Some>(arg)..., placeholder_template<Is>{}...)) {
    return std::bind(fn, std::forward<Some>(arg)..., placeholder_template<Is>{}...);
}

// For simple functions
template<typename Ret, typename ...Some, typename ...Args>
auto carry(Ret (*fn)(Args...), Some &&...arg)
    -> decltype(carry_(fn, make_int_sequence<sizeof...(Args) - sizeof...(Some)>{}, std::forward<Some>(arg)...)) {
    return carry_(fn, make_int_sequence<sizeof...(Args) - sizeof...(Some)>{}, std::forward<Some>(arg)...);
}

// For std::function
template<typename Ret, typename ...Some, typename ...Args>
auto carry(const std::function<Ret (Args...)> &fn, Some &&...arg)
    -> decltype(carry_(fn, make_int_sequence<sizeof...(Args) - sizeof...(Some)>{}, std::forward<Some>(arg)...)) {
    return carry_(fn, make_int_sequence<sizeof...(Args) - sizeof...(Some)>{}, std::forward<Some>(arg)...);
}

// For member-functions
template<typename T, typename Ret, typename ...Some, typename ...Args>
auto carry(Ret (T::*fn)(Args...), Some &&...arg)
    -> decltype(carry_(fn, make_int_sequence<sizeof...(Args) - sizeof...(Some) + 1>{}, std::forward<Some>(arg)...)) {
    return carry_(fn, make_int_sequence<sizeof...(Args) - sizeof...(Some) + 1>{}, std::forward<Some>(arg)...);
}

// For const member-functions
template<typename T, typename Ret, typename ...Some, typename ...Args>
auto carry(Ret (T::*fn)(Args...) const, Some &&...arg)
    -> decltype(carry_(fn, make_int_sequence<sizeof...(Args) - sizeof...(Some) + 1>{}, std::forward<Some>(arg)...)) {
    return carry_(fn, make_int_sequence<sizeof...(Args) - sizeof...(Some) + 1>{}, std::forward<Some>(arg)...);
}

// For lambdas/functional objects
template<typename Fn, typename ...Some>
auto carry(Fn f, Some &&...arg)
    -> decltype(carry(FFL(f), arg...)) {
    return carry(FFL(f), arg...);
}
