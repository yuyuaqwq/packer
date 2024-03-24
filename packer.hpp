#pragma once
#include <concepts>

namespace packer {
template <typename T>
inline constexpr bool always_false = false;

template <typename T>
struct Packer {};

namespace internal {
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

static constexpr size_t kOutputBufferSize = 256;

template <typename T>
class BasicOutputBuffer {
public:
	using value_type = T;

	BasicOutputBuffer(T* const ptr, size_t size, size_t capacity)
		: ptr_{ ptr }, size_{ size }, capacity_{ capacity }
	{}

	void push_back(const T& value) {
		TryReserve(size_ + 1);
		ptr_[size_++] = value;
	}

	void Clear() {
		size_ = 0;
	}

	auto size() const { return size_; }
	auto capacity() const { return capacity_; }

protected:
	virtual void Grow(size_t capacity) = 0;

private:
	void TryReserve(const size_t new_capacity) {
		if (new_capacity > capacity_) {
			Grow(new_capacity);
		}
	}

	T* const ptr_;
	size_t size_;
	size_t capacity_;
};

struct OutputBufferTraits {
	explicit OutputBufferTraits(ptrdiff_t) {}

	size_t size() const {
		return 0;
	}

	size_t Limit(const size_t size) {
		return size;
	}
};

template <typename OutputIt, typename T, typename Traits = OutputBufferTraits>
class OutputBuffer : public BasicOutputBuffer<T> {
public:
	using Inherit = BasicOutputBuffer<T>;

	explicit OutputBuffer(OutputIt out, ptrdiff_t size = kOutputBufferSize)
		: out_{ std::move(out) }, traits_{ size }, Inherit{ buffer_, 0, kOutputBufferSize } {}

	~OutputBuffer() {
		if (this->size()) {
			Flush();
		}
	}

	auto out() {
		Flush();
		return std::move(out_);
	}

private:
	void Grow(size_t capacity) final {
		if (this->size() == kOutputBufferSize) {
			Flush();
		}
	}

	void Flush() {
		auto size = this->size();
		this->Clear();
		const auto end = buffer_ + traits_.Limit(size);
		out_ = std::copy(buffer_, end, std::move(out_));
	}

	Traits traits_;
	OutputIt out_;
	T buffer_[kOutputBufferSize];
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
struct Serizlizer {
	void operator()(const T& val, ContextType& ctx) {
		if constexpr (HasImplSerialize<T, ContextType>)
			Packer<T>{}.Serialize(val, ctx);
		else if constexpr (HasImplBuiltInSerialize<T, ContextType>)
			BuiltInPacker<T>{}.Serialize(val, ctx);
		else
			static_assert(always_false<T>, "You haven't specialized the Packer::Serialize for this type T yet!");
	}
};

template <typename T, typename ContextType>
struct Deserizlizer {
	void operator()(T* res, ContextType& ctx) {
		if constexpr (HasImplDeserialize<T, ContextType>)
			Packer<T>{}.Deserialize(res, ctx);
		else if constexpr (HasImplBuiltInDeserialize<T, ContextType>)
			BuiltInPacker<T>{}.Deserialize(res, ctx);
		else
			static_assert(always_false<T>, "You haven't specialized the Packer::Deserialize for this type T yet!");
	}
};
}	// namespace internal


template <std::output_iterator<const char&> OutputIt, typename T>
OutputIt SerializeTo(OutputIt it, const T& val) {
	internal::OutputBuffer<OutputIt, char> buffer{std::move(it)};
	internal::BasicContext ctx{std::back_insert_iterator{ buffer }};
	internal::Serizlizer<T, decltype(ctx)>{}(val, ctx);
	return buffer.out();
}

template <std::input_iterator InputIt, typename T>
void DeserializeTo(InputIt it, T* res) {
	internal::BasicContext ctx{it};
	internal::Deserizlizer<T, decltype(ctx)>{}(res, ctx);
}


namespace internal {
template <typename T>
requires (sizeof(T) == sizeof(char))
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
}	// namespace packer