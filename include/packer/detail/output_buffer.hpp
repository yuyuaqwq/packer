#pragma once
#include <concepts>

namespace packer {
namespace detail {
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

using OutputBufferIterator = std::back_insert_iterator<BasicOutputBuffer<char>>;
}
}