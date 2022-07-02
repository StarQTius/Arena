#pragma once

// To avoid clashing with LTL
#undef FWD
#define FWD(X) static_cast<decltype(X) &&>(X)
