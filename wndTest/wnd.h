#pragma once

#ifdef _WIN32
#include "FramelessWnd.h"
#else
#include "widget.h"
#endif

#ifdef __APPLE__
#include "OSXHideTitleBar.h"
#endif