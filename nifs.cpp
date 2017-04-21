#include "erlbind/nif.hpp"

ERLBIND_MODULE(Elixir.Erlbind.Test) {}

ERLBIND_FUNCTION_PLAIN(test_fn, int)
{
    return 12;
}