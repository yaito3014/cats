#include <yk/cats/public.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("main")
{
  CHECK(yk::cats::func(12) == 12);
  CHECK(yk::cats::func(34) == 34);
}
