#pragma once
#include "socket_acceptor_auto_clean_up.h"
namespace ds
{
	struct socket_acceptor_requests : socket_acceptor_auto_clean_up
	{
		template<typename... Args> socket_acceptor_requests(Args&&... args)
			: socket_acceptor_auto_clean_up(std::forward<Args>(args)...) {}

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
			socket_acceptor_auto_clean_up::on_accept(socket);
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

				std::cout << body.get();
				// continue on -> receive next message
				receive_header(socket);
			});
		}
	};
}
