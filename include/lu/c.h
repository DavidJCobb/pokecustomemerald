
#if __STDC_VERSION__ < 202311L
   #define auto __auto_type
#endif

// Enum underlying types.
#if __STDC_VERSION__ < 202311L
   #define WITH_UNDERLYING_TYPE(type)
#else
   #define WITH_UNDERLYING_TYPE(type) : type
#endif