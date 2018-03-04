#pragma once
#include "socket_acceptor_auto_clean_up.h"
#include "amqp_client.h"

namespace ds
{
	struct socket_acceptor_requests : socket_acceptor_auto_clean_up
	{
		socket_acceptor_requests(io_service_ptr io_service, int port, const std::string& rabbit_host, const std::string& rabbit_port);
		// very basic TCP header. Just a version and a message length
#pragma pack(1)
		struct header_t
		{
			typedef std::shared_ptr<header_t> ptr;
			struct
			{
				uint32_t version = {};
				uint32_t message_length = {};
			} _data;
		};

	protected:
		virtual bool on_accept(const socket_ptr& socket) override;
		virtual void receive_header(socket_ptr socket);
		void receive_body(socket_ptr socket, header_t::ptr header);

		ds::rabbitmq::client _rabbit_client;
		AMQP::Channel _publisher;
	};
}
