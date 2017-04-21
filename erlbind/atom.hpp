#ifndef ERLBIND_ATOM_HPP_INCLUDED
#define ERLBIND_ATOM_HPP_INCLUDED

#include "convert.hpp"

#include <cassert>

namespace erl {

class atom {
    ::ERL_NIF_TERM _term;

public:
    atom(::ErlNifEnv* env, const std::string& spelling) {
        if (spelling.size() > 255) {
            throw std::bad_alloc{};
        }
        auto did_create = ::enif_make_existing_atom_len(env,
                                                        spelling.data(),
                                                        spelling.size(),
                                                        &_term,
                                                        ERL_NIF_LATIN1);
        if (!did_create) {
            ::enif_make_atom_len(env, spelling.data(), spelling.size());
        }
    }

    explicit atom(::ErlNifEnv* env, ::ERL_NIF_TERM term)
        : _term{term} {
        assert(::enif_is_atom(env, term));
    }

    ::ERL_NIF_TERM term() const {
        return _term;
    }
};

template <> struct convert<atom> {
    static ::ERL_NIF_TERM to_erl(::ErlNifEnv*, atom a) {
        return a.term();
    }

    static atom from_erl(::ErlNifEnv* env, ERL_NIF_TERM term) {
        return atom{env, term};
    }
};

}  // namespace erl

#endif  // ERLBIND_ATOM_HPP_INCLUDED