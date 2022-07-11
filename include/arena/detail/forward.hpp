#pragma once

#define ARENA_FWD(X) static_cast<decltype(X) &&>(X)
