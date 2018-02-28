#pragma once
#include "socket_acceptor_requests.h"
namespace ds
{
	struct socket_acceptor_requests_with_ascii_header : socket_acceptor_requests
	{
		template<typename... Args> socket_acceptor_requests_with_ascii_header(Args&&... args)
			: socket_acceptor_requests(std::forward<Args>(args)...) {}

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
