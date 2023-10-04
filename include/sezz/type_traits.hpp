#ifndef SEZZ_TYPE_TRAITS_HPP_
#define SEZZ_TYPE_TRAITS_HPP_

#include <typeinfo>

namespace sezz {
namespace type_traits {

template<template<class> class, class, class = void>
struct is_detected : std::false_type {};
template<template<class T> class OpT, class T>
struct is_detected<OpT, T, std::void_t<OpT<T>>> : std::true_type {};

template<template<class, class> class, class, class, class = void>
struct is_detected2 : std::false_type {};
template<template<class T1, class T2> class OpT, class T1, class T2>
struct is_detected2<OpT, T1, T2, std::void_t<OpT<T1, T2>>> : std::true_type {};

template<template<class T> class OpT, class T>
constexpr bool is_detected_v = is_detected<OpT, T>::value;
template<template<class T1, class T2> class OpT, class T1, class T2>
constexpr bool is_detected2_v = is_detected2<OpT, T1, T2>::value;

template <template <class...> class T>
struct TemplateType {};

template <class T>
struct ExtractTemplate {
    static constexpr bool IsTemplate = false;
};

template <template <class...> class T, class... Args>
struct ExtractTemplate<T<Args...>> {
    static constexpr bool IsTemplate = true;
    using Type = TemplateType<T>;
};

template <class T, class U>
constexpr bool is_same_template() {
    if constexpr (ExtractTemplate<T>::IsTemplate != ExtractTemplate<U>::IsTemplate) {
        return false;
    }
    else {
        return std::is_same_v<typename ExtractTemplate<T>::Type, typename ExtractTemplate<U>::Type>;
    }
}

template <class T, class U>
constexpr bool is_same_template_v = is_same_template<T, U>();

template <typename T>
using is_iterable_t = std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>;
template <class T>
constexpr bool is_iterable_v = type_traits::is_detected_v<is_iterable_t, T>;


using place_t = char;

} // namespace internal
} // namespace sezz

#endif // SEZZ_TYPE_TRAITS_HPP_