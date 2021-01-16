#pragma once
#include <concepts>

namespace stream_view
{
#define dummy_on_added                                     \
    ([](const auto &) {                                    \
    })
#define dummy_on_removed dummy_on_added


template <class T>
concept Accumulator = true;// TODO: fixme


template <class T>
concept _has_add = requires(T v)
{
    v.add(std::declval<typename T::X>(),
          dummy_on_added,
          dummy_on_removed);
};

template <class T>
concept _has_remove = requires(T v)
{
    v.remove(std::declval<typename T::X>(),
             dummy_on_added,
             dummy_on_removed);
};
}// namespace stream_view