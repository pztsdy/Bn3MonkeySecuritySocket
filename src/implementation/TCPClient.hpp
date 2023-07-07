#if !defined(__BN3MONKEY__TCPCLIENT__)
#define __BN3MONKEY__TCPCLIENT__

#include "../SecuritySocket.hpp"
#include "TCPSocket.hpp"
#include "TCPStream.hpp"

#include <type_traits>
#include <atomic>

#include <mutex>
#include <thread>
#include <condition_variable>

namespace Bn3Monkey
{
	class TCPClientImpl : public Bn3Monkey::TCPStream
	{
	public:
		TCPClientImpl() = delete;
		explicit TCPClientImpl(const TCPConfiguration& configuration, TCPEventHandler& handler);
		virtual ~TCPClientImpl();

		void close();

	private:
				
		static constexpr size_t container_size = sizeof(TCPSocket) > sizeof(TLSSocket) ? sizeof(TCPSocket) : sizeof(TLSSocket);
		char _container[container_size];
		TCPSocket* _socket{ nullptr };

		TCPEventHandler& _handler;
	};
}

#endif