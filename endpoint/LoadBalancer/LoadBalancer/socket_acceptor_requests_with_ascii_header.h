#pragma once
#include "socket_acceptor_requests.h"
namespace ds
{
	struct socket_acceptor_requests_with_ascii_header : socket_acceptor_requests
	{
		template<typename... Args> socket_acceptor_requests_with_ascii_header(Args&&... args)
			: socket_acceptor_requests(std::forward<Args>(args)...) {}

		virtual void receive_header(socket_ptr socket) const override;
	};
}
