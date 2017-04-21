#ifndef ERLBIND_COMMON_HPP_INCLUDED
#define ERLBIND_COMMON_HPP_INCLUDED

#ifdef _MSC_VER
#define ERLBIND_EXPORT_API __declspec(dllexport)
#else
#define ERLBIND_EXPORT_API __attribute__((visibility("default")))
#endif

namespace erl {

namespace detail {
template <typename FnPtr> struct signature_type;

template <typename Ret, typename... Args> struct signature_type<Ret(Args...)> {
    using type = Ret(Args...);
};

template <typename Ret, typename... Args> struct signature_type<Ret (*)(Args...)> {
    using type = Ret(Args...);
};

template <typename Sig> using signature_type_t = typename signature_type<Sig>::type;
}
}

#endif  // ERLBIND_COMMON_HPP_INCLUDED