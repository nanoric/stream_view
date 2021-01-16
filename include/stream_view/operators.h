#pragma once
#include <type_traits>
#include "interface.h"

namespace stream_view
{
template <class T>
concept derived_from_accumulator_interface =
    std::derived_from<
        std::remove_cvref_t<T>,
        AccumulatorInterface<std::remove_cvref_t<T>>>;

#define DEFINE_OPERATOR(Ret, op)                           \
    template <class Left, class Right>                     \
    requires derived_from_accumulator_interface<Left>      \
        &&derived_from_accumulator_interface<Right>        \
            SV_FORCE_INLINE constexpr auto operator op(    \
                Left &&lhs,                                \
                Right &&rhs)                               \
    {                                                      \
        return Ret<Left, Right>{                                        \
            .left = std::forward<Left>(lhs),               \
            .right = std::forward<Right>(rhs),             \
        };                                                 \
    }

#define COMMA ,
DEFINE_OPERATOR(ParallelLinked, COMMA);
DEFINE_OPERATOR(PipeLinked, |);
DEFINE_OPERATOR(SyncLinked, &);

#undef COMMA

#undef DEFINE_OPERATOR
}// namespace stream_view