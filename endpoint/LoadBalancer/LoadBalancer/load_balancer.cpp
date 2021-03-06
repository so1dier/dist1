#include "stdafx.h"
#include "socket_acceptor_nodes.h"
#include "socket_acceptor_requests_with_ascii_header.h"
#include "socket_acceptor_manage_io_service.h"
int main()
{
	using namespace boost::asio::ip;
	const auto io_service = std::make_shared<boost::asio::io_service>();

	auto acceptor_for_nodes = std::make_shared<ds::socket_acceptor_nodes>(io_service, 1234)->start_accept();
	auto acceptor_for_requests = std::make_shared<ds::socket_acceptor_requests>(io_service, 1235, "127.0.0.1", "5672")->start_accept();
	auto acceptor_for_requests_ascii = std::make_shared<ds::socket_acceptor_requests_with_ascii_header>(io_service, 1236, "127.0.0.1", "5672")->start_accept();
	auto socket_acceptor_manage_io_service = std::make_shared<ds::socket_acceptor_manage_io_service>(io_service, 1237)->start_accept();

	io_service->run();
	return 0;
}

