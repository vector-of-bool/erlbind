#ifndef ERLBIND_NIF_HPP_INCLUDED
#define ERLBIND_NIF_HPP_INCLUDED

#include "common.hpp"

#include <erl_nif.h>

#include <string>
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
};
}

// ERL_NIF_INIT

#define ERLBIND_MODULE(modname)                                                                                        \
    namespace erlbind_impls {                                                                                          \
    static inline void erlbind_module_init_fn(::erl::module_definition&);                                              \
    }                                                                                                                  \
    ERL_NIF_INIT_GLOB                                                                                                  \
    extern "C" ERLBIND_EXPORT_API::ErlNifEntry* nif_init(ERL_NIF_INIT_ARGS) {                                          \
        static ::erl::module_definition mod{ #modname };                                                               \
        erlbind_impls::erlbind_module_init_fn(mod);                                                                    \
        ERL_NIF_INIT_BODY;                                                                                             \
        return mod.nif_entry();                                                                                        \
    }                                                                                                                  \
    void erlbind_impls::erlbind_module_init_fn(::erl::module_definition& module)

#endif  // ERLBIND_NIF_HPP_INCLUDED