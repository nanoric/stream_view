#pragma once
#include <ranges>
#include <set>
#include "../interface.h"

namespace stream_view
{

template <class _Value,
          class _Container = std::multiset<_Value>>
struct Ordered: AccumulatorInterface<Ordered<_Value>>,
                private _Container
{
    using Value = _Value;
    using Container = _Container;

public:
    template <class Allocator = Container::allocator_type>
    constexpr Ordered(Allocator &&allocator = Allocator{})
        : Container(std::forward<Allocator>(allocator))
    {
    }

    SV_FORCE_INLINE
    bool has_value() const
    {
        return !std::ranges::empty(*this);
    }
    SV_FORCE_INLINE
    Value low_value() const
    {
        return *std::ranges::begin(*this);
    }
    SV_FORCE_INLINE
    std::optional<Value> low() const
    {
        if(has_value())
        {
            return low_value();
        }
        return {};
    }
    SV_FORCE_INLINE
    Value high_value() const
    {
        return *std::ranges::rbegin(*this);
    }
    SV_FORCE_INLINE
    std::optional<Value> high() const
    {
        if(has_value())
        {
            return high_value();
        }
        return {};
    }

public:
    using Container::begin;
    using Container::clear;
    using Container::empty;
    using Container::end;
    using Container::size;

public:
    template <class T, class AddFunc, class RemoveFunc>
    SV_FORCE_INLINE void add(T &&value,
                             AddFunc &&on_added,
                             RemoveFunc &&on_removing)
    {
        const auto &stored =
            *Container::insert(end(),
                               std::forward<T>(value));
        on_added(stored);
    }

    template <class T, class AddFunc, class RemoveFunc>
    SV_FORCE_INLINE void remove(T &&value,
                                AddFunc &&on_added,
                                RemoveFunc &&on_removing)
    {
        auto it = Container::find(value);
        if(it != end())
        {
            on_removing(*it);
            Container::erase(it);
        }
    }
};

};// namespace stream_view
