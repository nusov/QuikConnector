#pragma once

#include <zmq.hpp>
#include <msgpack.hpp>

class BaseSocket
{
public:
	virtual ~BaseSocket() {
		if (this->zmq_skt) {
			this->zmq_skt->close();
			delete this->zmq_skt;
		}
		if (this->zmq_ctx) {
			this->zmq_ctx->close();
			delete this->zmq_ctx;
		}
	}
protected:
	zmq::socket_t* zmq_skt;
	zmq::context_t* zmq_ctx;

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
};

