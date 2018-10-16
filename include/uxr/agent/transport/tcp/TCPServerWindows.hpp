// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _UXR_AGENT_TRANSPORT_TCP_SERVER_HPP_
#define _UXR_AGENT_TRANSPORT_TCP_SERVER_HPP_

#include <uxr/agent/transport/Server.hpp>
#include <uxr/agent/config.hpp>
#include <unordered_map>
#include <winsock2.h>
#include <vector>
#include <array>
#include <list>
#include <set>

namespace eprosima {
namespace uxr {

/**************************************************************************************************
 * TCP EndPoint.
 **************************************************************************************************/
typedef enum TCPInputBufferState
{
    TCP_BUFFER_EMPTY,
    TCP_SIZE_INCOMPLETE,
    TCP_SIZE_READ,
    TCP_MESSAGE_INCOMPLETE,
    TCP_MESSAGE_AVAILABLE

} TCPInputBufferState;

struct TCPInputBuffer
{
    std::vector<uint8_t> buffer;
    uint16_t position;
    TCPInputBufferState state;
    uint16_t msg_size;
};

struct TCPConnection
{
    struct pollfd* poll_fd;
    TCPInputBuffer input_buffer;
    uint32_t addr;
    uint16_t port;
    uint32_t id;
    bool active;
    std::mutex mtx;
};

class TCPEndPoint : public EndPoint
{
public:
    TCPEndPoint(uint32_t addr, uint16_t port) : addr_(addr), port_(port) {}
    ~TCPEndPoint() = default;

    uint32_t get_addr() const { return addr_; }
    uint16_t get_port() const { return port_; }

private:
    uint32_t addr_;
    uint16_t port_;
};

/**************************************************************************************************
 * TCP Server.
 **************************************************************************************************/
class TCPServer : public Server
{
public:
    microxrcedds_agent_DllAPI TCPServer(uint16_t port);
    microxrcedds_agent_DllAPI ~TCPServer() = default;

    virtual void on_create_client(EndPoint* source, const dds::xrce::ClientKey& client_key) override;
    virtual void on_delete_client(EndPoint* source) override;
    virtual const dds::xrce::ClientKey get_client_key(EndPoint *source) override;
    virtual std::unique_ptr<EndPoint> get_source(const dds::xrce::ClientKey& client_key) override;

private:
    virtual bool init() override;
    virtual bool close() override;
    virtual bool recv_message(InputPacket& input_packet, int timeout) override;
    virtual bool send_message(OutputPacket output_packet) override;
    virtual int get_error() override;
    bool read_message(int timeout);
    uint16_t read_data(TCPConnection& connection);
    bool open_connection(SOCKET fd, struct sockaddr_in* sockaddr);
    bool close_connection(TCPConnection& connection);
    bool connection_available();
    void listener_loop();
    static void init_input_buffer(TCPInputBuffer& buffer);
    static int recv_locking(TCPConnection& connection, char* buffer, int len);
    static int send_locking(TCPConnection& connection, char* buffer, int len);

private:
    uint16_t port_;
    std::array<TCPConnection, TCP_MAX_CONNECTIONS> connections_;
    std::set<uint32_t> active_connections_;
    std::list<uint32_t> free_connections_;
    std::mutex connections_mtx_;
    struct pollfd listener_poll_;
    std::array<struct pollfd, TCP_MAX_CONNECTIONS> poll_fds_;
    uint8_t buffer_[TCP_TRANSPORT_MTU];
    std::unordered_map<uint64_t, uint32_t> source_to_connection_map_;
    std::unordered_map<uint64_t, uint32_t> source_to_client_map_;
    std::unordered_map<uint32_t, uint64_t> client_to_source_map_;
    std::mutex clients_mtx_;
    std::unique_ptr<std::thread> listener_thread_;
    std::atomic<bool> running_cond_;
    std::queue<InputPacket> messages_queue_;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_TRANSPORT_TCP_SERVER_HPP_