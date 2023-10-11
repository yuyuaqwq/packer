#ifndef SEZZ_STL_VECTOR_HPP_
#define SEZZ_STL_VECTOR_HPP_

#include <sezz/type_traits.hpp>
#include <vector>

namespace sezz {

template <class Archive, class T>
    requires detail::is_same_template_v<std::decay_t<T>, std::vector<detail::place_t>>
void Serialize(Archive& ar, T& val) {
    size_t size = val.size();
    ar.Save(size);
    for (auto& v : val) {
        ar.Save(v);
    }
}

template <class T, class Archive, class DecayT = std::decay_t<T>>
    requires detail::is_same_template_v<DecayT, std::vector<detail::place_t>>
T Deserialize(Archive& ar) {
    auto size = ar.Load<size_t>();
    DecayT res;
    res.reserve(size);
    for (size_t i = 0; i < size; i++) {
        res.emplace_back(ar.Load<typename DecayT::value_type>());
    }
    return res;
}

} // namespace sezz


#endif // SEZZ_STL_VECTOR_HPP_