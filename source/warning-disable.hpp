#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Wextra"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma gcc diagnostic push
#pragma gcc diagnostic ignored "-Wall"
#pragma gcc diagnostic ignored "-Wextra"
#elif defined(_MSC_VER)
#pragma warning(push, 0)
#else
#endif
