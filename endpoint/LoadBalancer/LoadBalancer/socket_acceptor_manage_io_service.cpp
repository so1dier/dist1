#include "stdafx.h"
#include "socket_acceptor_manage_io_service.h"

void ds::socket_acceptor_manage_io_service::receive(const socket_ptr& socket) const
{
	typedef uint32_t header_t;
	auto message_length = std::make_shared<header_t>();

	boost::asio::async_read(*socket, mdk::to_asio_buffer(*message_length),
		[=, ensure_lifetime = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		constexpr auto max_message_length = 1'000'000;
		if (bytes_transferred == 0 || ec != boost::system::errc::success && *message_length < max_message_length)
			return; // Remote socket down. Please close local socket.

		std::shared_ptr<char[]> body(new char[*message_length]);
		boost::asio::async_read(*socket, boost::asio::buffer(body.get(), *message_length),
			[=, ensure_lifetime = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
		{
			if (bytes_transferred == 0 || ec != boost::system::errc::success)
				return; // Remote socket down. Please close local socket.

			std::string command(body.get(), *message_length);
			if (command == "STOP")
			{
				_io_service->stop();
				return;
			}
			else
				return; // placeholder
			receive(socket); // placeholder
		});
	});
}

bool ds::socket_acceptor_manage_io_service::on_accept(const socket_ptr& socket)
{
	receive(socket);
	return true;
}
