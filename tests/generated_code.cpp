/*
In this example we assume there are some trade ticks.
every tick contains the average trading price and number of trades
happened since last tick.

whenver a new tick comes, we want to calculates the turnover inside
the 1st, 2nd and 3rd seconds.
Also, we want to get the lowest and highest price in the nearest 1.5 seconds.

So, we have to create two pipes, one for calculating turnover in the first 3 seconds
another for getting lowest and highest price for the first 1.5 seconds.
*/
#include <chrono>
#include <stream_view/functional.h>
#include <stream_view/stream_view.h>


using namespace stream_view;

using namespace std::chrono_literals;
using Duration = std::chrono::seconds;
using TimePoint =
    std::chrono::time_point<std::chrono::system_clock,
                            Duration>;

/* source data, unprocessed
 */
struct Tick: TimePoint
{
    TimePoint &time_point()
    {
        return *this;
    }
    double low;
    double high;
    double avg_price;
    int trades;
};
using LowHigh = std::tuple<double, double>;

auto get_turnover = [](const Tick &tick) {
    return tick.avg_price * tick.trades;
};

SV_FORCE_INLINE
auto xget_turnover(const Tick &tick)
{
    return tick.avg_price * tick.trades;
}

SV_FORCE_INLINE
auto get_low_high(const Tick &tick)
{
    return std::make_tuple(tick.low, tick.high);
}

SV_FORCE_INLINE
auto get_low(const Tick &tick)
{
    return tick.low;
}

SV_FORCE_INLINE
auto get_high(const Tick &tick)
{
    return tick.high;
}

auto get_fake(int)
{
}

struct Example
{
    DurationWindow<Tick> w1{1s};
    Sum<double> sum1;
    DurationWindow<Tick> w2{1s};
    Sum<double> sum2;
    DurationWindow<Tick> w3{1s};
    Sum<double> sum3;

    DurationWindow<Tick> wx1{1s};
    Ordered<double> low;
    Ordered<double> high;
    Sum<double> sumx1;
    Sum<double> sumx2;

    SV_FORCE_INLINE auto pipe()
    {
        auto p1 =                          //
            w1 & sub(get_turnover & sum1)  // & sub(...)
            | w2 & sub(get_turnover & sum2)//
            | sub(w3 & get_turnover & sum3)// | sub(...)
            ;
        auto p2 =//
            wx1  //
            & parallel(
                (get_low & low)               //
                ,                             //
                (get_high & high)             //
                ,                             //
                (xget_turnover & sumx1)       // function op
                ,                             //
                (sync & xget_turnover & sumx2)// op function
                )                             //
            ;
        // if std::move is omitted, p1 p2 will be referenced,
        return parallel(std::move(p1), std::move(p2));
    }
};

int main()
{
    Example e;
    constexpr auto size = 20;
    Tick ticks[size];
    for(int i = 0; i < size; ++i)
    {
        auto &tick = ticks[i];
        tick.time_point() = TimePoint(1s * i);
        tick.low = i - 1;
        tick.high = i + 1;
        tick.avg_price = i;
        tick.trades = 2;
    }

    // these two loops should generates the same code.
    // it is not easy to make a automatic test for this.
    // So this test is verified manually.
    {
        for(int i = 0; i <= 9; ++i)
            e.pipe() << ticks[i];
    }
    {
        auto p = e.pipe();
        for(int i = 0; i <= 9; ++i)
            p << ticks[i];
    }
    // for MSVC, it does generate the same code.
};
