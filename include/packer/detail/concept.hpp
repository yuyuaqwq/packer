#pragma once

namespace packer {
template <typename T>
inline constexpr bool kAlwaysFalse = false;
template <typename T, typename... Types>
concept AnyOf = (std::is_same_v<T, Types> || ...);

namespace detail {
template <typename T, typename ContextType>
using BuiltInPackerType = typename ContextType::template BuiltInPackerType<T>;
template <typename T, typename ContextType>
using PackerType = typename ContextType::template PackerType<T>;

template <typename T, typename ContextType>
concept HasImplBuiltInSerialize = requires(T & t, ContextType & ctx) {
	std::declval<BuiltInPackerType<T, ContextType>>().Serialize(t, ctx);
};
template <typename T, typename ContextType>
concept HasImplSerialize = requires(T & t, ContextType & ctx) {
	std::declval<PackerType<T, ContextType>>().Serialize(t, ctx);
};

template <typename T, typename ContextType>
concept HasImplBuiltInDeserialize = requires(T * t, ContextType & ctx) {
	std::declval<BuiltInPackerType<T, ContextType>>().Deserialize(t, ctx);
};
template <typename T, typename ContextType>
concept HasImplDeserialize = requires(T * t, ContextType & ctx) {
	std::declval<PackerType<T, ContextType>>().Deserialize(t, ctx);
};
}
}