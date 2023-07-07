#if !defined(__BN3MONKEY_SECURITY_SOCKET__)
#define __BN3MONKEY_SECURITY_SOCKET__

#include <string>
#include <cstdint>
#include <memory>

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <WS2tcpip.h>
#else
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <netinet/in.h>
#endif

#include <mutex>
#include <condition_variable>
#include <vector>

namespace Bn3Monkey
{
    enum class ConnectionCode
    {
        SUCCESS,

        ADDRESS_NOT_AVAILABLE,

        WINDOWS_SOCKET_INITIALIZATION_FAIL,
        TLS_CONTEXT_INITIALIZATION_FAIL,
        TLS_INITIALIZATION_FAIL,

        // - SOCKET INITIALIZATION
        SOCKET_PERMISSION_DENIED,
        SOCKET_ADDRESS_FAMILY_NOT_SUPPORTED,
        SOCKET_INVALID_ARGUMENT,
        SOCKET_CANNOT_CREATED,
        SOCKET_CANNOT_ALLOC,
        SOCKET_OPTION_ERROR,

        // - SOCKET CONNECTION
        // SOCKET_PERMISSION_DENIED,
        // SOCKET_ADDRESS_NOT_SUPPORTED
        SOCKET_CONNECTION_ADDRESS_IN_USE,
        SOCKET_CONNECTION_BAD_DESCRIPTOR,
        SOCKET_CONNECTION_REFUSED,
        SOCKET_CONNECTION_BAD_ADDRESS,
        SOCKET_CONNECTION_UNREACHED,
        SOCKET_CONNECTION_INTERRUPTED,
        
        SOCKET_CONNECTION_ALREADY,
        SOCKET_CONNECTION_IN_PROGRESS,

        TLS_SETFD_ERROR,

        SSL_PROTOCOL_ERROR,
        SSL_ERROR_CLOSED_BY_PEER,

        SOCKET_TIMEOUT,
        SOCKET_CLOSED,

        POLL_ERROR,
        UNKNOWN_ERROR,

        LENGTH,
    };
    struct ConnectionResult
    {
        ConnectionCode code{ ConnectionCode::SUCCESS };
        std::string message;
        
        ConnectionResult(const ConnectionCode& code = ConnectionCode::SUCCESS, const std::string& message = "") : code(code), message(message) {}
    };


    struct TCPConfiguration {
        constexpr static size_t MAX_PDU_SIZE = 32768;

        const std::string ip;
        const std::string port;
        const bool tls;
        const uint32_t max_retries;
        const uint32_t timeout_milliseconds;
        const size_t pdu_size;

        explicit TCPConfiguration(
            const std::string& ip,
            const std::string& port,
            bool tls,
            uint32_t max_retries,
            uint32_t timeout_milliseconds,
            size_t pdu_size = MAX_PDU_SIZE) : 
            ip(ip), port(port), tls(tls), 
            max_retries(max_retries), timeout_milliseconds(timeout_milliseconds),
            pdu_size(pdu_size)
        {}
    };

    class TCPEventHandler
    {
    public:
        
        virtual void onConnected() = 0;
        virtual void onDisconnected() = 0;
        virtual void onRead(char* buffer, size_t size) = 0;
        virtual void onWrite(char* buffer, size_t size) = 0;
        virtual void onError(const ConnectionResult& result) = 0;

    private:
    };

    class TCPStreamHandler : public TCPEventHandler
    {
    public:
        TCPStreamHandler(size_t size) : _read_buffer(size), _write_buffer(size) {

        }
        void onConnected() override {
            printf("Socket Connected!");
            _read_dirty = false;
            _write_dirty = false;
        }
        void onDisconnected() override {
            printf("Socket Disconnected!");
            _read_dirty = true;
            _write_dirty = true;
            _read_cv.notify_all();
            _write_cv.notify_all();
        }

        void read(char* buffer, size_t size) {
            {
                std::unique_lock<std::mutex> lock(_read_mtx);
                _read_cv.wait(
                    lock, [&]() {
                        return _read_dirty == true;
                    }
                );

                ::memcpy(buffer, _read_buffer.data(), size);
                _read_dirty = false;
            }
        }
        void onRead(char* buffer, size_t size) override {
            {
                std::lock_guard<std::mutex> lock(_read_mtx);
                ::memcpy(_read_buffer.data(), buffer, size);
                _read_dirty = true;
            }
            _read_cv.notify_all();
        }

        void write(char* buffer, size_t size) {
            {
                std::lock_guard<std::mutex> lock(_read_mtx);
                ::memcpy(_write_buffer.data(), buffer, size);
                _write_dirty = true;
            }
            _write_cv.notify_all();
        }
        void onWrite(char* buffer, size_t size) override {
            {
                std::unique_lock<std::mutex> lock(_write_mtx);
                _write_cv.wait(
                    lock, [&]() {
                        return _write_dirty == true;
                    }
                );

                ::memcpy(buffer, _write_buffer.data(), size);
                _write_dirty = false;
            }
        }

        void onError(const ConnectionResult& result) override {
            printf("result : %s\n", result.message.c_str());
        }

    private:
        std::vector<char> _read_buffer;
        std::mutex _read_mtx;
        std::condition_variable _read_cv;
        bool _read_dirty{ false };

        std::vector<char> _write_buffer;
        std::mutex _write_mtx;
        std::condition_variable _write_cv;
        bool _write_dirty{ false };
    };

    class TCPClientImpl;
    class TCPServerImpl;

    class TCPClient
    {
    public:
        explicit TCPClient(
            const TCPConfiguration& configuration,
            TCPEventHandler& handler
            );
        
        virtual ~TCPClient();

        ConnectionResult getLastError();
        void close();
    private:
        std::shared_ptr<TCPClientImpl> _impl;
    };

    class TCPServer
    {
    public:
        explicit TCPServer(
            const TCPConfiguration& configuration,
            TCPEventHandler& handler
        );
        virtual ~TCPServer();

        ConnectionResult getLastError();
        void close();
    private:
        std::shared_ptr<TCPServerImpl> _impl;
    };

}

#endif