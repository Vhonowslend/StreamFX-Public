#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma gcc diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#else
#endif
