#include <private.hpp>

#include <yk/cats/public.hpp>

namespace yk {
namespace cats {

int func_internal(int x) { return x; }

int func(int x) { return func_internal(x); }

}  // namespace cats
}  // namespace yk
