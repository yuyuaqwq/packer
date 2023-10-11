#ifndef SEZZ_STL_INDEX_HPP_
#define SEZZ_STL_INDEX_HPP_

#include <sezz/type_traits.hpp>

namespace sezz {
template <class Archive, class T>
void SerializeIndex(Archive& ar, T&& val) {
    size_t size = val.size();
    ar.Save(size);
    for (auto& v : val) {
        ar.Save(v);
    }
}

template <class T, class Archive>
T DeserializeIndex(Archive& ar) {
    T res{};
    auto size = ar.Load<size_t>();
    for (size_t i = 0; i < size; i++) {
        res.emplace(ar.Load<typename T::value_type>());
    }
    return res;
}



} // namespace sezz


#endif // SEZZ_STL_INDEX_HPP_