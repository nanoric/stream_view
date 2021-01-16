#pragma once
#include <type_traits>
#include "interface.h"
#include "utility.h"

namespace stream_view
{

#define FORWARD(target, method, on_added, on_removing)     \
    [&, this]<typename T0>(T0 &&value) {                   \
        stream_view::method(target,                        \
                            std::forward<T0>(value),       \
                            on_added,                      \
                            on_removing);                  \
    }

#define FORWARD_TO_RIGHT_ADD()                             \
    FORWARD(right, add, on_right_added, on_right_removing)
#define FORWARD_TO_RIGHT_REMOVE()                          \
    FORWARD(right,                                         \
            remove,                                        \
            on_right_added,                                \
            on_right_removing)

#define INTERFACE(name)                                    \
    template <class Value,                                 \
              class AddFunc,                               \
              class RemoveFunc>                            \
    SV_FORCE_INLINE void name(                             \
        Value &&value,                                     \
        AddFunc &&on_right_added,                          \
        RemoveFunc &&on_right_removing)

#define REFERENCE_VALUE static_cast<const Value &>(value)
#define FORWARD_VALUE std::forward<Value>(value)

/*! distribute input to all its children,
 *  discards all outputs 
 */
template <class Left, class Right>
struct ParallelLinked
    : AccumulatorInterface<ParallelLinked<Left, Right>>
{
    Left left;
    Right right;

    INTERFACE(add)
    {
        stream_view::add(left, REFERENCE_VALUE);
        stream_view::add(right, FORWARD_VALUE);
    }
    INTERFACE(remove)
    {
        stream_view::remove(left, REFERENCE_VALUE);
        stream_view::remove(right, FORWARD_VALUE);
    }
};
/*! pipe left's output as right's input
 *  data flow is : input -> left -> right -> output
 */
template <class Left, class Right>
struct PipeLinked
    : AccumulatorInterface<PipeLinked<Left, Right>>
{
    Left left;
    Right right;

#define OUTPUT_TO_RIGHT                                    \
    dummy_on_added, FORWARD_TO_RIGHT_ADD()

    INTERFACE(add)
    {
        stream_view::add(left,
                         FORWARD_VALUE,
                         OUTPUT_TO_RIGHT);
    }
#undef OUTPUT_TO_RIGHT
};

/*! set right as left's children, or said sync right with left.
 * which means whenever left added an element, right's add()
 * will be called by that element.
 * and whenever element is being removed from left, right's remove()
 * will be called by that element.
 * */
template <class Left, class Right>
struct SyncLinked
    : AccumulatorInterface<SyncLinked<Left, Right>>
{
    Left left;
    Right right;

#define SYNC_WITH_RIGHT                                    \
    FORWARD_TO_RIGHT_ADD(), FORWARD_TO_RIGHT_REMOVE()

    INTERFACE(add)
    {
        stream_view::add(left,
                         FORWARD_VALUE,
                         SYNC_WITH_RIGHT);
    }
    INTERFACE(remove)
    {
        stream_view::remove(left,
                            FORWARD_VALUE,
                            SYNC_WITH_RIGHT);
    }
#undef SYNC_WITH_RIGHT
};

/*!
 * 
 */
template <class _Transformer>
struct Transform
    : AccumulatorInterface<Transform<_Transformer>>
{
    _Transformer transformer;
    template <class Value, class AddFunc, class RemoveFunc>
    SV_FORCE_INLINE void add(Value &&value,
                             AddFunc &&on_added,
                             RemoveFunc &&on_removing)
    {
        on_added(transformer(FORWARD_VALUE));
    }
    template <class Value, class AddFunc, class RemoveFunc>
    SV_FORCE_INLINE void remove(Value &&value,
                                AddFunc &&on_added,
                                RemoveFunc &&on_removing)
    {
        on_removing(transformer(FORWARD_VALUE));
    }
};

/*! convert input to output.
 */
struct Echo: AccumulatorInterface<Echo>
{
    template <class Value, class AddFunc, class RemoveFunc>
    SV_FORCE_INLINE void add(Value &&value,
                             AddFunc &&on_added,
                             RemoveFunc &&on_removing)
    {
        on_added(REFERENCE_VALUE);
        on_removing(FORWARD_VALUE);
    }
};

/*! keep inputs.
 */
struct Sync: AccumulatorInterface<Sync>
{
    template <class Value, class AddFunc, class RemoveFunc>
    SV_FORCE_INLINE void add(Value &&value,
                             AddFunc &&on_added,
                             RemoveFunc &&on_removing)
    {
        on_added(FORWARD_VALUE);
    }
    template <class Value, class AddFunc, class RemoveFunc>
    SV_FORCE_INLINE void remove(Value &&value,
                                AddFunc &&on_added,
                                RemoveFunc &&on_removing)
    {
        on_removing(FORWARD_VALUE);
    }
};

/*! input->input && input->sub's input
 */
template <class Right>
struct Sub: AccumulatorInterface<Sub<Right>>
{
    Right right;
    template <class Value, class AddFunc, class RemoveFunc>
    SV_FORCE_INLINE void add(Value &&value,
                             AddFunc &&on_added,
                             RemoveFunc &&on_removing)
    {
        // input->input
        stream_view::add(right, REFERENCE_VALUE);
        on_added(FORWARD_VALUE);
    }
    template <class Value, class AddFunc, class RemoveFunc>
    SV_FORCE_INLINE void remove(Value &&value,
                                AddFunc &&on_added,
                                RemoveFunc &&on_removing)
    {
        stream_view::remove(right, REFERENCE_VALUE);
        on_removing(FORWARD_VALUE);
    }
};

template <class Acc>
SV_FORCE_INLINE constexpr auto sub(Acc &&acc)
{
    return Sub<Acc>{
        .right = std::forward<Acc>(acc),
    };
}

template <class L, class R>
SV_FORCE_INLINE constexpr auto parallel(L &&l, R &&r)
{
    return ParallelLinked{
        .left = std::forward<L>(l),
        .right = std::forward<R>(r),
    };
}

inline Echo echo, pipe;
inline Sync sync;

#undef INTERFACE

};// namespace stream_view
