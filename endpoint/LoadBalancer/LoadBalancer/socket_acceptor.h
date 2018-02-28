#pragma once
#include "ThreadSafeResource.h"
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
	typedef std::shared_ptr<boost::asio::io_service> io_service_ptr;

	class socket_acceptor : public std::enable_shared_from_this<socket_acceptor>
	{
		typedef std::shared_ptr<socket_acceptor> ptr;
		typedef tcp::socket socket_t;
	protected:
		typedef std::shared_ptr<socket_acceptor> ptr;
		typedef std::shared_ptr<socket_t> socket_ptr;
		typedef std::weak_ptr<socket_t> socket_weak_ptr;

		tcp::acceptor mAcceptor;
		// Sockets call close on their own destruction. We keep weak pointers just in case the programmer wants to force close early. 
		ThreadSafeResource<std::list<socket_weak_ptr>> mConnections;
		io_service_ptr _io_service;

		// virtual function hook for handling accept. return true to add connection to connection pool
		virtual bool on_accept(const socket_ptr& ptr) = 0;

	private:
		void accept(const boost::system::error_code& ec, tcp::socket&& socket);

	public:
		socket_acceptor(const io_service_ptr& io_service, uint32_t port, const tcp& t = tcp::v4())
			: mAcceptor(*io_service, tcp::endpoint(t, port)), _io_service(io_service) {}

		void clean_up();
		void close();
		ptr start_accept();
	};
}
