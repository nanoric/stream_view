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
    class _Container = std::list<_Value>>

struct FixedWindow
    : AccumulatorInterface<FixedWindow<_Value, _Container>>,
      private _Container
{
public:
    using Value = _Value;
    using Container = _Container;

private:
    size_t _window_size;
    bool _filled = false;

public:
    template <class Allocator =
                  typename Container::allocator_type>
    constexpr FixedWindow(
        size_t window_size,
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

public:
    template <class T, class AddFunc, class RemoveFunc>
    SV_FORCE_INLINE void add(T &&value,
                             AddFunc &&on_added,
                             RemoveFunc &&on_removing)
    {
        const auto &stored_value =
            *Container::insert(Container::end(),
                               std::forward<T>(value));

        on_added(stored_value);

        if(this->_filled)
        {
            auto i = Container::begin();
            on_removing(*i);
            Container::erase(i);
        }
        else
        {
            if(Container::size() == _window_size)
            {
                this->_filled = true;
            }
        }
    }
};
};// namespace stream_view
