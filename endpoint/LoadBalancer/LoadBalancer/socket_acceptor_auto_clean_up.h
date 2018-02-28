#pragma once
#include "socket_acceptor.h"
namespace ds
{
	// Automatically clean up dead sockets every request? Then inherit from this. 
	struct socket_acceptor_auto_clean_up : socket_acceptor
	{
		template<typename... Args> socket_acceptor_auto_clean_up(Args&&... args)
			: socket_acceptor(std::forward<Args>(args)...) {}
	protected:
		virtual bool on_accept(const socket_ptr&) override
		{
			clean_up();
			return true;
		}
	};
}
