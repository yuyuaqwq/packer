#ifndef SEZZ_STL_VECTOR_HPP_
#define SEZZ_STL_VECTOR_HPP_

#include <sezz/type_traits.hpp>
#include <vector>

namespace sezz {

template <class Archive, class T>
    requires type_traits::is_same_template_v<std::decay_t<T>, std::vector<type_traits::place_t>>
void Serialize(Archive& ar, T& val) {
    uint32_t size = val.size();
    ar.GetIoStream().write((const char*)&size, sizeof(size));
    for (auto& v : val) {
        ar.Save(v);
    }
}

template <class T, class Archive>
    requires type_traits::is_same_template_v<std::decay_t<T>, std::vector<type_traits::place_t>>
T Deserialize(Archive& ar) {
    using DecayT = std::decay_t<T>;
    uint32_t size = 0;
    ar.GetIoStream().read((char*)&size, sizeof(uint32_t));
    DecayT res{ size };
    for (int64_t i = 0; i < size; i++) {
        res[i] = ar.Load<typename T::value_type>();
    }
    return res;
}

} // namespace sezz


#endif // SEZZ_STL_VECTOR_HPP_