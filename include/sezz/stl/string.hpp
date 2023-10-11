#ifndef SEZZ_STL_STRING_HPP_
#define SEZZ_STL_STRING_HPP_

#include <string>

#include <sezz/type_traits.hpp>

namespace sezz {

template <class Archive, class T>
    requires detail::is_same_template_v<std::decay_t<T>, std::basic_string<detail::place_t>>
void Serialize(Archive& ar, T&& val) {
    size_t size = val.size();
    ar.Save(size);
    ar.GetOutputStream().write(val.data(), size);
}

template <class T, class Archive, class DecayT = std::decay_t<T>>
    requires detail::is_same_template_v<DecayT, std::basic_string<detail::place_t>>
T Deserialize(Archive& ar) {
    auto size = ar.Load<size_t>();
    DecayT res{};
    res.resize(size);
    ar.GetInputStream().read(res.data(), size);
    if (ar.GetInputStream().fail()) {
        throw std::runtime_error("input stream read fail.");
    }
    return res;
}

} // namespace sezz


#endif // SEZZ_STL_STRING_HPP_