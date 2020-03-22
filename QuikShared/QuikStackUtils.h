#pragma once

// Third party dependencies
#include <msgpack.hpp>

// Lua
extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

void quik_stack_push(lua_State *L, msgpack::object& obj);
void quik_stack_pack(msgpack::packer<msgpack::sbuffer> &pk, lua_State *L, int i);
