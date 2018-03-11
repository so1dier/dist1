#include "stdafx.h"
#include "socket_acceptor.h"
#include <functional>
#include <boost/bind.hpp>

using namespace boost::asio::ip;

void ds::socket_acceptor::accept(const boost::system::error_code& ec, tcp::socket&& socket)
{
	auto socket_ptr = std::make_shared<socket_t>(std::move(socket));
	if (!on_accept(socket_ptr))
		socket_ptr->close();
	else
		mConnections.for_write().r().push_back(socket_ptr);

	start_accept();
}

void ds::socket_acceptor::clean_up()
{
	auto lock = mConnections.for_write();
	for (auto it = lock.r().begin(); it != lock.r().end();)
        {
		auto strong_socket = (*it).lock();
		if (!strong_socket)
			it = lock.r().erase(it);
		else
			++it;
        }
	mAcceptor.cancel();
}

void ds::socket_acceptor::close()
{
	for (auto& socket : mConnections.for_write().r())
		if (auto strong_socket = socket.lock())
			strong_socket->cancel();

	mAcceptor.cancel();
	clean_up(); // erase all (or at least the ones we just closed)
}

ds::socket_acceptor::ptr ds::socket_acceptor::start_accept()
{
//	tcp::socket new_connection(*_io_service);
//	mAcceptor.async_accept(boost::bind(&socket_acceptor::accept, this, boost::ref(new_connection), boost::asio::placeholders::error));
	return shared_from_this();
}
