#include "ServerActiveSocket.hpp"
#include "SocketResult.hpp"
#include "SocketHelper.hpp"
using namespace Bn3Monkey;

ServerActiveSocket::ServerActiveSocket(int32_t sock)
{
    _socket = sock;
    _result = createResult(_socket);
    if (_result.code() != SocketCode::SUCCESS)
    {
        return;
    }

    setNonBlockingMode(_socket);
}
ServerActiveSocket::~ServerActiveSocket()
{
    close();
}
void ServerActiveSocket::close()
{
#ifdef _WIN32
	::closesocket(_socket);
#else
	::close(_socket);
#endif
    _socket = -1;
}
int ServerActiveSocket::read(void* buffer, size_t size)
{
	int32_t ret{ 0 };
	ret = ::recv(_socket, static_cast<char*>(buffer), size, 0);
	return ret;
}
int ServerActiveSocket::write(const void* buffer, size_t size)
{
	int32_t ret{0};
#ifdef __linux__
	ret = send(_socket, buffer, size, MSG_NOSIGNAL);
#else
	ret = send(_socket, static_cast<const char*>(buffer), size, 0);
#endif
	return ret;
}