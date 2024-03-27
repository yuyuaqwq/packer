#pragma once
#include <concepts>

namespace packer {
template <typename T>
struct Packer;
namespace detail {
template <typename T>
struct BuiltInPacker;
}	// namespace detail

template <std::output_iterator<const char&> OutputIt, typename T>
OutputIt SerializeTo(OutputIt it, const T& val);

template <std::input_iterator InputIt, typename T>
InputIt DeserializeTo(InputIt it, T* res);
}	// namespace packer