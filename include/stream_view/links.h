#pragma once
#include <type_traits>
#include "interface.h"
#include "utility.h"

namespace stream_view
{
#define INTERFACE(name)                                    \
    template <class Value, class Added, class Removing>    \
    SV_FORCE_INLINE void name(Value &&value,               \
                              Added &&on_added,            \
                              Removing &&on_removing)

#define REFERENCE_VALUE static_cast<const Value &>(value)
#define FORWARD_VALUE std::forward<Value>(value)

#define FORWARD(target, method, on_added, on_removing)     \
    [&, this]<typename T0>(T0 &&value) {                   \
        stream_view::method(target,                        \
                            std::forward<T0>(value),       \
                            on_added,                      \
                            on_removing);                  \
    }

#define FORWARD_TO_RIGHT_ADD                               \
    LambdaRightAdd<Right, Added, Removing>                 \
    {                                                      \
        .right = right, .on_added = on_added,              \
        .on_removing = on_removing,                        \
    }
#define FORWARD_TO_RIGHT_REMOVE                            \
    LambdaRightRemove<Right, Added, Removing>              \
    {                                                      \
        .right = right, .on_added = on_added,              \
        .on_removing = on_removing,                        \
    }

// C++'s lambda is not inlined in most of the case.
// so I choose a hand-written lambda
// to increase the probability of being inlined.
// maybe [[gnu::always_inline]] is a good choice
// but there is no such an attribute for MSVC
template <class Right, class Added, class Removing>
struct LambdaRightAdd
{
    std::remove_cvref_t<Right> &right;
    std::remove_cvref_t<Added> &on_added;
    std::remove_cvref_t<Removing> &on_removing;
    template <class Value>
    SV_FORCE_INLINE void operator()(Value &&value)
    {
        stream_view::add(right,
                         FORWARD_VALUE,
                         on_added,
                         on_removing);
    }
};

template <class Right, class Added, class Removing>
struct LambdaRightRemove
{
    std::remove_cvref_t<Right> &right;
    std::remove_cvref_t<Added> &on_added;
    std::remove_cvref_t<Removing> &on_removing;
    template <class Value>
    SV_FORCE_INLINE void operator()(Value &&value)
    {
        stream_view::remove(right,
                            FORWARD_VALUE,
                            on_added,
                            on_removing);
    }
};


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
    INTERFACE(add)
    {
        stream_view::add(left,
                         FORWARD_VALUE,
                         dummy_on_added,
                         FORWARD_TO_RIGHT_ADD);
    }
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
    INTERFACE(add)
    {
        stream_view::add(left,
                         FORWARD_VALUE,
                         FORWARD_TO_RIGHT_ADD,
                         FORWARD_TO_RIGHT_REMOVE);
    }
    INTERFACE(remove)
    {
        stream_view::remove(left,
                            FORWARD_VALUE,
                            FORWARD_TO_RIGHT_ADD,
                            FORWARD_TO_RIGHT_REMOVE);
    }
};

/*!
 * 
 */
template <class _Transformer>
struct Transform
    : AccumulatorInterface<Transform<_Transformer>>
{
    _Transformer transformer;
    INTERFACE(add)
    {
        on_added(transformer(FORWARD_VALUE));
    }
    INTERFACE(remove)
    {
        on_removing(transformer(FORWARD_VALUE));
    }
};

/*! convert input to output.
 */
struct Echo: AccumulatorInterface<Echo>
{
    INTERFACE(add)
    {
        on_added(REFERENCE_VALUE);
        on_removing(FORWARD_VALUE);
    }
};

/*! keep inputs.
 */
struct Sync: AccumulatorInterface<Sync>
{
    INTERFACE(add)
    {
        on_added(FORWARD_VALUE);
    }
    INTERFACE(remove)
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
    INTERFACE(add)
    {
        // input->input
        stream_view::add(right, REFERENCE_VALUE);
        on_added(FORWARD_VALUE);
    }
    INTERFACE(remove)
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
    return ParallelLinked<L, R>{
        .left = std::forward<L>(l),
        .right = std::forward<R>(r),
    };
}
template <class L, class R, class... Args>
SV_FORCE_INLINE constexpr auto parallel(L &&l,
                                        R &&r,
                                        Args &&... args)
{
    return parallel(
        parallel(std::forward<L>(l), std::forward<R>(r)),
        std::forward<Args>(args)...);
}

inline Echo echo, pipe;
inline Sync sync;

#undef INTERFACE

};// namespace stream_view
