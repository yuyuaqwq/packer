#pragma once
#include <packer/detail/algorithm.hpp>
#include <packer/detail/context.hpp>
#include <packer/detail/structure_binding.hpp>
#include <packer/detail/output_buffer.hpp>
#include <array>
#include <bit>
#include <assert.h>

namespace packer {
namespace detail {
template <typename T, typename Context>
concept HasIntrusiveSerialize = requires(T t, Context ctx) { t.Serialize(ctx); };
template <typename T, typename Context>
concept HasIntrusiveDeserialize = requires(T t, Context ctx) { t.Deserialize(ctx); };


template <typename T, typename Context>
void SerializeImpl(const T& val, Context& ctx) {
	static_assert(!(HasImplSerialize<T, Context> && HasIntrusiveSerialize<T, Context>), "There's no need to implement both intrusive and non-intrusive serialization at the same program!");

	if constexpr (HasImplSerialize<T, Context>)
		Packer<T>{}.Serialize(val, ctx);
	else if constexpr (HasIntrusiveSerialize<T, Context>)
		val.Serialize(ctx);
	else if constexpr (HasImplBuiltInSerialize<T, Context>)
		BuiltInPacker<T>{}.Serialize(val, ctx);
	else {
		static_assert(kAlwaysFalse<T>, "You haven't specialized the Packer::Serialize for this type T yet!");
	}
}

template <typename T, typename Context>
void DeserializeImpl(T* res, Context& ctx) {
	static_assert(!(HasImplDeserialize<T, Context> && HasIntrusiveDeserialize<T, Context>), "There's no need to implement both intrusive and non-intrusive deserialization at the same program!");

	using Type = std::remove_cvref_t<T>;
	auto noconst_res = const_cast<Type*>(res);
	if constexpr (HasImplDeserialize<Type, Context>)
		Packer<Type>{}.Deserialize(noconst_res, ctx);
	else if constexpr (HasIntrusiveDeserialize<Type, Context>)
		noconst_res->Deserialize(ctx);
	else if constexpr (HasImplBuiltInDeserialize<Type, Context>)
		BuiltInPacker<Type>{}.Deserialize(noconst_res, ctx);
	else {
		static_assert(kAlwaysFalse<T>, "You haven't specialized the Packer::Deserialize for this type T yet!");
	}
}
}	// namespace detail


//TODO SerializeTo支持wchar的OutputIt
//TODO SerializeTo支持限制OutputIt的Max size

template <std::output_iterator<const char&> OutputIt, typename T>
OutputIt SerializeTo(OutputIt it, const T& val) {
	if constexpr (std::is_same_v<OutputIt, detail::OutputBufferIterator>) {
		detail::BasicOutputContext ctx{it};
		detail::SerializeImpl(val, ctx);
		return it;
	}
	else {
		detail::OutputBuffer<OutputIt, char> buffer{std::move(it)};
		detail::BasicOutputContext ctx{detail::OutputBufferIterator{ buffer }};
		detail::SerializeImpl(val, ctx);
		return buffer.out();
	}
}

//TODO DeserializeTo支持wchar的InputIt
//TODO DeserializeTo支持限制InputIt的Max size

template <std::input_iterator InputIt, typename T>
InputIt DeserializeTo(InputIt it, T* res) {
	detail::BasicInputContext ctx{it};
	detail::DeserializeImpl(res, ctx);
	return ctx.iter();
}


namespace detail {
//TODO BuiltInPacker待处理wchar
template <typename T>
struct BuiltInPacker {
	void Serialize(const T& val, auto& ctx) {
		StructureBinding(val, [&]<typename... Members>(Members&&... members) {
			(SerializeImpl(members, ctx), ...);
		});
	}

	void Deserialize(T* res, auto& ctx) {
		StructureBinding(*res, [&]<typename... Members>(Members&&... members) {
			(DeserializeImpl(std::addressof(members), ctx), ...);
		});
	}
};

template <AnyOf<char, unsigned char, bool> T>
struct BuiltInPacker<T> {
	void Serialize(const T& val, auto& ctx) {
		ContextWriteByte(val, ctx);
	}

	void Deserialize(T* res, auto& ctx) {
		*res = ContextReadByte(ctx);
	}
};

template <typename T, size_t N>
struct BuiltInPacker<T[N]> {
	using Type = T[N];

	void Serialize(const Type& val, auto& ctx) {
		ContextWriteValue(N, ctx);
		if constexpr (AnyOf<T, char, unsigned char>) {
			ContextWrite(val, val + N, ctx);
			return;
		}
		for (size_t i = 0; i < N; i++)
			SerializeImpl(val[N], ctx);
	}

	void Deserialize(Type* res, auto& ctx) {
		auto size = ContextReadValue<size_t>(ctx);
		assert(size == N);
		if constexpr (AnyOf<T, char, unsigned char>) {
			ContextRead(*res, *res + N, ctx);
			return;
		}
		for (size_t i = 0; i < N; i++)
			DeserializeImpl(*res + N, ctx);
	}
};

template <typename T, size_t N>
struct BuiltInPacker<std::array<T, N>> {
	using Type = std::array<T, N>;

	void Serialize(const Type& val, auto& ctx) {
		ContextWriteValue(N, ctx);
		if constexpr (AnyOf<T, char, unsigned char>) {
			ContextWrite(val.begin(), val.end(), ctx);
			return;
		}
		for (size_t i = 0; i < N; i++)
			SerializeImpl(val[i], ctx);
	}

	void Deserialize(Type* res, auto& ctx) {
		auto size = ContextReadValue<size_t>(ctx);
		assert(size == N);
		if constexpr (AnyOf<T, char, unsigned char>) {
			ContextRead(res->begin(), res->end(), ctx);
			return;
		}
		for (size_t i = 0; i < N; i++)
			DeserializeImpl(res->data() + i, ctx);
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
				res->emplace_back(std::move(tmp));
			else if constexpr (requires(T & t) { t.emplace(std::declval<Value>()); })
				res->emplace(std::move(tmp));
			else if constexpr (requires(T & t) { t.push_back(std::declval<Value>()); })
				res->push_back(std::move(tmp));
			else {
				static_assert(kAlwaysFalse<T>, "Containers using default serialization must implement emplace_back || emplace || push_back functions to support inserting data at the end, or customize serialization!");
			}
		}
	}
};
}
}	// namespace detail