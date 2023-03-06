// cope.h : Include file for standard system include files,
// or project specific include files.

#pragma once

// TODO: Reference additional headers your program requires here.

#include "dp.h"

class MainWindow_t;

namespace cope {
  HRESULT Intialize(SIZE size);
  dp::result_code SellItem(const MainWindow_t& main_window);
}
