#ifndef SEZZ_STL_OPTIONAL_HPP_
#define SEZZ_STL_OPTIONAL_HPP_

#include <sezz/type_traits.hpp>
#include <optional>

namespace sezz {

template <class Archive, class T>
    requires detail::is_same_template_v<std::decay_t<T>, std::optional<detail::place_t>>
void Serialize(Archive& ar, T& val) {
    bool has_value = val.has_value();
    ar.Save(has_value);
    if (has_value) {
        ar.Save(val.value());
    }
}

template <class T, class Archive, class DecayT = std::decay_t<T>>
    requires detail::is_same_template_v<DecayT, std::optional<detail::place_t>>
T Deserialize(Archive& ar) {
    bool has_value = ar.Load<bool>();
    if (has_value) {
        return ar.Load<typename DecayT::value_type>();
    } else {
        return std::nullopt;
    }
}

} // namespace sezz


#endif // SEZZ_STL_OPTIONAL_HPP_