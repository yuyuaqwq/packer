#pragma once
#include <concepts>

namespace packer {
template <typename T>
inline constexpr bool kAlwaysFalse = false;
template <typename T, typename... Types>
concept AnyOf = (std::is_same_v<T, Types> || ...);

template <typename T>
struct Packer;

namespace detail {
template <typename T>
struct BuiltInPacker;

template <typename Iter>
class BasicContext {
public:
	template <typename T>
	using BuiltInPackerType = BuiltInPacker<T>;
	template <typename T>
	using PackerType = Packer<T>;

	BasicContext(Iter iter) : iter_{ std::move(iter) } {}

	Iter iter() {
		return std::move(iter_);
	}

	void advance_to(Iter iter) {
		iter_ = std::move(iter);
	}

private:
	Iter iter_;
};

template <std::output_iterator<const char&> Iter>
using BasicOutputContext = BasicContext<Iter>;
template <std::input_iterator Iter>
using BasicInputContext = BasicContext<Iter>;

template <typename T, typename Context>
using BuiltInPackerType = typename Context::template BuiltInPackerType<T>;
template <typename T, typename Context>
using PackerType = typename Context::template PackerType<T>;

template <typename T, typename Context>
concept HasImplBuiltInSerialize = requires(T t, Context ctx) {
	std::declval<BuiltInPackerType<T, Context>>().Serialize(t, ctx);
};
template <typename T, typename Context>
concept HasImplSerialize = requires(T t, Context ctx) {
	std::declval<PackerType<T, Context>>().Serialize(t, ctx);
};

template <typename T, typename Context>
concept HasImplBuiltInDeserialize = requires(T * t, Context ctx) {
	std::declval<BuiltInPackerType<T, Context>>().Deserialize(t, ctx);
};
template <typename T, typename Context>
concept HasImplDeserialize = requires(T * t, Context ctx) {
	std::declval<PackerType<T, Context>>().Deserialize(t, ctx);
};

template <typename>
struct ContextIteratorGetter;

template <template<typename> typename Context, typename Iter>
struct ContextIteratorGetter<Context<Iter>> {
	using Iterator = Iter;
};

template <typename Context>
using ContextIterator = typename ContextIteratorGetter<Context>::Iterator;

template <typename Context>
concept BasicContextConcept = requires(Context ctx) {
	{ctx.iter()} -> std::same_as<ContextIterator<Context>>;
	ctx.advance_to(std::declval<ContextIterator<Context>>());
};
template <typename Context>
concept OutputContextConcept = std::output_iterator<ContextIterator<Context>, const char&> && BasicContextConcept<Context>;
template <typename Context>
concept InputContextConcept = std::input_iterator<ContextIterator<Context>> && BasicContextConcept<Context>;

template <OutputContextConcept Context, typename InIt>
void ContextWrite(InIt beg, InIt end, Context& ctx) {
	ctx.advance_to(std::copy(beg, end, ctx.iter()));
}

template <InputContextConcept Context, std::output_iterator<const char&> OutIt>
void ContextRead(OutIt beg, OutIt end, Context& ctx) {
	auto src_it = ctx.iter();
	for (; beg != end; ++beg, ++src_it) {
		*beg = *src_it;
	}
	ctx.advance_to(src_it);
}

template <OutputContextConcept Context>
void ContextWriteByte(const char ch, Context& ctx) {
	auto it = ctx.iter();
	*it++ = ch;
	ctx.advance_to(it);
}

template <InputContextConcept Context>
char ContextReadByte(Context& ctx) {
	auto it = ctx.iter();
	auto res = *it++;
	ctx.advance_to(it);
	return res;
}

template <typename T, OutputContextConcept Context>
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

template <typename T, InputContextConcept Context>
T ContextReadValue(Context& ctx) {
	static_assert(std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>);
	if constexpr (sizeof(T) == 1) {
		return reinterpret_cast<T>(ContextReadByte(ctx));
	}
	else {
		T res;
		auto buf = reinterpret_cast<char*>(&res);
		ContextRead(buf, buf + sizeof(T), ctx);
		return res;
	}
}
}
}	// namespace packer