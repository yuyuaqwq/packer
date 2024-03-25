#pragma once
#include <concepts>
#include <packer/detail/concept.hpp>
#include <packer/detail/structure_binding.hpp>
#include <packer/detail/output_buffer.hpp>

namespace packer {
template <typename T>
struct Packer {};

namespace detail {
template <typename T>
struct BuiltInPacker {};

template <typename Iter>
class BasicContext {
public:
	using Iterator = Iter;
	template <typename T>
	using BuiltInPackerType = BuiltInPacker<T>;
	template <typename T>
	using PackerType = Packer<T>;

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