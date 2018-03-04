#pragma once
// From: https://gist.github.com/ricejasonf/c9335d86b169bcca9d73f57ea1b97a37 
#include <amqpcpp.h>
#include "common_tcp_defs.h"
#include <string>

namespace ds
{
	namespace rabbitmq
	{
		namespace detail
		{
			class connection_handler : public AMQP::ConnectionHandler
			{
				virtual void onData(AMQP::Connection *connection, const char *data, size_t size) override;
				virtual void onConnected(AMQP::Connection *connection) override;
				virtual void onError(AMQP::Connection *connection, const char *message) override;
				virtual void onClosed(AMQP::Connection *connection) override;
			public:
				socket_ptr _socket;
				io_service_ptr _io_service;

				connection_handler(const io_service_ptr& io_service, std::string const& host, std::string const& port);
			};

			class connection_impl : public std::enable_shared_from_this<connection_impl>
			{
				socket_t &socket;
				char buffer[1024];

			public:
				// just expose the impl layer
				AMQP::Connection amqp_impl;
				connection_impl(connection_handler* handler);
				// prevent copy because we are holding a reference
				connection_impl(connection_impl const&) = delete;
				void keep_reading();
			};
		}

		class client
		{
			detail::connection_handler handler;
			std::shared_ptr<detail::connection_impl> impl;

		public:
			client(const io_service_ptr& io_service, std::string const& host, std::string const& port);
			client(client const&) = delete;

			AMQP::Connection& amqp()
			{
				return impl->amqp_impl;
			}
		};
	}
}
