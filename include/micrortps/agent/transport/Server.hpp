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

#ifndef _MICRORTPS_AGENT_TRANSPORT_SERVER_HPP_
#define _MICRORTPS_AGENT_TRANSPORT_SERVER_HPP_

#include <micrortps/agent/scheduler/FCFSScheduler.hpp>
#include <micrortps/agent/message/Packet.hpp>
#include <stdint.h>
#include <stddef.h>
#include <thread>

namespace eprosima {
namespace micrortps {

/**************************************************************************************************
 * EndPoint interface.
 **************************************************************************************************/
class EndPoint
{
public:
    EndPoint() {}
};

/**************************************************************************************************
 * Server interface.
 **************************************************************************************************/
class Server
{
public:
    Server() {}
    virtual ~Server() {}

    bool run();
    void stop();

    virtual bool recv_message(InputPacket& input_packet, int timeout) = 0;
    virtual bool send_message(OutputPacket output_packet) = 0;
    virtual int get_error() = 0;
    void push_output_packet(OutputPacket output_packet);

private:
    virtual bool init() = 0;
    void receiver_loop();
    void sender_loop();
    void processing_loop();

private:
    std::unique_ptr<std::thread> receiver_thread_;
    std::unique_ptr<std::thread> sender_thread_;
    std::unique_ptr<std::thread> processing_thread_;
    std::atomic<bool> running_cond_;
    FCFSScheduler<InputPacket> input_scheduler_;
    FCFSScheduler<OutputPacket> output_scheduler_;
};

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_TRANSPORT_XRCE_SERVER_HPP_