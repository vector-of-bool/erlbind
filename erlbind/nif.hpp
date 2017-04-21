#ifndef ERLBIND_NIF_HPP_INCLUDED
#define ERLBIND_NIF_HPP_INCLUDED

#include "common.hpp"
#include "convert.hpp"
#include "atom.hpp"

#include <erl_nif.h>

#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace erl {

class module_definition {
    std::vector<::ErlNifFunc> _funcs;
    ::ErlNifEntry _nif_entry;
    std::string _name;

public:
    explicit module_definition(std::string name)
        : _name(name) {
        _nif_entry.major = ERL_NIF_MAJOR_VERSION;
        _nif_entry.minor = ERL_NIF_MINOR_VERSION;
        _nif_entry.name = _name.data();
        _nif_entry.load = nullptr;
        _nif_entry.reload = nullptr;
        _nif_entry.upgrade = nullptr;
        _nif_entry.unload = nullptr;
        _nif_entry.vm_variant = ERL_NIF_VM_VARIANT;
        _nif_entry.options = ERL_NIF_ENTRY_OPTIONS;
    }
    ::ErlNifEntry* nif_entry() {
        _nif_entry.num_of_funcs = _funcs.size();
        _nif_entry.funcs = _funcs.data();
        return &_nif_entry;
    }

    void register_fn(const char* name,
                     ::ERL_NIF_TERM (*wrapper)(ErlNifEnv*, int, const ::ERL_NIF_TERM*),
                     unsigned arity) {
        ::ErlNifFunc func{name, arity, wrapper, 0};
        _funcs.push_back(func);
    }
};

namespace detail {

template <typename...> struct tag {};

class function_registerer {
public:
    template <typename Ret, typename... Args>
    function_registerer(module_definition& mod,
                        const char* name,
                        ::ERL_NIF_TERM (*wrapper)(ErlNifEnv*, int, const ::ERL_NIF_TERM*),
                        tag<Ret(Args...)>) {
        mod.register_fn(name, wrapper, sizeof...(Args));
    }
};

template <typename Fn, typename... Args, std::size_t... Is>
auto apply_impl(Fn&& fn, const std::tuple<Args...>& args, std::index_sequence<Is...>) {
    return fn(std::get<Is>(args)...);
}

template <typename Fn, typename... Args> auto apply(Fn&& fn, const std::tuple<Args...>& args) {
    return apply_impl(fn, args, std::make_index_sequence<sizeof...(Args)>{});
}

template <typename... Ts, std::size_t... Is>
std::tuple<Ts...> build_args_tuple(::ErlNifEnv* env,
                                   int argc,
                                   const ::ERL_NIF_TERM* argv,
                                   std::index_sequence<Is...>) {
    return std::tuple<Ts...>(erl::from_erl<std::decay_t<Ts>>(env, argv[Is])...);
}

template <typename Signature> struct call_helper;

template <typename... Args> struct call_helper<void(Args...)> {
    template <typename Func>
    static ERL_NIF_TERM call(Func, ::ErlNifEnv* env, int argc, const ::ERL_NIF_TERM* argv) {
    }
};

template <typename Ret, typename... Args> struct call_helper<Ret(Args...)> {
    template <typename Func>
    static ERL_NIF_TERM call(Func fn, ::ErlNifEnv* env, int argc, const ::ERL_NIF_TERM* argv) {
        try {
            auto tup = build_args_tuple<Args...>(env,
                                                argc,
                                                argv,
                                                std::make_index_sequence<sizeof...(Args)>{});
            auto ret = apply(fn, tup);
            return erl::to_erl(env, ret);
        } catch (const conversion_failed& conv) {
            auto error = to_erl(env, std::make_tuple("Invalid argument", conv.what()));
            return ::enif_raise_exception(env, error);
        }
    }
};

template <typename FuncPtr>
ERL_NIF_TERM call_fn(FuncPtr fn, ::ErlNifEnv* env, int argc, const ::ERL_NIF_TERM* argv) {
    return call_helper<signature_type_t<FuncPtr>>::call(fn, env, argc, argv);
}
}
}

// ERL_NIF_INIT

#define ERLBIND_PASTE_1(a, b) a##b
#define ERLBIND_PASTE(a, b) ERLBIND_PASTE_1(a, b)
#define ERLBIND_STRINGIFY_1(a) #a
#define ERLBIND_STRINGIFY(a) ERLBIND_STRINGIFY_1(a)

#define ERLBIND_MODULE(modname)                                                                    \
    /* Helper functions */                                                                         \
    namespace erlbind_impls {                                                                      \
    static inline void erlbind_module_init_fn(::erl::module_definition&);                          \
    static ::erl::module_definition erlbind_current_module{ERLBIND_STRINGIFY(modname)};            \
    }                                                                                              \
    ERL_NIF_INIT_GLOB                                                                              \
    extern "C" ERLBIND_EXPORT_API::ErlNifEntry* nif_init(ERL_NIF_INIT_ARGS) {                      \
        erlbind_impls::erlbind_module_init_fn(erlbind_impls::erlbind_current_module);              \
        ERL_NIF_INIT_BODY;                                                                         \
        return erlbind_impls::erlbind_current_module.nif_entry();                                  \
    }                                                                                              \
    void erlbind_impls::erlbind_module_init_fn(::erl::module_definition& module)

#define ERLBIND_FUNCTION_PLAIN_1(count, funcname, rt, ...)                                         \
    /* This dummy function is only here so we can get the signature of the user function before    \
     * their function has been declared */                                                         \
    static rt funcname(__VA_ARGS__);                                                               \
    static ERL_NIF_TERM ERLBIND_PASTE(funcname, _wrapper)(::ErlNifEnv * env,                       \
                                                          int argc,                                \
                                                          const ERL_NIF_TERM* argv) {              \
        return ::erl::detail::call_fn(funcname, env, argc, argv);                                  \
    }                                                                                              \
    using ERLBIND_PASTE(fn_signature_, count)                                                      \
        = ::erl::detail::signature_type_t<decltype(funcname)>;                                     \
    ::erl::detail::function_registerer                                                             \
        ERLBIND_PASTE(registerer_,                                                                 \
                      count){::erlbind_impls::erlbind_current_module,                              \
                             ERLBIND_STRINGIFY(funcname),                                          \
                             ERLBIND_PASTE(funcname, _wrapper),                                    \
                             erl::detail::tag<ERLBIND_PASTE(fn_signature_, count)>{}};             \
    static rt funcname(__VA_ARGS__)

#define ERLBIND_FUNCTION_PLAIN(funcname, rt, ...)                                                  \
    ERLBIND_FUNCTION_PLAIN_1(__COUNTER__, funcname, rt, __VA_ARGS__)

#endif  // ERLBIND_NIF_HPP_INCLUDED