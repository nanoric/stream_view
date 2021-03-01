#pragma once
#include "../interface.h"

namespace stream_view
{

/*! @brief Sum values.
 sum() = sum(<elements passed by add()>) - sum(<elements pass by remove()>)
*/
template <class _SumType,
          class _Plus = std::plus<>,
          class _Minus = std::minus<>>
struct Sum
    : AccumulatorInterface<Sum<_SumType, _Plus, _Minus>>
{
    using SumType = _SumType;
    using Plus = _Plus;
    using Minus = _Minus;

    Sum() = default;
    Sum(Sum&rhs) = delete;

private:
    SumType _sum{};

public:
    SV_FORCE_INLINE
    constexpr auto sum() const
    {
        return _sum;
    }

    SV_FORCE_INLINE
    void clear() noexcept
    {
        _sum = 0;
    }

public:
    template <class T, class AddFunc, class RemoveFunc>
    SV_FORCE_INLINE void add(T &&value,
                             AddFunc &&on_added,
                             RemoveFunc &&on_removing)
    {
        _sum = Plus{}(_sum, value);
    }

    template <class T, class AddFunc, class RemoveFunc>
    SV_FORCE_INLINE void remove(T &&value,
                                AddFunc &&on_added,
                                RemoveFunc &&on_removing)
    {
        _sum = Minus{}(_sum, value);
    }
};
};// namespace stream_view
