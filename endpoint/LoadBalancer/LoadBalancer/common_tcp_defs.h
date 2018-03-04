#pragma once
#include <memory>
// todo: investigate forward declares
#include <boost/asio.hpp>

namespace ds
{
	using namespace boost::asio::ip;
	typedef std::shared_ptr<boost::asio::io_service> io_service_ptr;
	typedef tcp::socket socket_t;
	typedef std::shared_ptr<socket_t> socket_ptr;
	typedef std::weak_ptr<socket_t> socket_weak_ptr;
}
