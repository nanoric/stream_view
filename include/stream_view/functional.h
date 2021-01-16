#pragma once
#include <type_traits>
#include "interface.h"
#include "operators.h"

namespace stream_view
{

template <class T>
concept has_call_operator = requires(T val)
{
    &T::operator();
};
template <class T>
concept callable_class =
    std::is_class_v<T> &&has_call_operator<T>;

template <class T>
concept function_pointer = std::is_pointer_v<T>
    &&std::is_function_v<std::remove_pointer_t<T>>;

template <class T>
concept callable =
    function_pointer<std::remove_cvref_t<
        T>> || callable_class<std::remove_cvref_t<T>> || std::is_function_v<std::remove_cvref_t<T>>;

template <class T>
requires std::is_function_v<T> auto &&
function_pointer_to_reference(T &&f)
{
    return f;
}

template <class T>
requires std::is_function_v<T> auto &&
function_pointer_to_reference(T *f)
{
    return *f;
}

template <class Left, class Right>
    requires derived_from_accumulator_interface<Left> //
    && (callable<Right>)                              //
    &&(std::is_function_v<std::remove_cvref_t<Right>>)//
    SV_FORCE_INLINE constexpr auto
    operator&(Left &&lhs, Right &&right)
{
    return SyncLinked<Left, Transform<Right &&>>{
        .left = std::forward<Left>(lhs),
        .right =
            Transform<Right &&>{
                .transformer =
                    function_pointer_to_reference(right),
            },
    };
}

template <class Left, class Right>
    requires derived_from_accumulator_interface<Left>  //
    && (callable<Right>)                               //
    &&(!std::is_function_v<std::remove_cvref_t<Right>>)//
    SV_FORCE_INLINE constexpr auto
    operator&(Left &&lhs, Right &&right)
{
    return SyncLinked<Left, Transform<Right>>{
        .left = std::forward<Left>(lhs),
        .right =
            Transform<Right>{
                .transformer = std::forward<Right>(right),
            },
    };
}

template <class Left, class Right>
    requires callable<Left>                           //
    && (std::is_function_v<std::remove_cvref_t<Left>>)//
    &&(derived_from_accumulator_interface<Right>)     //
    SV_FORCE_INLINE constexpr auto
    operator&(Left &&callable, Right &&rhs)
{
    return SyncLinked<Transform<Left &&>, Right>{
        .left =
            Transform<Left &&>{
                .transformer =
                    function_pointer_to_reference(callable),
            },
        .right = std::forward<Right>(rhs),
    };
}

template <class Left, class Right>
    requires callable<Left>                            //
    && (!std::is_function_v<std::remove_cvref_t<Left>>)//
    &&derived_from_accumulator_interface<Right>        //
    SV_FORCE_INLINE constexpr auto operator&(Left &&left,
                                             Right &&right)
{
    return SyncLinked<Transform<Left>, Right>{
        .left =
            Transform<Left>{
                .transformer = std::forward<Left>(left),
            },
        .right = std::forward<Right>(right),
    };
}
}// namespace stream_view