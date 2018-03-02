#include "stdafx.h"
#include "socket_acceptor.h"

using namespace boost::asio::ip;

void ds::socket_acceptor::accept(const boost::system::error_code& ec, tcp::socket&& socket)
{
	auto socket_ptr = std::make_shared<socket_t>(std::move(socket));
	if (!on_accept(socket_ptr))
		socket_ptr->close();
	else
		mConnections.GetResourceForWrite().r().push_back(socket_ptr);

	start_accept();
}

void ds::socket_acceptor::clean_up()
{
	auto lock = mConnections.GetResourceForWrite();
	for (auto it = lock.r().begin(); it != lock.r().end();)
		if (auto strong_socket = (*it).lock(); !strong_socket)
			it = lock.r().erase(it);
		else
			++it;

	mAcceptor.cancel();
}

void ds::socket_acceptor::close()
{
	for (auto& socket : mConnections.GetResourceForWrite().r())
		if (auto strong_socket = socket.lock())
			strong_socket->cancel();

	mAcceptor.cancel();
	clean_up(); // erase all (or at least the ones we just closed)
}

ds::socket_acceptor::ptr ds::socket_acceptor::start_accept()
{
	mAcceptor.async_accept(std::bind(&socket_acceptor::accept, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	return shared_from_this();
}
