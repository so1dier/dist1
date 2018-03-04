#include "stdafx.h"
#include "amqp_client.h"

ds::rabbitmq::detail::connection_handler::connection_handler(const io_service_ptr& io_service, std::string const& host, std::string const& port)
	: _socket(std::make_shared<socket_t>(*io_service)), _io_service(io_service)
{
	boost::asio::connect(*_socket, tcp::resolver(*_io_service).resolve(tcp::resolver::query(host, port)));
}

/**
*  Method that is called by the AMQP library every time it has data
*  available that should be sent to RabbitMQ.
*  @param  connection  pointer to the main connection object
*  @param  data        memory buffer with the data that should be sent to RabbitMQ
*  @param  size        size of the buffer
*/
void ds::rabbitmq::detail::connection_handler::onData(AMQP::Connection *connection, const char *data, size_t size)
{
	boost::asio::async_write(*_socket, boost::asio::const_buffer(data, size), [](auto& ec, std::size_t)
	{
		if (ec)
		{
			// not implemented
		}
	});
}

/**
*  Method that is called by the AMQP library when the login attempt
*  succeeded. After this method has been called, the connection is ready
*  to use.
*  @param  connection      The connection that can now be used
*/
void ds::rabbitmq::detail::connection_handler::onConnected(AMQP::Connection *connection)
{
	// not implemented
}

/**
*  Method that is called when the connection was closed. This is the
*  counter part of a call to Connection::close() and it confirms that the
*  connection was correctly closed.
*
*  @param  connection      The connection that was closed and that is now unusable
*/
void ds::rabbitmq::detail::connection_handler::onClosed(AMQP::Connection *connection)
{
	// not implemented
}

void ds::rabbitmq::detail::connection_handler::onError(AMQP::Connection *connection, const char *message)
{
	// not implemented
}

ds::rabbitmq::detail::connection_impl::connection_impl(connection_handler * handler)
	: socket(*handler->_socket)
	, amqp_impl(handler, AMQP::Login("node", "node"), "/")
	, buffer()
{
}

void ds::rabbitmq::detail::connection_impl::keep_reading()
{
	socket.async_read_some
	(
		boost::asio::buffer(buffer, sizeof(buffer)),
		[this, self = shared_from_this()](auto& ec, std::size_t length)
		{
			if (!ec)
			{
				// send the received data to the amqp layer
				amqp_impl.parse(buffer, length);
				keep_reading();
			}
			else
			{
				// not implemented
			}
		}
	);
}

ds::rabbitmq::client::client(const io_service_ptr & io_service, std::string const & host, std::string const & port) 
	: handler(io_service, host, port), impl(new detail::connection_impl(&handler))
{
	impl->keep_reading();
}
