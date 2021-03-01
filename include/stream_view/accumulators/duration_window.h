#pragma once
#include <list>
#include <ranges>
#include "../interface.h"

namespace stream_view
{

/*!
  @brief 
  maintain a sliding window that holds elements.

  @detail 
  whenever you add a new value, 
  any values that outside the range
  [Operator(back(), window_size), back()] will be removed

  # About Element monotonicity:

  For better performance, the default configuration
  assume all your elements are monotonically increasing.

  you can change the Container used to support non-monotonic elements, 
  such as using a multimap as Container

  You can also change the Comparator used 
  to supports elements that monotonically decreasing.

  # About elements that falls on the edges:

  for default configuration, these values are kept(not removed),
  You can change the Comparator used to change this behavior.
  such as using std::less_equal<> to remove them.
 */
template <
    // value type
    class _Value,

    // Container used to hold elements.
    class _Container = std::list<_Value>,

    // Operator used to
    class _Operator = std::minus<>,

    // Comparator used to determine if element is out of range.
    class _Comparator = std::less<>>
struct DurationWindow
    : AccumulatorInterface<DurationWindow<_Value,
                                          _Container,
                                          _Operator,
                                          _Comparator>>,
      private _Container
{
public:
    using Operator = _Operator;
    using Value = _Value;
    using Duration =
        decltype(Operator{}(std::declval<Value>(),
                            std::declval<Value>()));
    using TimePoint =
        decltype(Operator{}(std::declval<Value>(),
                            std::declval<Duration>()));
    using Container = _Container;
    using Comparator = _Comparator;

private:
    Duration _window_size;
    bool _filled = false;

public:
    template <class Allocator =
                  typename Container::allocator_type>
    constexpr DurationWindow(
        const Duration &window_size,
        Allocator &&allocator = Allocator{})
        : Container(std::forward<Allocator>(allocator))
        , _window_size(window_size)
    {
    }
    /*! filled means there is ever an element run out of window,
     */
    [[nodiscard]] inline bool filled() const noexcept
    {
        return this->_filled;
    }

    /*! real-time window size
     * return: Edge{back() - fron()} if not empty
     *         Edge{} if empty.
    */
    [[nodiscard]] constexpr Duration current_window_size()
        const
    {
        if(Container::empty())
            return Duration{};
        return Container::back() - Container::front();
    }

public:
    using Container::begin;
    using Container::clear;
    using Container::empty;
    using Container::end;
    using Container::size;

    //! for default configuration, this is the oldest value
    constexpr auto front() const
    {
        return *std::ranges::begin(*this);
    }

    //! for default configuration, this is the newest value
    constexpr auto back() const
    {
        return *std::ranges::rbegin(*this);
    }

protected:
    SV_FORCE_INLINE
    constexpr bool out_of_range(const TimePoint &value,
                                const TimePoint &edge) const
    {
        return Comparator{}(value, edge);
    }

public:
    template <class T, class AddFunc, class RemoveFunc>
    SV_FORCE_INLINE void add(T &&value,
                             AddFunc &&on_added,
                             RemoveFunc &&on_removing)
    {
        const auto &stored_value =
            *Container::insert(Container::end(),
                               std::forward<T>(value));

        // hook add
        on_added(stored_value);

        auto window_edge =
            Operator{}(stored_value, _window_size);

        // delete all elements that run out of window
        for(auto i = Container::begin();
            i != Container::end();)
        {
            if(auto &&v = *i; out_of_range(v, window_edge))
            {
                // hook remove
                on_removing(v);

                i = Container::erase(i);
                _filled = true;
                continue;
            }
            else
            {
                break;
            }
        }
    }
};
};// namespace stream_view
