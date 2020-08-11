// QuikRouter.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "QuikStackUtils.h"


class QuikRouterSocket {
public:
	QuikRouterSocket(const std::string& bind_address) {
		this->zmq_ctx = new zmq::context_t(1);
		this->zmq_skt = new zmq::socket_t(*this->zmq_ctx, ZMQ_ROUTER);
		this->zmq_skt->setsockopt(ZMQ_ROUTER_HANDOVER, 1);
		this->zmq_skt->bind(bind_address);
	}

	~QuikRouterSocket() {
		if (this->zmq_skt) {
			this->zmq_skt->close();
			delete this->zmq_skt;
		}
		if (this->zmq_ctx) {
			this->zmq_ctx->close();
			delete this->zmq_ctx;
		}
	}

	int send(lua_State *L) {
		std::string identity = luaL_checkstring(L, 2);
		luaL_checkany(L, 3);

		if (this->zmq_skt) {
			try {
				zmq::message_t msg;
				
				msgpack::sbuffer packed;
				msgpack::packer<msgpack::sbuffer> pk(packed);

				quik_stack_pack(pk, L, 3);
				send_response(identity, packed);
			}
			catch (zmq::error_t ex) {
				// TODO: handle transport exceptions
			}
		}
		return 1;
	}
protected:
	void send_response(const std::string& identity, const msgpack::sbuffer& buffer) {
		if (this->zmq_skt) {
			try {
				zmq::message_t response1(identity.size());
				std::memcpy(response1.data(), identity.data(), identity.size());
				this->zmq_skt->send(response1, zmq::send_flags::sndmore);

				zmq::message_t response2(buffer.size());
				std::memcpy(response2.data(), buffer.data(), buffer.size());
				this->zmq_skt->send(response2, zmq::send_flags::none);
			}
			catch (zmq::error_t ex) {
				// TODO: handle transport exceptions
			}
		}
	}
private:
	zmq::socket_t* zmq_skt;
	zmq::context_t* zmq_ctx;
};

static QuikRouterSocket * quik_router_socket_check(lua_State *L, int n) {
	return *(QuikRouterSocket **)luaL_checkudata(L, n, "luaL_QuikRouterSocket");
}

static int quik_router_socket_bind(lua_State *L) {
	std::string bind_address = luaL_checkstring(L, 1);
	QuikRouterSocket** udata = (QuikRouterSocket**)lua_newuserdata(L, sizeof(QuikRouterSocket *));
	*udata = new QuikRouterSocket(bind_address);
	luaL_getmetatable(L, "luaL_QuikRouterSocket");
	lua_setmetatable(L, -2);
	return 1;
}

static int quik_router_socket_send(lua_State *L) {
	QuikRouterSocket *s = quik_router_socket_check(L, 1);
	return s->send(L);
}

static int quik_router_socket_destructor(lua_State *L) {
	QuikRouterSocket *s = quik_router_socket_check(L, 1);
	delete s;
	return 0;
}

static luaL_Reg quik_router_socket_funcs[] =
{
	{ "bind", quik_router_socket_bind },
	{ "send", quik_router_socket_send },
	{ "__gc", quik_router_socket_destructor },
	{ NULL, NULL }
};


static luaL_Reg ls_lib[] = {
	{ NULL, NULL }
};

extern "C" LUALIB_API int luaopen_QuikRouter(lua_State *L) {
	luaL_openlib(L, "QuikRouter", ls_lib, 0);
	luaL_newmetatable(L, "luaL_QuikRouterSocket");
	luaL_register(L, NULL, quik_router_socket_funcs);
	lua_pushvalue(L, -1);
	lua_setfield(L, -1, "__index");
	lua_setglobal(L, "QuikRouterSocket");
	return 1;
}