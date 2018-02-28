#include "stdafx.h"
#include "socket_acceptor_nodes.h"
#include "socket_acceptor_requests_with_ascii_header.h"
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

int main()
{
	using namespace boost::asio::ip;
	const auto io_service = std::make_shared<boost::asio::io_service>();

	auto acceptor_for_nodes = std::make_shared<ds::socket_acceptor_nodes>(io_service, 1234)->start_accept();
	auto acceptor_for_requests = std::make_shared<ds::socket_acceptor_requests>(io_service, 1235)->start_accept();
	auto acceptor_for_requests_ascii = std::make_shared<ds::socket_acceptor_requests_with_ascii_header>(io_service, 1236)->start_accept();
	
	// test io service stop...
	//std::thread t([=]() 
	//{
	//	std::this_thread::sleep_for(10s);
	//	io_service->stop();
	//});
	
	io_service->run();
	//t.join();

	return 0;
}

