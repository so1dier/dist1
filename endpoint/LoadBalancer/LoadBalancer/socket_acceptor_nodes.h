#pragma once
#include "socket_acceptor_auto_clean_up.h"
namespace ds
{
	struct socket_acceptor_nodes : socket_acceptor_auto_clean_up
	{
		template<typename... Args> socket_acceptor_nodes(Args&&... args)
			: socket_acceptor_auto_clean_up(std::forward<Args>(args)...) {}

		virtual bool on_accept(const socket_ptr& socket) override
		{
			// This one will talk to the rasberry pi nodes... basically forwarding requests onto them
			socket_acceptor_auto_clean_up::on_accept(socket);
			throw std::exception("not implemented");
		}
	};
}
