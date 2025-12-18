#include "doctest.h"
#include "interlaced_core/filesystem.hpp"
#include "interlaced_core/json.hpp"
#include "interlaced_core/logging.hpp"
#include "interlaced_core/network.hpp"

TEST_CASE("sanity: headers compile and basics work") {
    CHECK(1 + 1 == 2);
}
