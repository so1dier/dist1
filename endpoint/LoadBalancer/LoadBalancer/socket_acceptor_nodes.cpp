#include "stdafx.h"
#include "socket_acceptor_nodes.h"

bool ds::socket_acceptor_nodes::on_accept(const socket_ptr& socket)
{
	// This one will talk to the rasberry pi nodes... basically forwarding requests onto them
	socket_acceptor_auto_clean_up::on_accept(socket);
	throw std::exception("not implemented");
}

