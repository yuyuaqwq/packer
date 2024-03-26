#pragma once
#include <concepts>

namespace packer {
template <typename T>
struct Packer;

namespace detail {
template <typename T>
struct BuiltInPacker;

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

template <typename OutputContext, typename InIt>
void ContextWrite(InIt beg, InIt end, OutputContext& ctx) {
	ctx.advance_to(std::copy(beg, end, ctx.iter()));
}

template <typename InputContext, std::output_iterator<const char&> OutIt>
void ContextRead(InputContext& ctx, OutIt beg, OutIt end) {
	auto src_it = ctx.iter();
	for (; beg != end; ++beg, ++src_it) {
		*beg = *src_it;
	}
	ctx.advance_to(src_it);
}

template <typename OutputContext>
void ContextWriteByte(const char ch, OutputContext& ctx) {
	auto it = ctx.iter();
	*it++ = ch;
	ctx.advance_to(it);
}

template <typename Context>
char ContextReadByte(Context& ctx) {
	auto it = ctx.iter();
	auto res = *it++;
	ctx.advance_to(it);
	return res;
}

template <typename T, typename Context>
void ContextWriteValue(const T& val, Context& ctx) {
	static_assert(std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>);
	if constexpr (sizeof(T) == 1) {
		ContextWriteByte(reinterpret_cast<const char>(val), ctx);
	}
	else {
		auto beg = reinterpret_cast<const char*>(std::addressof(val));
		auto end = beg + sizeof(T);
		ContextWrite(beg, end, ctx);
	}
}

template <typename T, typename Context>
T ContextReadValue(Context& ctx) {
	static_assert(std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>);
	if constexpr (sizeof(T) == 1) {
		return reinterpret_cast<T>(ContextReadByte(ctx));
	}
	else {
		T res;
		auto buf = reinterpret_cast<char*>(&res);
		ContextRead(ctx, buf, buf + sizeof(T));
		return res;
	}
}
}
}	// namespace packer