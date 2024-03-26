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

template <typename Context, typename InIt>
void ContextWrite(Context& ctx, InIt first, InIt last) {
	ctx.advance_to(std::copy(first, last, ctx.iter()));
}

template <typename Context, std::output_iterator<const char&> OutIt>
void ContextRead(Context& ctx, size_t size, OutIt dest) {
	auto it = ctx.iter();
	for (size_t i = 0; i < size; ++i, ++dest, ++it) {
		*dest = *it;
	}
	ctx.advance_to(it);
}

template <typename Context>
void ContextWriteByte(Context& ctx, char ch) {
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
}
}	// namespace packer