#pragma once
#include <concepts>

namespace stream_view
{
template <class T>
constexpr auto _dummy(const std::remove_cvref_t<T> &val){};


template <class T>
concept Accumulator = true;// TODO: fixme


template <class T>
concept _has_add = requires(T v)
{
    v.add(std::declval<typename T::X>(),
          &_dummy<typename T::X>,
          &_dummy<typename T::X>);
};

template <class T>
concept _has_remove = requires(T v)
{
    v.remove(std::declval<typename T::X>(),
             &_dummy<typename T::X>,
             &_dummy<typename T::X>);
};
}// namespace stream_view