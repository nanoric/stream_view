#pragma once

#ifdef _MSC_VER
#    define SV_MSVC
#elif defined(__clang__)
#    define SV_CLANG
#elif defined(__GNUC__)
#    define SV_GCC
#else
#    pragma warning("unknown compiler")
#endif// _MSC_VER


#if defined(_DEBUG) || defined(DEBUG) || defined(__DEBUG)
#    if defined(NDEBUG)
#        pragma warning("Unable to determine DEBUG/RELEASE")
#    endif
#    define SV_IS_DEBUG() 1
#    define SV_IS_RELEASE() 0
#else
#    define SV_IS_DEBUG() 0
#    define SV_IS_RELEASE() 1
#endif

#ifndef SV_FORCE_INLINE
#    if defined(SV_MSVC)
#        define SV_FORCE_INLINE __forceinline
#    else /*gcc or clang*/
#        define SV_FORCE_INLINE                            \
            __attribute__((                                \
                always_inline)) inline /*additional inline is required for gcc*/
#    endif
#endif

#ifndef SV_NOINLINE
#    if defined(SV_MSVC)
#        define SV_NOINLINE __declspec(noinline)
#    else /*gcc or clang*/
#        define SV_NOINLINE __attribute__((noinline))
#    endif
#endif

#ifndef SV_INLINE
#    define SV_INLINE inline
#endif

#ifndef SV_NODISCARD
#    if __has_cpp_attribute(nodiscard)
#        define SV_NODISCARD [[nodiscard]]
#    else
#        define SV_NODISCARD
#    endif
#endif
