#pragma once
#include <tuple>
#include "concepts.h"
#include "utility.h"

namespace stream_view
{

template <class Class1, class Class2>
struct ParallelLinked;
template <class Class1, class Class2>
struct PipeLinked;
template <class Class1, class Class2>
struct SyncLinked;
template <class Class1, class Class2>
struct TransformLinked;

template <class Derived>
struct AccumulatorInterface
{
    static_assert(
        std::is_same_v<Derived,
                       std::remove_cvref_t<Derived>>);
    using This = Derived;

    template <class T>
    SV_FORCE_INLINE constexpr This &operator<<(T &&value)
    {
        auto &p = cast();
        stream_view::add(cast(), std::forward<T>(value));
        return p;
    }

    SV_FORCE_INLINE constexpr Derived &cast() noexcept
    {
        static_assert(
            std::derived_from<
                Derived,
                AccumulatorInterface<Derived>>,
            "AccumulatorInterface's template argument "
            "Derived must derive from "
            "AccumulatorInterface<Derived>."
            "");

        static_assert(
            Accumulator<Derived>,
            "AccumulatorInterface's template argument "
            "Derived must models the Accumulator concept "
            "");
        return static_cast<Derived &>(*this);
    }

    SV_FORCE_INLINE constexpr const Derived &cast()
        const noexcept
    {
        static_assert(
            std::derived_from<Derived,
                              AccumulatorInterface>,
            "AccumulatorInterface's template argument "
            "Derived must derive from "
            "AccumulatorInterface<Derived>."
            "");

        static_assert(
            Accumulator<Derived>,
            "AccumulatorInterface's template argument "
            "Derived must models the Accumulator concept "
            "");
        return static_cast<const Derived &>(*this);
    }
};


#define DEFINE_OPERATOR_FOR_ACCUMULATOR_INTERFACE(Ret, op) \
    template <class Left, class Right>                     \
    requires std::derived_from<                            \
        std::decay_t<Left>,                                \
        AccumulatorInterface<std::decay_t<Left>>>          \
        &&std::derived_from<                               \
            std::decay_t<Right>,                           \
            AccumulatorInterface<std::decay_t<Right>>>     \
            SV_FORCE_INLINE constexpr Ret<Left, Right>     \
            operator op(Left &&lhs, Right &&rhs)           \
    {                                                      \
        return {                                           \
            .left = std::forward<Left>(lhs),               \
            .right = std::forward<Right>(rhs),             \
        };                                                 \
    }
#define COMMA ,
DEFINE_OPERATOR_FOR_ACCUMULATOR_INTERFACE(ParallelLinked,
                                          COMMA)
DEFINE_OPERATOR_FOR_ACCUMULATOR_INTERFACE(PipeLinked, |)
DEFINE_OPERATOR_FOR_ACCUMULATOR_INTERFACE(SyncLinked, &)
#undef COMMA
}// namespace stream_view