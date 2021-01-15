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
    template <class T, class AddFunc, class RemoveFunc>    \
    SV_FORCE_INLINE void name(                             \
        T &&val,                                           \
        AddFunc &&on_right_added,                          \
        RemoveFunc &&on_right_removing)

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
        stream_view::add(left, std::forward<T>(val));
        stream_view::add(right, std::forward<T>(val));
    }
    INTERFACE(remove)
    {
        stream_view::remove(left, std::forward<T>(val));
        stream_view::remove(right, std::forward<T>(val));
    }
};
/*! pipe left's output as right's input
 *  data flow is : input -> left -> right -> output
 */
template <class Class1, class Class2>
struct PipeLinked
    : AccumulatorInterface<PipeLinked<Class1, Class2>>
{
    Class1 left;
    Class2 right;
#define OUTPUT_TO_RIGHT                                    \
    &_dummy<T>, FORWARD_TO_RIGHT_ADD()

    INTERFACE(add)
    {
        stream_view::add(left,
                         std::forward<T>(val),
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
                         std::forward<T>(val),
                         SYNC_WITH_RIGHT);
    }
    INTERFACE(remove)
    {
        stream_view::remove(left,
                            std::forward<T>(val),
                            SYNC_WITH_RIGHT);
    }
#undef SYNC_WITH_RIGHT
};

};// namespace stream_view
