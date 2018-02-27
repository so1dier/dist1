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
using namespace std::string_literals;
using namespace std::chrono_literals;

struct node_endpoint
{

};

namespace ds
{
	using namespace boost::asio::ip;
	typedef std::shared_ptr<tcp::socket> socket_ptr;

	class socket_acceptor
	{
		tcp::acceptor mAcceptor;
		std::mutex mMutex;
		ThreadSafeResource<std::vector<socket_ptr>> mConnections;
		
		void accept_next()
		{
			mAcceptor.async_accept(std::bind(&socket_acceptor::accept, this, std::placeholders::_1, std::placeholders::_2));
		}

		void accept(const boost::system::error_code& ec, tcp::socket&& socket)
		{
			auto socket_ptr = std::make_shared<tcp::socket>(std::move(socket));
			mConnections.GetResourceForWrite().mResource.push_back(socket_ptr);
			auto buffer = std::make_shared<std::array<char, 4096>>();
			
			
			accept_next();
		}

	public:
		socket_acceptor(boost::asio::io_service& io_service, uint32_t port, const tcp& t = tcp::v4())
			: mAcceptor(io_service, tcp::endpoint(t, port)) 
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
}

int main()
{
	using namespace boost::asio::ip;
	boost::asio::io_service io_service;

	ds::socket_acceptor acceptor(io_service, 1234);
	io_service.run();

    return 0;
}

