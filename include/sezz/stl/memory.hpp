#ifndef SEZZ_STL_MEMORY_HPP_
#define SEZZ_STL_MEMORY_HPP_

#include <sezz/type_traits.hpp>
#include <memory>

namespace sezz {

template <class Archive, class T>
    requires type_traits::is_same_template_v<std::decay_t<T>, std::unique_ptr<type_traits::place_t>>
void Serialize(Archive& os, T&& val) {
    Serialize(os, *val, args...);
}

template <class T, class Archive>
    requires type_traits::is_same_template_v<std::decay_t<T>, std::unique_ptr<type_traits::place_t>>
T Deserialize(Archive& is) {
    using DecayRawT = std::decay_t<DecayT::element_type>;
    return std::make_unique<DecayRawT>(Deserialize<DecayRawT>(is));
}

} // namespace sezz


#endif // SEZZ_STL_MEMORY_HPP_