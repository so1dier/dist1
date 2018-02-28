#include "stdafx.h"
#include "socket_acceptor_requests.h"

bool ds::socket_acceptor_requests::on_accept(const socket_ptr& socket)
{
	// This one is responsible for accepting requests from outside the system (the API)
	socket_acceptor_auto_clean_up::on_accept(socket);
	receive_header(socket);

	return true;
}

void ds::socket_acceptor_requests::receive_header(socket_ptr socket) const
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

void ds::socket_acceptor_requests::receive_body(socket_ptr socket, header_t::ptr header) const
{
	// receive and process body
	if (header->_data.version != 1)
		return; // only 1 supported

	constexpr auto max_message_length = 10 * 1'024 * 1'024;
	if (header->_data.message_length > max_message_length)
		return;

	std::shared_ptr<char[]> body(new char[header->_data.message_length]);

	boost::asio::async_read(*socket, boost::asio::buffer(body.get(), header->_data.message_length),
		[=, ensure_lifetime = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (bytes_transferred == 0 || ec != boost::system::errc::success)
			return; // Remote socket down. Please close local socket.

		
		// continue on -> receive next message
		receive_header(socket);
	});
}


