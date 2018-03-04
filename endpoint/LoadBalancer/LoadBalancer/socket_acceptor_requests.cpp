#include "stdafx.h"
#include "socket_acceptor_requests.h"
#include <iostream>
#include "nlohmann\json.hpp"
#include "amqp_client.h"
using namespace nlohmann;

bool ds::socket_acceptor_requests::on_accept(const socket_ptr& socket)
{
	// This one is responsible for accepting requests from outside the system (the API)
	socket_acceptor_auto_clean_up::on_accept(socket);
	receive_header(socket);

	return true;
}

void ds::socket_acceptor_requests::receive_header(socket_ptr socket)
{
	auto header = std::make_shared<header_t>();

	boost::asio::async_read(*socket, mdk::to_asio_buffer(*header),
		[=, ensure_lifetime = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (bytes_transferred == 0 || ec != boost::system::errc::success)
			return; // Remote socket down. Please close local socket.

		receive_body(socket, header);
	});
}

void ds::socket_acceptor_requests::receive_body(socket_ptr socket, header_t::ptr header)
{
	// receive and process body
	if (header->_data.version != 1)
		return; // only 1 supported

	constexpr auto max_message_length = 10 * 1'024 * 1'024;
	if (header->_data.message_length > max_message_length)
		return;

	std::shared_ptr<char[]> body(new char[header->_data.message_length + 1]);
	body.get()[header->_data.message_length] = 0;

	boost::asio::async_read(*socket, boost::asio::buffer(body.get(), header->_data.message_length),
		[=, ensure_lifetime = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (bytes_transferred == 0 || ec != boost::system::errc::success)
			return; // Remote socket down. Please close local socket.

		try
		{
			auto data = json::parse(body.get());
			_publisher.publish("distributed_txns", "distributed_txns", data.dump())
				.onError([](const char* err)
				{
					// not implemented
				})
				.onSuccess([&]()
				{
					// not implemented
				});
		}
		catch (const nlohmann::detail::parse_error& e)
		{
			std::cout << e.what();
		}
		// continue on -> receive next message
		receive_header(socket);
	});
}


