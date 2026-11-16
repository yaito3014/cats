#include <yk/cats/public.hpp>

#if YK_CATS_CATCH2_MAJOR_VERSION < 3
#  include <catch2/catch.hpp>
#else
#  include <catch2/catch_test_macros.hpp>
#endif

TEST_CASE("main")
{
  CHECK(yk::cats::func(12) == 12);
  CHECK(yk::cats::func(34) == 34);
}
