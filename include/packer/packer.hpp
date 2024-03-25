#pragma once
#include <concepts>
#include <packer/detail/structure_binding.hpp>
#include <packer/detail/output_buffer.hpp>

namespace packer {
template <typename T>
inline constexpr bool kAlwaysFalse = false;
template <typename T, typename... Types>
concept AnyOf = (std::is_same_v<T, Types> || ...);

template <typename T>
struct Packer {};

namespace detail {
template <typename T>
struct BuiltInPacker {};

template <typename T, typename ContextType>
concept HasImplBuiltInSerialize = requires(T & t, ContextType & ctx) {
	std::declval<BuiltInPacker<T>>().Serialize(t, ctx);
};
template <typename T, typename ContextType>
concept HasImplSerialize = requires(T & t, ContextType & ctx) {
	std::declval<Packer<T>>().Serialize(t, ctx);
};

template <typename T, typename ContextType>
concept HasImplBuiltInDeserialize = requires(T* t, ContextType & ctx) {
	std::declval<BuiltInPacker<T>>().Deserialize(t, ctx);
};
template <typename T, typename ContextType>
concept HasImplDeserialize = requires(T* t, ContextType & ctx) {
	std::declval<Packer<T>>().Deserialize(t, ctx);
};

template <typename Iter>
class BasicContext {
public:
	using Iterator = Iter;

	BasicContext(Iter iter) : iter_{ std::move(iter) } {}

	Iterator iter() {
		return std::move(iter_);
	}

	void advance_to(Iterator iter) {
		iter_ = std::move(iter);
	}

private:
	Iter iter_;
};

template <typename T, typename ContextType>
void SerializeImpl(const T& val, ContextType& ctx) {
	if constexpr (HasImplSerialize<T, ContextType>)
		Packer<T>{}.Serialize(val, ctx);
	else if constexpr (HasImplBuiltInSerialize<T, ContextType>)
		BuiltInPacker<T>{}.Serialize(val, ctx);
	else if constexpr (StructurellyBindable<T>) {
		StructureBinding(val, [&]<typename... Members>(Members&&... members) {
			(SerializeImpl(members, ctx), ...);
		});
	}
	else
		static_assert(kAlwaysFalse<T>, "You haven't specialized the Packer::Serialize for this type T yet!");
}

template <typename T, typename ContextType>
void DeserializeImpl(T* res, ContextType& ctx) {
	if constexpr (HasImplDeserialize<T, ContextType>)
		Packer<T>{}.Deserialize(res, ctx);
	else if constexpr (HasImplBuiltInDeserialize<T, ContextType>)
		BuiltInPacker<T>{}.Deserialize(res, ctx);
	else if constexpr (StructurellyBindable<T>) {
		StructureBinding(*res, [&]<typename... Members>(Members&&... members) {
			(DeserializeImpl(std::addressof(members), ctx), ...);
		});
	}
	else
		static_assert(kAlwaysFalse<T>, "You haven't specialized the Packer::Deserialize for this type T yet!");
}
}	// namespace detail


template <std::output_iterator<const char&> OutputIt, typename T>
OutputIt SerializeTo(OutputIt it, const T& val) {
	detail::OutputBuffer<OutputIt, char> buffer{std::move(it)};
	detail::BasicContext ctx{std::back_insert_iterator{ buffer }};
	detail::SerializeImpl(val, ctx);
	return buffer.out();
}

template <std::input_iterator InputIt, typename T>
void DeserializeTo(InputIt it, T* res) {
	detail::BasicContext ctx{it};
	detail::DeserializeImpl(res, ctx);
}


namespace detail {
template <AnyOf<char, unsigned char> T>
struct BuiltInPacker<T> {
	template<typename OutputContext>
	void Serialize(const T& val, OutputContext& ctx) {
		auto it = ctx.iter();
		*it++ = val;
		ctx.advance_to(it);
	}
	template<typename InputContext>
	void Deserialize(T* val, InputContext& ctx) {
		auto it = ctx.iter();
		*val = *it++;
		ctx.advance_to(it);
	}
};
}
}	// namespace detail