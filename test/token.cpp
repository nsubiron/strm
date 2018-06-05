#include <strm/detail/token.h>

static_assert(
    sizeof(strm::token_type) == 24u,
    "Size shouldn't be more than"
    "  v6 address  : 128"
    "  + port      :  16"
    "  + state     :  16"
    "  + stream id :  32"
    "  -----------------"
    "                192");
