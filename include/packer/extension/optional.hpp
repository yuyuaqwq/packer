#pragma once
#include <optional>
#include <packer/detail/declaration.hpp>

namespace packer {
namespace detail {
template <typename T>
struct BuiltInPacker<std::optional<T>> {
	using Type = std::optional<T>;

	void Serialize(const Type& val, auto& ctx) {
		auto b = val.has_value();
		SerializeTo(ctx.iter(), b);
		if (b) {
			SerializeTo(ctx.iter(), val.value());
		}
	}

	void Deserialize(Type* res, auto& ctx) {
		bool b;
		DeserializeTo(ctx.iter(), &b);
		if (b) {
			T tmp;
			DeserializeTo(ctx.iter(), &tmp);
			res->emplace(std::move(tmp));
		}
	}
};
}	// namespace detail
}	// namespace packer