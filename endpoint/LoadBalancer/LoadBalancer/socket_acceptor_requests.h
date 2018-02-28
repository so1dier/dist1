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
		virtual bool on_accept(const socket_ptr& socket) override;
		virtual void receive_header(socket_ptr socket) const;
		void receive_body(socket_ptr socket, header_t::ptr header) const;
	};
}
