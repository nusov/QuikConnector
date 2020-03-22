#include "QuikStackUtils.h"

int quik_stack_table_count(lua_State *L, int t) {
	lua_pushnil(L);
	int count = 0;
	while (lua_next(L, t) != 0) {
		lua_pop(L, 1);
		count++;
	}
	return count;
}

void quik_stack_push(lua_State *L, msgpack::object& obj) {
	switch (obj.type) {
	case msgpack::type::STR:
		lua_pushlstring(L, obj.via.str.ptr, obj.via.str.size);
		break;
	case msgpack::type::BOOLEAN:
		lua_pushboolean(L, obj.via.boolean);
		break;
	case msgpack::type::NIL:
		lua_pushnil(L);
		break;
	case msgpack::type::POSITIVE_INTEGER:
		lua_pushnumber(L, (lua_Number)obj.via.u64);
		break;
	case msgpack::type::NEGATIVE_INTEGER:
		lua_pushnumber(L, (lua_Number)obj.via.i64);
		break;
	case msgpack::type::FLOAT32:
		lua_pushnumber(L, (lua_Number)obj.via.f64);
		break;
	case msgpack::type::FLOAT64:
		lua_pushnumber(L, (lua_Number)obj.via.f64);
		break;
	case msgpack::type::ARRAY:
		lua_newtable(L);
		for (uint32_t i = 0; i < obj.via.array.size; i++) {
			lua_pushinteger(L, i + 1);
			quik_stack_push(L, obj.via.array.ptr[i]);
			lua_settable(L, -3);
		}
		break;
	case msgpack::type::MAP:
		lua_newtable(L);
		for (uint32_t i = 0; i < obj.via.map.size; i++) {
			quik_stack_push(L, obj.via.map.ptr[i].key);
			quik_stack_push(L, obj.via.map.ptr[i].val);
			lua_settable(L, -3);
		}
	default:
		break;
	}
}

void quik_stack_pack(msgpack::packer<msgpack::sbuffer> &pk, lua_State *L, int i) {
	switch (lua_type(L, i)) {
	case LUA_TNIL:
		pk.pack_nil();
		break;
	case LUA_TBOOLEAN:
		if (lua_toboolean(L, i)) {
			pk.pack_true();
		}
		else {
			pk.pack_false();
		}
		break;
	case LUA_TNUMBER:
		pk.pack(lua_tonumber(L, i));
		break;
	case LUA_TSTRING:
		pk.pack(lua_tostring(L, i));
		break;
	case LUA_TTABLE:
		pk.pack_map(quik_stack_table_count(L, i));
		lua_pushnil(L);
		while (lua_next(L, i)) {
			quik_stack_pack(pk, L, -2);
			quik_stack_pack(pk, L, lua_gettop(L));
			lua_pop(L, 1);
		}
		break;
	default:
		pk.pack_nil();
		break;
	}
}

