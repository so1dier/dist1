#include "stdafx.h"
#include <boost/asio.hpp>
#include <thread>
#include <string>
#include <array>
#include <memory>
#include <vector>
#include <functional>
#include <chrono>
#include <iostream>
#include <mutex>
#include <iterator>
#include <exception>
using namespace std::string_literals;
using namespace std::chrono_literals;

namespace ds
{
	using namespace boost::asio::ip;
	typedef std::shared_ptr<tcp::socket> socket_ptr;
	typedef std::shared_ptr<boost::asio::io_service> io_service_ptr;

	class socket_acceptor
	{
		tcp::acceptor mAcceptor;
		ThreadSafeResource<std::vector<socket_ptr>> mConnections;

		void accept_next()
		{
			mAcceptor.async_accept(std::bind(&socket_acceptor::accept, this, std::placeholders::_1, std::placeholders::_2));
		}

		void accept(const boost::system::error_code& ec, tcp::socket&& socket)
		{
			auto socket_ptr = std::make_shared<tcp::socket>(std::move(socket));
			if (!on_accept(socket_ptr))
				socket_ptr->close();
			else
				mConnections.GetResourceForWrite().mResource.push_back(socket_ptr);

			accept_next();
		}

	protected:
		io_service_ptr _io_service;
		
		// virtual function hook for handling accept. return true to add connection to connection pool
		virtual bool on_accept(const socket_ptr& ptr) = 0;

	public:
		socket_acceptor(const io_service_ptr& io_service, uint32_t port, const tcp& t = tcp::v4())
			: mAcceptor(*io_service, tcp::endpoint(t, port)), _io_service(io_service)
		{
			accept_next();
		}

		void close()
		{
			for (auto& socket : mConnections.GetResourceForWrite().mResource)
				socket->cancel();

			mAcceptor.cancel();
		}
	};

	struct socket_acceptor_nodes : socket_acceptor
	{
		socket_acceptor_nodes(const io_service_ptr& io_service, uint32_t port, const tcp& t = tcp::v4())
			: socket_acceptor(io_service, port, t) {}

		virtual bool on_accept(const socket_ptr& ptr) override
		{
			// This one will talk to the rasberry pi nodes... basically forwarding requests onto them
			throw std::exception("not implemented");
		}
	};

	struct socket_acceptor_requests : socket_acceptor
	{
		socket_acceptor_requests(const io_service_ptr& io_service, uint32_t port, const tcp& t = tcp::v4())
			: socket_acceptor(io_service, port, t) {}

		// very basic TCP header for now. Just a version and a message length
		#pragma pack(1)
		struct header_t
		{
			typedef std::shared_ptr<header_t> ptr;
			struct data_t
			{
				uint32_t version = {};
				uint32_t message_length = {};
			} _data;

			// The header_buffer_t satisfies boost's ConstBufferSequence. 
			typedef std::array<char, sizeof(data_t)> header_buffer_t;
			operator header_buffer_t&() 
			{
				return reinterpret_cast<header_buffer_t&>(_data);
			}
		};

	protected:
		virtual bool on_accept(const socket_ptr& socket) override
		{
			// This one is responsible for accepting requests from outside the system (the API)
			receive_header(socket);
			
			return true;
		}

		void receive_header(socket_ptr socket)
		{
			auto header = std::make_shared<header_t>();

			boost::asio::async_read(*socket, boost::asio::buffer(header->operator ds::socket_acceptor_requests::header_t::header_buffer_t &()), 
				[=](const boost::system::error_code& ec, size_t bytes_transferred)
			{
				if (bytes_transferred == 0 || ec != boost::system::errc::success)
					return; // Remote socket down. Please close local socket.
				
				receive_body(socket, header);
			});

		}

		void receive_body(socket_ptr socket, header_t::ptr header)
		{
			// process body

			// continue on -> receive next message
			receive_header(socket);
		}
	};
}

int main()
{
	using namespace boost::asio::ip;
	const auto io_service = std::make_shared<boost::asio::io_service>();

	ds::socket_acceptor_nodes acceptor_for_nodes(io_service, 1234);
	ds::socket_acceptor_requests acceptor_for_requests(io_service, 1235);
	io_service->run();

	return 0;
}

