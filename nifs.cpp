#include "erlbind/nif.hpp"

ERLBIND_MODULE(Elixir.Erlbind.Test) {
}

using int_pair = std::tuple<int, erl::atom>;
ERLBIND_FUNCTION_PLAIN(test_fn, int_pair, int second, erl::atom something) {
    return int_pair{1, something};
}