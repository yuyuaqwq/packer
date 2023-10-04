#ifndef SEZZ_TYPE_TRAITS_HPP_
#define SEZZ_TYPE_TRAITS_HPP_

#include <typeinfo>

namespace sezz {
namespace type_traits {

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

template<class T>
concept iterate_accept = requires(T t) {
    t.begin();
    t.end();
};


using place_t = char;

} // namespace internal
} // namespace sezz

#endif // SEZZ_TYPE_TRAITS_HPP_