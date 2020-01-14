/*
* Modern effects for a modern Streamer
* Copyright (C) 2017 Michael Fabian Dirks
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#include "util-memory.hpp"
#include <cstdlib>
#include <stdexcept>
#include <stdlib.h>

#if defined(_MSC_VER) && (_MSC_VER <= 2100)
//#define USE_MSC_ALLOC
#elif defined(_cplusplus) && (__cplusplus >= 201103L)
#define USE_STD_ALLOC
#endif

using namespace std;
