#pragma once
#include "socket_acceptor.h"
namespace ds
{
	struct socket_acceptor_manage_io_service : socket_acceptor
	{
		template<typename... Args> socket_acceptor_manage_io_service(Args&&... args)
			: socket_acceptor(std::forward<Args>(args)...) {}
	protected:
		virtual bool on_accept(const socket_ptr& socket) override;
		
		void receive(const socket_ptr& socket) const;
	};
}
