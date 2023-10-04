#ifndef SEZZ_STL_PAIR_HPP_
#define SEZZ_STL_PAIR_HPP_

#include <utility>

#include <sezz/type_traits.hpp>

namespace sezz {
    
template <class Archive, class T>
    requires detail::is_same_template_v<std::decay_t<T>, std::pair<detail::place_t, detail::place_t>>
void Serialize(Archive& ar, T&& val) {
    ar.Save(val.first);
    ar.Save(val.second);
}

template <class T, class Archive, class DecayT = std::decay_t<T>>
    requires detail::is_same_template_v<DecayT, std::pair<detail::place_t, detail::place_t>>
T Deserialize(Archive& ar) {
    //DecayT res{};
    //auto first = ar.Load<typename DecayT::first_type>();
    //auto second = ar.Load<typename DecayT::second_type>();
    //::new(&res) DecayT(first, second);
    //return res;
    return std::make_pair(ar.Load<typename DecayT::first_type>(), ar.Load<typename DecayT::second_type>());
}

} // namespace sezz


#endif // SEZZ_STL_PAIR_HPP_