#pragma once
#include <concepts>
#include <packer/detail/algorithm.hpp>
#include <packer/detail/context.hpp>
#include <packer/detail/structure_binding.hpp>
#include <packer/detail/output_buffer.hpp>

namespace packer {
template <typename T>
struct Packer {};

//template <typename T>
//struct Packer<T> {
//	void Serialize(const T& val, auto& ctx) {
//	}
//
//	void Deserialize(T* res, auto& ctx) {
//	}
//};

namespace detail {
template <typename T>
struct BuiltInPacker {};

template <typename T, typename ContextType>
void SerializeImpl(const T& val, ContextType& ctx) {
	if constexpr (HasImplSerialize<T, ContextType>)
		Packer<T>{}.Serialize(val, ctx);
	else if constexpr (HasImplBuiltInSerialize<T, ContextType>)
		BuiltInPacker<T>{}.Serialize(val, ctx);
	else {
		StructureBinding(val, [&]<typename... Members>(Members&&... members) {
			(SerializeImpl(members, ctx), ...);
		});
	}
}

template <typename T, typename ContextType>
void DeserializeImpl(T* res, ContextType& ctx) {
	using Type = std::remove_cvref_t<T>;

	auto noconst_res = const_cast<Type*>(res);

	if constexpr (HasImplDeserialize<Type, ContextType>)
		Packer<Type>{}.Deserialize(noconst_res, ctx);
	else if constexpr (HasImplBuiltInDeserialize<Type, ContextType>)
		BuiltInPacker<Type>{}.Deserialize(noconst_res, ctx);
	else {
		StructureBinding(*noconst_res, [&]<typename... Members>(Members&&... members) {
			(DeserializeImpl(std::addressof(members), ctx), ...);
		});
	}
}
}	// namespace detail


template <std::output_iterator<const char&> OutputIt, typename T>
OutputIt SerializeTo(OutputIt it, const T& val) {
	detail::OutputBuffer<OutputIt, char> buffer{std::move(it)};
	detail::BasicOutputContext ctx{std::back_insert_iterator{ buffer }};
	detail::SerializeImpl(val, ctx);
	return buffer.out();
}

template <std::input_iterator InputIt, typename T>
void DeserializeTo(InputIt it, T* res) {
	detail::BasicInputContext ctx{it};
	detail::DeserializeImpl(res, ctx);
}


namespace detail {
template <AnyOf<char, unsigned char> T>
struct BuiltInPacker<T> {
	void Serialize(const T& val, auto& ctx) {
		ContextWriteByte(val, ctx);
	}

	void Deserialize(T* res, auto& ctx) {
		*res = ContextReadByte(ctx);
	}
};

template <std::integral T>
requires (sizeof(T) > 2)
struct BuiltInPacker<T> {
	void Serialize(const T& val, auto& ctx) {
		auto it = ctx.iter();
		if constexpr (std::is_signed_v<T>)
			it = ZigzagEncoded(val, it);
		else
			it = VarintEncoded(val, it);
		ctx.advance_to(it);
	}

	void Deserialize(T* res, auto& ctx) {
		int64_t tmp;
		auto it = ctx.iter();
		if constexpr (std::is_signed_v<T>)
			it = ZigzagDecode(&tmp, it);
		else
			it = VarintDecode(&tmp, it);
		ctx.advance_to(it);
		*res = static_cast<T>(tmp);
	}
};

template <std::floating_point T>
struct BuiltInPacker<T> {
	static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little);

	void Serialize(const T& val, auto& ctx) {
		T tmp = val;
		if constexpr (std::endian::native == std::endian::big) {
			tmp = RevereseByte(tmp);
		}
		ContextWriteValue(tmp, ctx);
	}

	void Deserialize(T* res, auto& ctx) {
		T tmp;
		tmp = ContextReadValue<T>(ctx);
		if constexpr (std::endian::native == std::endian::big) {
			tmp = RevereseByte(tmp);
		}
		*res = tmp;
	}
};

template <std::ranges::input_range T>
struct BuiltInPacker<T> {
	using Value = std::ranges::range_value_t<T>;

	void Serialize(const T& val, auto& ctx) {
		size_t size = std::size(val);
		ContextWriteValue(size, ctx);
		if constexpr (AnyOf<Value, char, unsigned char> && std::ranges::random_access_range<T>) {
			if constexpr (requires(T & t) { t.data(); }) {
				auto data = val.data();
				ContextWrite(data, data + size, ctx);
				return;
			}
		}
		for (auto& elem : val)
			SerializeImpl(elem, ctx);
	}

	void Deserialize(T* res, auto& ctx) {
		auto size = ContextReadValue<size_t>(ctx);
		if constexpr (AnyOf<Value, char, unsigned char> && std::ranges::random_access_range<T>) {
			if constexpr (requires(T & t) { t.data(); }) {
				res->resize(size * sizeof(Value));
				ContextRead(res->begin(), res->end(), ctx);
				return;
			}
		}
		Value tmp;
		for (size_t i = 0; i < size; i++) {
			DeserializeImpl(&tmp, ctx);
			if constexpr (requires(T & t) { t.emplace_back(std::declval<Value>()); })
				res->emplace_back(tmp);
			else if constexpr (requires(T & t) { t.emplace(std::declval<Value>()); })
				res->emplace(tmp);
			else if constexpr (requires(T & t) { t.push_back(std::declval<Value>()); })
				res->push_back(tmp);
			else {
				static_assert(kAlwaysFalse<T>, "Containers using default serialization must implement emplace_back || emplace || push_back functions to support inserting data at the end, or customize serialization!");
			}
		}
	}
};
}
}	// namespace detail