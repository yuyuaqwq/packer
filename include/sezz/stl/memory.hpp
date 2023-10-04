#ifndef SEZZ_STL_MEMORY_HPP_
#define SEZZ_STL_MEMORY_HPP_

#include <sezz/type_traits.hpp>
#include <memory>

namespace sezz {

template <class Archive, class T>
    requires detail::is_same_template_v<std::decay_t<T>, std::unique_ptr<detail::place_t>>
void Serialize(Archive& os, T&& val) {
    ar.Sava(*val, args...);
}

template <class T, class Archive, class DecayT = std::decay_t<T>>
    requires detail::is_same_template_v<DecayT, std::unique_ptr<detail::place_t>>
T Deserialize(Archive& ar) {
    using DecayRawT = std::decay_t<DecayT::element_type>;
    return std::make_unique<DecayRawT>(ar.Load<DecayRawT>());
}

} // namespace sezz


#endif // SEZZ_STL_MEMORY_HPP_