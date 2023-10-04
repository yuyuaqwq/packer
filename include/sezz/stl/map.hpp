#ifndef SEZZ_STL_MAP_HPP_
#define SEZZ_STL_MAP_HPP_

#include <map>

#include <sezz/type_traits.hpp>
#include <sezz/stl/pair.hpp>
#include <sezz/stl/index.hpp>

namespace sezz {

template <class Archive, class T>
    requires type_traits::is_same_template_v<std::decay_t<T>, std::map<type_traits::place_t, type_traits::place_t>>
void Serialize(Archive& os, T&& val) {
    SerializeIndex(os, val);
}

template <class T, class Archive>
    requires type_traits::is_same_template_v<std::decay_t<T>, std::map<type_traits::place_t, type_traits::place_t>>
T Deserialize(Archive& is) {
    return DeserializeIndex<T>(is);
}

} // namespace sezz


#endif // SEZZ_STL_MAP_HPP_