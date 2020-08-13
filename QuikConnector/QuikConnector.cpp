// QuikConnector.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

class QuikSocket: public BaseSocket {
public:
	static inline const std::string RUNTIME_ERROR = "RUNTIME_ERROR";
	static inline const std::string OK = "OK";
	static inline const std::string NOT_FOUND = "NOT_FOUND";

	QuikSocket(const std::string& bind_address) {
		this->zmq_ctx = new zmq::context_t(1);
		this->zmq_skt = new zmq::socket_t(*this->zmq_ctx, ZMQ_REP);
		this->zmq_skt->bind(bind_address);
	}

	int process(lua_State *L, zmq::recv_flags flags) {
		if (this->zmq_skt) {
			try {
				zmq::message_t msg;
				if (zmq_skt->recv(msg, flags)) {
					auto handle = msgpack::unpack(static_cast<const char*>(msg.data()), msg.size());

					msgpack::sbuffer packed;
					msgpack::packer<msgpack::sbuffer> pk(packed);

					std::string funcname;
					handle.get().via.array.ptr[0].convert(funcname);

					int level = lua_gettop(L);
					lua_getglobal(L, funcname.c_str());
					if (!lua_isnil(L, -1)) {
						for (uint32_t i = 1; i < handle.get().via.array.size; i++) {
							quik_stack_push(L, handle.get().via.array.ptr[i]);
						}

						int top_prev = lua_gettop(L);
						if (lua_pcall(L, handle.get().via.array.size - 1, LUA_MULTRET, 0) != 0) {
							pk.pack(lua_tostring(L, -1));
							lua_pop(L, -1);
							send_response(RUNTIME_ERROR, packed);
						}
						else {
							int results = lua_gettop(L) - level;

							pk.pack_array(results);

							for (int i = results; i > 0; i--) {
								quik_stack_pack(pk, L, lua_gettop(L) - i + 1);
							}

							// cleanup stack
							for (int i = 0; i < results; i++) {
								lua_pop(L, 1);
							}

							send_response(OK, packed);
						}
					}
					else {
						lua_pop(L, -1);
						pk.pack(funcname);
						send_response(NOT_FOUND, packed);
					}
				}
			}
			catch (zmq::error_t ex) {
				// TODO: handle transport exceptions
			}
		}
		return 1;
	}
};

static QuikSocket * quik_check(lua_State *L, int n) {
	return *(QuikSocket **)luaL_checkudata(L, n, "luaL_QuikSocket");
}

static int quik_bind(lua_State *L) {
	std::string bind_address = luaL_checkstring(L, 1);
	QuikSocket** udata = (QuikSocket**)lua_newuserdata(L, sizeof(QuikSocket *));
	*udata = new QuikSocket(bind_address);
	luaL_getmetatable(L, "luaL_QuikSocket");
	lua_setmetatable(L, -2);
	return 1;
}

static int quik_process(lua_State *L) {
	QuikSocket *s = quik_check(L, 1);
	return s->process(L, zmq::recv_flags::none);
}

static int quik_process_noblock(lua_State *L) {
	QuikSocket *s = quik_check(L, 1);
	return s->process(L, zmq::recv_flags::dontwait);
}

static int quik_destructor(lua_State *L) {
	QuikSocket *s = quik_check(L, 1);
	delete s;
	return 0;
}

static luaL_Reg quik_funcs[] =
{
	{ "bind", quik_bind },
	{ "process", quik_process },
	{ "process_noblock", quik_process_noblock },
	{ "__gc", quik_destructor },
	{ NULL, NULL }
};


static luaL_Reg ls_lib[] = {
	{ NULL, NULL }
};

extern "C" LUALIB_API int luaopen_QuikConnector(lua_State *L) {
	luaL_openlib(L, "QuikConnector", ls_lib, 0);
	luaL_newmetatable(L, "luaL_QuikSocket");
	luaL_register(L, NULL, quik_funcs);
	lua_pushvalue(L, -1);
	lua_setfield(L, -1, "__index");
	lua_setglobal(L, "QuikSocket");
	return 1;
}