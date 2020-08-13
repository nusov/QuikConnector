// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "StackUtils.h"
#include "BaseSocket.h"

// Windows configuration
#define WIN32_LEAN_AND_MEAN

// LUA configuration
#define LUA_LIB
#define LUA_BUILD_AS_DLL
#define LUA_COMPAT_5_1

// Windows Header Files:
#include <windows.h>

// Headers
#include <iostream>
#include <exception>

// Third party dependencies
#include <zmq.hpp>
#include <msgpack.hpp>

// Lua
extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}