#pragma once
#include <list>
#include <ranges>
#include <set>
#include <xutility>
#include "interface.h"

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
    class _TimePoint,

    // Container used to hold elements.
    class _Container = std::list<_TimePoint>,

    // Operator used to
    class _Operator = std::minus<>,

    // Comparator used to determine if element is out of range.
    class _Comparator = std::less<>>
struct Window: AccumulatorInterface<Window<_TimePoint,
                                           _Container,
                                           _Operator,
                                           _Comparator>>,
               private _Container
{
public:
    using Operator = _Operator;
    using Duration =
        decltype(Operator{}(std::declval<_TimePoint>(),
                            std::declval<_TimePoint>()));
    using Container = _Container;
    using Comparator = _Comparator;
    using TimePoint = _TimePoint;
    using Value = TimePoint;

private:
    Duration _window_size;
    bool _filled = false;

public:
    template <class Allocator =
                  typename Container::allocator_type>
    constexpr Window(const Duration &window_size,
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
    using Container::empty;
    using Container::end;
    using Container::size;

    constexpr auto front() const
    {
        return *std::ranges::begin(*this);
    }
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

private:
    SumType _sum{};

public:
    SV_FORCE_INLINE
    constexpr auto sum() const
    {
        return _sum;
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

public:
    using Container::begin;
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
            *Container::emplace(end(),
                                std::forward<T>(value));
        on_added(stored);
    }

    template <class T, class AddFunc, class RemoveFunc>
    SV_FORCE_INLINE void remove(T &&value,
                                AddFunc &&on_added,
                                RemoveFunc &&on_removing)
    {
        auto it = std::forward<T>(value);
        if(it != end())
        {
            on_removing(*it);
        }
        Container::erase(it);
    }
};

template <class _Value,
          class _Container = std::multiset<_Value>>
struct Low: Ordered<_Value, _Container>
{
    using Container = _Container;
    using Value = typename Container::Value;

    SV_FORCE_INLINE
    bool has_value() const
    {
        return !Container::empty();
    }
    SV_FORCE_INLINE
    Value value() const
    {
        return *Container::begin();
    }
    SV_FORCE_INLINE
    std::optional<Value> low() const
    {
        if(has_value())
        {
            return value();
        }
        return {};
    }
};

template <class _Value,
          class _Container = std::multiset<_Value>>
struct High: Ordered<_Value, _Container>
{
    using Container = _Container;
    using Value = typename Container::Value;

    SV_FORCE_INLINE
    bool has_value() const
    {
        return !Container::empty();
    }
    SV_FORCE_INLINE
    Value value() const
    {
        return *Container::rbegin();
    }
    SV_FORCE_INLINE
    std::optional<Value> high() const
    {
        if(has_value())
        {
            return value();
        }
        return {};
    }
};

};// namespace stream_view
