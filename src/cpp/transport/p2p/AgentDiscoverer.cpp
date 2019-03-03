// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/transport/p2p/AgentDiscoverer.hpp>

namespace eprosima {
namespace uxr {

AgentDiscoverer::AgentDiscoverer()
    : running_cond_{false}
{
}

bool AgentDiscoverer::run(uint16_t discovery_port)
{
    if (running_cond_ || !init(discovery_port))
    {
        return false;
    }

    /* Init thread. */
    running_cond_ = true;
    thread_ = std::thread(&AgentDiscoverer::loop, this);

    return true;
}

bool AgentDiscoverer::stop()
{
    /* Stop thread. */
    running_cond_ = false;
    if (thread_.joinable())
    {
        thread_.join();
    }
    return close();
}

void AgentDiscoverer::loop()
{
    /* Header. */
    dds::xrce::MessageHeader header;
    header.session_id(dds::xrce::SESSIONID_NONE_WITHOUT_CLIENT_KEY);
    header.stream_id(dds::xrce::STREAMID_NONE);
    header.sequence_nr(0x0000);

    /* GET_INFO subheader. */
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(dds::xrce::GET_INFO);
    subheader.flags(dds::xrce::FLAG_LITTLE_ENDIANNESS);

    /* GET_INFO payload. */
    dds::xrce::GET_INFO_Payload payload;
    payload.info_mask(dds::xrce::INFO_ACTIVITY);

    /* Compute message size. */
    const size_t message_size =
            header.getCdrSerializedSize() +
            subheader.getCdrSerializedSize() +
            payload.getCdrSerializedSize();

    OutputMessage output_message{header, message_size};
    InputMessagePtr input_message;

    while (running_cond_)
    {
        send_message(output_message);
        bool message_received;
        do
        {
            message_received = recv_message(input_message, 0);
            if (message_received)
            {
                dds::xrce::INFO_Payload info_payload;
                input_message->prepare_next_submessage();
                input_message->get_payload(info_payload);
                // TODO (julian): call InternalClientManager ...
            }
        } while(message_received);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

} // namespace uxr
} // namespace eprosima
