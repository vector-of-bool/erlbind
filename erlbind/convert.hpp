#ifndef ERLBIND_CONVERT_HPP_INCLUDED
#define ERLBIND_CONVERT_HPP_INCLUDED

#include <erl_nif.h>

#include <tuple>
#include <utility>
#include <vector>

namespace erl {

class conversion_failed : public std::exception {
    ::ERL_NIF_TERM _term;
    std::string _type;
    std::string _what;

public:
    conversion_failed(::ERL_NIF_TERM term, const std::string type)
        : _term{term}
        , _type{type} {
        _what.resize(512);
        const char* fmt = "Unable to convert %T to value of type '%s'";
        auto size_req = ::enif_snprintf(&_what[0], _what.size(), fmt, term, _type.data());
        if (size_req > _what.size()) {
            _what.resize(size_req);
            auto size_req = ::enif_snprintf(&_what[0], _what.size(), fmt, term, _type.data());
        }
    }

    ::ERL_NIF_TERM term() const {
        return _term;
    }
    const std::string type() const {
        return _type;
    }

    const char* what() const noexcept override {
        return _what.data();
    }
};

template <typename T> struct convert;

template <typename T> ERL_NIF_TERM to_erl(::ErlNifEnv* env, T&& what) {
    return convert<std::decay_t<T>>::to_erl(env, std::forward<T>(what));
}

template <typename T> T from_erl(::ErlNifEnv* env, ERL_NIF_TERM term) {
    return convert<T>::from_erl(env, term);
}

template <> struct convert<ERL_NIF_TERM> {
    static ERL_NIF_TERM to_erl(::ErlNifEnv* env, ERL_NIF_TERM t) {
        return t;
    }
    static ERL_NIF_TERM from_erl(::ErlNifEnv* env, ERL_NIF_TERM t) {
        return t;
    }
};

#define DECLARE_INTEGER_CONVERTER(type, tofn, fromfn)                                              \
    template <> struct convert<type> {                                                             \
        static ERL_NIF_TERM to_erl(::ErlNifEnv* env, type value) {                                 \
            return tofn(env, value);                                                               \
        }                                                                                          \
        static type from_erl(::ErlNifEnv* env, ERL_NIF_TERM term) {                                \
            type ret;                                                                              \
            auto did_convert = fromfn(env, term, &ret);                                            \
            if (!did_convert)                                                                      \
                throw conversion_failed{term, #type};                                              \
            return ret;                                                                            \
        }                                                                                          \
    }

DECLARE_INTEGER_CONVERTER(int, ::enif_make_int, ::enif_get_int);

template <typename T> struct convert<std::vector<T>> {
    static ERL_NIF_TERM to_erl(::ErlNifEnv* env, const std::vector<T>& vec) {
        auto riter = vec.rbegin();
        const auto rend = vec.rend();
        auto list = ::enif_make_list(env, 0);
        while (riter != rend) {
            auto elem = erl::to_erl(env, *riter++);
            list = ::enif_make_list_cell(env, elem, list);
        }
        return list;
    }
};

template <typename... Ts> struct convert<std::tuple<Ts...>> {
    static ERL_NIF_TERM to_erl(::ErlNifEnv* env, const std::tuple<Ts...>& tup) {
        return to_erl_helper(env, tup, std::make_index_sequence<sizeof...(Ts)>{});
    }

    template <std::size_t... Is>
    static ERL_NIF_TERM
    to_erl_helper(::ErlNifEnv* env, const std::tuple<Ts...>& tup, std::index_sequence<Is...>) {
        return ::enif_make_tuple(env, sizeof...(Ts), erl::to_erl(env, std::get<Is>(tup))...);
    }
};

template <> struct convert<const char*> {
    static ERL_NIF_TERM to_erl(::ErlNifEnv* env, const char* str) {
        return erl::to_erl(env, std::string(str));
    }
};

template <> struct convert<std::string> {
    static ERL_NIF_TERM to_erl(::ErlNifEnv* env, const std::string& str) {
        ERL_NIF_TERM ret;
        auto ptr = ::enif_make_new_binary(env, str.size(), &ret);
        if (!ptr)
            throw std::bad_alloc{};
        std::copy(str.begin(), str.end(), ptr);
        return ret;
    }
};


}  // namespace erl

#endif  // ERLBIND_CONVERT_HPP_INCLUDED