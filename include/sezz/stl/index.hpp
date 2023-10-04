#ifndef SEZZ_STL_INDEX_HPP_
#define SEZZ_STL_INDEX_HPP_

#include <sezz/type_traits.hpp>

namespace sezz {
template <class Archive, class T>
void SerializeIndex(Archive& ar, T&& val) {
    uint32_t size = val.size();
    ar.GetIoStream().write((const char*)&size, sizeof(size));
    for (auto& v : val) {
        ar.Save(v);
    }
}

template <class T, class Archive>
T DeserializeIndex(Archive& ar) {
    T res{};
    uint32_t size = 0;
    ar.GetIoStream().read((char*)&size, sizeof(uint32_t));
    for (int64_t i = 0; i < size; i++) {
        res.insert(ar.Load<typename T::value_type>());
    }
    return res;
}



} // namespace sezz


#endif // SEZZ_STL_INDEX_HPP_