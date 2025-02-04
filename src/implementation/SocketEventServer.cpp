#include "SocketEventServer.hpp"

using namespace Bn3Monkey;

SocketEventServerImpl::~SocketEventServerImpl()
{

}
SocketResult SocketEventServerImpl::open(size_t num_of_clients)
{
    _num_of_clients = num_of_clients;

    SocketResult result = SocketResult(SocketCode::SUCCESS);
#ifdef _WIN32
	WSADATA data;
	int ret = WSAStartup(MAKEWORD(2, 2), &data);
	if (ret != 0) {
		result = SocketResult(SocketCode::WINDOWS_SOCKET_INITIALIZATION_FAIL, "Cannot start windows socket");
		return result;
	}
#endif


	SocketAddress address{ _configuration.ip(), _configuration.port(), false };
	result = address;
	if (result.code() != SocketCode::SUCCESS) {
		return result;
	}

	if (_configuration.tls()) {
		_socket = reinterpret_cast<ActiveSocket*>(new(_container) TLSActiveSocket{ address });
	}
	else {
		_socket = new (_container) ActiveSocket{ address };
	}

	result = _socket->open();
	if (result.code() != SocketCode::SUCCESS)
	{
		return result;
	}
}
SocketResult SocketEventServerImpl::write(const void* buffer, size_t size)
{

}
void SocketEventServerImpl::close()
{
    _socket->close();
#ifdef _WIN32
    WSACleanup();
#endif
}