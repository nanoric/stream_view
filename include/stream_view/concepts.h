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


template <class T, class Value>
concept _has_add = requires(T v)
{
    v.add(std::declval<Value>(),
          dummy_on_added,
          dummy_on_removed);
};

template <class T, class Value>
concept _has_remove = requires(T v)
{
    v.remove(std::declval<Value>(),
             dummy_on_added,
             dummy_on_removed);
};

template <class T>
concept _has_pipe = requires(const T &v)
{
    v.pipe();
};

template <class T>
concept Pipe = _has_pipe<T>;// TODO: fixme
}// namespace stream_view