#ifndef ERLBIND_CONVERT_HPP_INCLUDED
#define ERLBIND_CONVERT_HPP_INCLUDED

#include <erl_nif.h>

namespace erl {

template <typename T> struct convert;

template <> struct convert<ERL_NIF_TERM> {
    static ERL_NIF_TERM to_erl(::ErlNifEnv* env, ERL_NIF_TERM t) {
        return t;
    }
    static ERL_NIF_TERM from_erl(::ErlNifEnv* env, ERL_NIF_TERM t) {
        return t;
    }
};

template <> struct convert<int> {
    static ERL_NIF_TERM to_erl(::ErlNifEnv* env, int value) {
        return ::enif_make_int(env, value);
    }
    // static int from_erl(::ErlNifEnv* env, ERL_NIF_TERM t) {
    //     return t;
    // }
};

}  // namespace erl

#endif  // ERLBIND_CONVERT_HPP_INCLUDED