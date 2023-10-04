#ifndef SEZZ_STL_STRING_HPP_
#define SEZZ_STL_STRING_HPP_

#include <string>

#include <sezz/type_traits.hpp>

namespace sezz {

template <class Archive, class T>
    requires type_traits::is_same_template_v<std::decay_t<T>, std::basic_string<type_traits::place_t>>
void Serialize(Archive& ar, T&& val) {
    uint32_t size = val.size();
    ar.GetIoStream().write((const char*)&size, sizeof(size));
    ar.GetIoStream().write(val.data(), size);
}

template <class T, class Archive>
    requires type_traits::is_same_template_v<std::decay_t<T>, std::basic_string<type_traits::place_t>>
T Deserialize(Archive& ar) {
    using DecayT = std::decay_t<T>;
    uint32_t size = 0;
    ar.GetIoStream().read((char*)&size, sizeof(uint32_t));
    DecayT res{};
    res.resize(size);
    ar.GetIoStream().read(res.data(), size);
    return res;
}

} // namespace sezz


#endif // SEZZ_STL_STRING_HPP_