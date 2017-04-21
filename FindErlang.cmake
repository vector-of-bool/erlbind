find_path(
    ERL_NIF_INCLUDE_DIR erl_nif.h
    PATHS
        "C:/Program Files/erl8.0/usr/include"
        "C:/Program Files (x86)/erl8.0/usr/include"
    )

if(NOT ERL_NIF_INCLUDE_DIR)
    message(SEND_ERROR "Unable to find Erlang include directory. Is Erlang installed?")
else()
    message(STATUS "Found Erlang include path: ${ERL_NIF_INCLUDE_DIR}")
endif()

if(NOT TARGET erl_nif)
    add_library(erl_nif INTERFACE IMPORTED)
    set_target_properties(erl_nif PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${ERL_NIF_INCLUDE_DIR}"
        )
endif()