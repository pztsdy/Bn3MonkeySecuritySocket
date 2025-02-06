#if !defined(__BN3MONKEY__SOCKETBROADCASTSERVER__)
#define __BN3MONKEY__SOCKETBROADCASTSERVER__
#include "../SecuritySocket.hpp"

#include "ServerActiveSocket.hpp"

namespace Bn3Monkey
{
    class SocketBroadcastServerImpl
    {
    public:
		SocketBroadcastServerImpl(const SocketConfiguration& configuration) : _configuration(configuration) {}
		virtual ~SocketBroadcastServerImpl();

		SocketResult open(size_t num_of_clients);
        SocketResult write(const void* buffer, size_t size);
        void close();

	private:
		SocketConfiguration _configuration;
        size_t _num_of_clients;
    };
}

#endif // __BN3MONKEY__SOCKETEVENTSERVER__