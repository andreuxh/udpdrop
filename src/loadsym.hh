#ifndef LOADSYM_HH
#define LOADSYM_HH

#include <stdexcept>

#include <dlfcn.h>

template <typename T, typename... P>
using Func = T(P...);

template <typename T, typename... P>
Func<T, P...> *loadSym(const Func<T, P...>&, const char *symbol)
{
    dlerror();
    auto *func = reinterpret_cast<Func<T, P...>*>(dlsym(RTLD_NEXT, symbol));
    if (!func) {
        const char *error = dlerror();
        std::string message(symbol);
        if (error) {
            message += ": ";
            message += error;
        }
        else {
            message += " loaded as NULL";
        }
        throw std::runtime_error(message);
    }
    return func;
}

#define LOADSYM(symbol) loadSym(symbol, #symbol)

#endif // LOADSYM_HH
