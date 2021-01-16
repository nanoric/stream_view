#pragma once
#include "concepts.h"
#include "macros.h"


namespace stream_view
{
template <class Acc, class Val>
SV_FORCE_INLINE Acc &add(Acc &acc, Val &&value)
{
    acc.add(std::forward<Val>(value),
            dummy_on_added,
            dummy_on_removed);
    return acc;
}

template <class Acc,
          class Val,
          class AddFunc,
          class RemoveFunc>
SV_FORCE_INLINE Acc &add(Acc &acc,
                         Val &&value,
                         AddFunc &&on_right_added,
                         RemoveFunc &&on_right_removed)

{
    acc.add(std::forward<Val>(value),
            std::forward<AddFunc>(on_right_added),
            std::forward<RemoveFunc>(on_right_removed));
    return acc;
}


template <class Acc, class Val>
SV_FORCE_INLINE Acc &remove(Acc &acc, Val &&value)
{
    acc.remove(std::forward<Val>(value),
               dummy_on_added,
               dummy_on_removed);
    return acc;
}

template <class Acc,
          class Val,
          class AddFunc,
          class RemoveFunc>
SV_FORCE_INLINE Acc &remove(Acc &acc,
                            Val &&value,
                            AddFunc &&on_right_added,
                            RemoveFunc &&on_right_removed)
{
    acc.remove(std::forward<Val>(value),
               std::forward<AddFunc>(on_right_added),
               std::forward<RemoveFunc>(on_right_removed));
    return acc;
}


};// namespace stream_view
