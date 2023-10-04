#ifndef SEZZ_STL_VECTOR_HPP_
#define SEZZ_STL_VECTOR_HPP_

#include <sezz/type_traits.hpp>
#include <vector>

namespace sezz {

template <class Archive, class T>
    requires type_traits::is_same_template_v<std::decay_t<T>, std::vector<type_traits::place_t>>
void Serialize(Archive& os, T& val) {
    uint32_t size = val.size();
    os.write((const char*)&size, sizeof(size));
    for (auto& v : val) {
        Serialize(os, v);
    }
}

template <class T, class Archive>
    requires type_traits::is_same_template_v<std::decay_t<T>, std::vector<type_traits::place_t>>
T Deserialize(Archive& is) {
    using DecayT = std::decay_t<T>;
    uint32_t size = 0;
    is.read((char*)&size, sizeof(uint32_t));
    DecayT res{ size };
    for (int64_t i = 0; i < size; i++) {
        res[i] = Deserialize<typename T::value_type>(is);
    }
    return res;
}

} // namespace sezz


#endif // SEZZ_STL_VECTOR_HPP_