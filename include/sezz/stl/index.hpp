#ifndef SEZZ_STL_INDEX_HPP_
#define SEZZ_STL_INDEX_HPP_

#include <sezz/type_traits.hpp>

namespace sezz {
template <class Archive, class T>
void SerializeIndex(Archive& os, T&& val) {
    uint32_t size = val.size();
    os.write((const char*)&size, sizeof(size));
    for (auto& v : val) {
        Serialize(os, v);
    }
}

template <class T, class Archive>
T DeserializeIndex(Archive& is) {
    using DecayT = std::decay_t<T>;
    DecayT res{};
    uint32_t size = 0;
    is.read((char*)&size, sizeof(uint32_t));
    for (int64_t i = 0; i < size; i++) {
        res.insert(Deserialize<typename T::value_type>(is));
    }
    return res;
}

} // namespace sezz


#endif // SEZZ_STL_INDEX_HPP_