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
#include <unordered_set>
using namespace std::string_literals;
using namespace std::chrono_literals;

namespace ds
{
	using namespace boost::asio::ip;
	typedef std::shared_ptr<boost::asio::io_service> io_service_ptr;

	class socket_acceptor : protected std::enable_shared_from_this<socket_acceptor>
	{
	protected:
		typedef std::shared_ptr<socket_acceptor> ptr;

		struct socket_t : tcp::socket, std::enable_shared_from_this<socket_t>
		{
			template<typename... Args> socket_t(ptr acceptor, Args&&... args) 
				: tcp::socket(std::forward<Args>(args)...), _acceptor(acceptor) {}

			~socket_t()
			{
				_acceptor->mConnections.GetResourceForWrite().mResource.erase(shared_from_this());
			}

			ptr _acceptor;
		};

		typedef std::shared_ptr<socket_t> socket_ptr;

		tcp::acceptor mAcceptor;
		ThreadSafeResource<std::unordered_set<socket_ptr>> mConnections;
		io_service_ptr _io_service;
		
		// virtual function hook for handling accept. return true to add connection to connection pool
		virtual bool on_accept(const socket_ptr& ptr) = 0;

	private:
		void accept_next()
		{
			mAcceptor.async_accept(std::bind(&socket_acceptor::accept, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}

		void accept(const boost::system::error_code& ec, tcp::socket&& socket)
		{
			auto socket_ptr = std::make_shared<socket_t>(shared_from_this(), std::move(socket));
			if (!on_accept(socket_ptr))
				socket_ptr->close();
			else
				mConnections.GetResourceForWrite().mResource.insert(socket_ptr);

			accept_next();
		}

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

			auto get_buffer()
			{
				return boost::asio::buffer(reinterpret_cast<char(&)[sizeof(_data)]>(_data));
			}
		};

	protected:
		virtual bool on_accept(const socket_ptr& socket) override
		{
			// This one is responsible for accepting requests from outside the system (the API)
			receive_header(socket);
			
			return true;
		}

		virtual void receive_header(socket_ptr socket)
		{	
			auto header = std::make_shared<header_t>();

			boost::asio::async_read(*socket, header->get_buffer(),
				[=, ensure_lifetime = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
			{
				if (bytes_transferred == 0 || ec != boost::system::errc::success)
					return; // Remote socket down. Please close local socket.
				
				receive_body(socket, header);
			});

		}

		void receive_body(socket_ptr socket, header_t::ptr header)
		{
			// receive and process body
			if (header->_data.version == 1)
			{
				constexpr auto max_message_length = 10 * 1'024 * 1'024;
				if (header->_data.message_length > max_message_length)
					return;
				std::shared_ptr<char[]> buffer(new char[header->_data.message_length]);
				boost::asio::async_read(*socket, header->get_buffer(),
					[=, ensure_lifetime = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
				{
					if (bytes_transferred == 0 || ec != boost::system::errc::success)
						return; // Remote socket down. Please close local socket.

					receive_body(socket, header);
				});
			}
			// continue on -> receive next message
			receive_header(socket);
		}
	};

	struct socket_acceptor_requests_with_ascii_header : socket_acceptor_requests
	{
		socket_acceptor_requests_with_ascii_header(const io_service_ptr& io_service, uint32_t port, const tcp& t = tcp::v4())
			: socket_acceptor_requests(io_service, port, t) {}

		virtual void receive_header(socket_ptr socket)
		{
			// Very simple, just receive the message length as ascii, and convert to binary header
			#pragma pack(1)
			struct ascii_header_t
			{
				struct data_t
				{
					char version[10] = {};
					char message_length[10] = {};
				} _data;

				auto get_buffer()
				{
					return boost::asio::buffer(reinterpret_cast<char(&)[sizeof(data_t)]>(_data));
				}
			};

			auto ascii_header = std::make_shared<ascii_header_t>();

			boost::asio::async_read(*socket, boost::asio::buffer(ascii_header->get_buffer()),
				[=, ensure_lifetime = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
			{
				if (bytes_transferred == 0 || ec != boost::system::errc::success)
					return; // Remote socket down. Please close local socket.

				auto header = std::make_shared<header_t>();
				header->_data.version = atoi(ascii_header->_data.version);
				header->_data.message_length = atoi(ascii_header->_data.message_length);
				receive_body(socket, header);
			});
		}
	};
}

int main()
{
	using namespace boost::asio::ip;
	const auto io_service = std::make_shared<boost::asio::io_service>();

	ds::socket_acceptor_nodes acceptor_for_nodes(io_service, 1234);
	ds::socket_acceptor_requests acceptor_for_requests(io_service, 1235);
	ds::socket_acceptor_requests_with_ascii_header acceptor_for_requests_ascii(io_service, 1236);
	io_service->run();

	return 0;
}

