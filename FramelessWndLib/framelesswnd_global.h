#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(FRAMELESSWND_LIB)
#  define FRAMELESSWND_EXPORT Q_DECL_EXPORT
# else
#  define FRAMELESSWND_EXPORT Q_DECL_IMPORT
# endif
#else
# define FRAMELESSWND_EXPORT
#endif
