#ifndef SEZZ_STL_MAP_HPP_
#define SEZZ_STL_MAP_HPP_

#include <map>

#include <sezz/type_traits.hpp>
#include <sezz/stl/pair.hpp>
#include <sezz/stl/index.hpp>

namespace sezz {

template <class Archive, class T>
    requires detail::is_same_template_v<std::decay_t<T>, std::map<detail::place_t, detail::place_t>>
void Serialize(Archive& ar, T&& val) {
    SerializeIndex(ar, val);
}

template <class T, class Archive, class DecayT = std::decay_t<T>>
    requires detail::is_same_template_v<DecayT, std::map<detail::place_t, detail::place_t>>
T Deserialize(Archive& ar) {
    return DeserializeIndex<DecayT>(ar);
}

} // namespace sezz


#endif // SEZZ_STL_MAP_HPP_