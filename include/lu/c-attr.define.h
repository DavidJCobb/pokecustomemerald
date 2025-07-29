
// Attribute to be applied to functions that take pointer arguments, 
// to indicate that the arguments must not be null. Macro parameters 
// are a list of one-based argument indices.
#if __GNU_C__
   #if __STDC_VERSION__ < 202311L
      #define NON_NULL_PARAMS(...) __attribute__((nonnull (__VA_ARGS__)))
   #else
      #define NON_NULL_PARAMS(...) [[gnu::nonnull(__VA_ARGS__)]]
   #endif
#else
   #define NON_NULL_PARAMS(...)
#endif

// Indicates that the pointer returned by a function will never be 
// null.
#if __GNU_C__
   #if __STDC_VERSION__ < 202311L
      #define RETURNS_NON_NULL __attribute__((returns_nonnull))
   #else
      #define RETURNS_NON_NULL [[gnu::returns_nonnull]]
   #endif
#else
   #define RETURNS_NON_NULL
#endif

#if __STDC_VERSION__ >= 202311L
   #define NODISCARD [[nodiscard]]
#else
   #if __GNU_C__
      #define NODISCARD __attribute__((warn_unused_result))
   #else
      #define NODISCARD
   #endif
#endif

#if __STDC_VERSION__ >= 202311L
   #define FALLTHROUGH [[fallthrough]]
#else
   #if __GNU_C__
      #define FALLTHROUGH __attribute__((fallthrough))
   #else
      #define FALLTHROUGH
   #endif
#endif
