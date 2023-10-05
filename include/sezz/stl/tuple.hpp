#ifndef SEZZ_STL_TUPLE_HPP_
#define SEZZ_STL_TUPLE_HPP_

#include <tuple>

#include <sezz/type_traits.hpp>

namespace sezz {

template <class Archive, class T>
    requires detail::is_same_template_v<std::decay_t<T>, std::tuple<detail::place_t>>
void Serialize(Archive& ar, T&& val) {
    std::apply([&](auto&&... args) { 
        (ar.Save(args), ...); 
    }, std::forward<T>(val));
}

template <class T, class Archive, class DecayT = std::decay_t<T>>
    requires detail::is_same_template_v<std::decay_t<T>, std::tuple<detail::place_t>>
T Deserialize(Archive& ar) {
    DecayT res{};
    std::apply([&](auto&... args) { 
        (ar.Load(args), ...); 
    }, res);
    return res;
}
 
} // namespace sezz


#endif // SEZZ_STL_TUPLE_HPP_