#ifndef SEZZ_STL_PAIR_HPP_
#define SEZZ_STL_PAIR_HPP_

#include <utility>

#include <sezz/type_traits.hpp>

namespace sezz {
    
template <class Archive, class T>
    requires type_traits::is_same_template_v<std::decay_t<T>, std::pair<type_traits::place_t, type_traits::place_t>>
void Serialize(Archive& os, T&& val) {
    Serialize(os, val.first);
    Serialize(os, val.second);
}

template <class T, class Archive>
    requires type_traits::is_same_template_v<std::decay_t<T>, std::pair<type_traits::place_t, type_traits::place_t>>
T Deserialize(Archive& is) {
    //auto first = Deserialize<typename T::first_type>(is);
    //auto second = Deserialize<typename T::second_type>(is);
    //::new(&res) T(first, second);
    //return res;
    return std::make_pair(Deserialize<typename T::first_type>(is), Deserialize<typename T::second_type>(is));
}

} // namespace sezz


#endif // SEZZ_STL_PAIR_HPP_