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

#ifndef _UXR_AGENT_CLIENT_SESSION_STREAM_INPUT_STREAM_HPP_
#define _UXR_AGENT_CLIENT_SESSION_STREAM_INPUT_STREAM_HPP_

#include <uxr/agent/config.hpp>
#include <uxr/agent/message/Packet.hpp>
#include <uxr/agent/utils/SeqNum.hpp>
#include <map>
#include <mutex>

namespace eprosima {
namespace uxr {

/**************************************************************************************************
 * None Input Streams.
 **************************************************************************************************/
class NoneInputStream
{
public:
    NoneInputStream() {}

    bool next_message(SeqNum seq_num);
};

inline bool NoneInputStream::next_message(SeqNum seq_num)
{
    (void) seq_num;
    return true;
}

/**************************************************************************************************
 * Best-Effort Input Streams.
 **************************************************************************************************/
class BestEffortInputStream
{
public:
    BestEffortInputStream() : last_received_(UINT16_MAX) {}

    bool next_message(SeqNum seq_num);
    void reset() { last_received_ = UINT16_MAX; }

private:
    SeqNum last_received_;
    std::mutex mtx_;
};

inline bool BestEffortInputStream::next_message(SeqNum seq_num)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (seq_num > last_received_)
    {
        last_received_ = seq_num;
        return true;
    }
    return false;
}

/**************************************************************************************************
 * Reliable Input Stream.
 **************************************************************************************************/
class ReliableInputStream
{
public:
    ReliableInputStream()
        : last_handled_(UINT16_MAX),
          last_announced_(UINT16_MAX),
          fragment_msg_{},
          fragment_message_available_(false)
    {}

    ReliableInputStream(const ReliableInputStream&) = delete;
    ReliableInputStream& operator=(const ReliableInputStream) = delete;
    ReliableInputStream(ReliableInputStream&&);
    ReliableInputStream& operator=(ReliableInputStream&&);

    bool next_message(SeqNum seq_num, InputMessagePtr& message);
    bool pop_message(InputMessagePtr& message);
    void update_from_heartbeat(SeqNum first_available, SeqNum last_available);
    SeqNum get_first_unacked();
    std::array<uint8_t, 2> get_nack_bitmap();
    void reset();
    void push_fragment(InputMessagePtr& message);
    bool pop_fragment_message(InputMessagePtr& message);

private:
    SeqNum last_handled_;
    SeqNum last_announced_;
    std::map<uint16_t, InputMessagePtr> messages_;
    std::vector<uint8_t> fragment_msg_;
    bool fragment_message_available_;
    std::mutex mtx_;
};

inline ReliableInputStream::ReliableInputStream(ReliableInputStream&& x)
    : last_handled_(x.last_handled_),
      last_announced_(x.last_announced_),
      messages_(std::move(x.messages_))
{
}

inline ReliableInputStream& ReliableInputStream::operator=(ReliableInputStream&& x)
{
    last_handled_ = x.last_handled_;
    last_announced_ = x.last_announced_;
    messages_ = std::move(x.messages_);
    return *this;
}

inline bool ReliableInputStream::next_message(SeqNum seq_num, InputMessagePtr& message)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (seq_num == last_handled_ + 1)
    {
        last_handled_ += 1;
        rv = true;
    }
    else
    {
        if ((seq_num > last_handled_ + 1) && (seq_num <= last_handled_ + SeqNum(RELIABLE_STREAM_DEPTH)))
        {
            if (seq_num > last_announced_)
            {
                last_announced_ = seq_num;
                messages_.insert(std::make_pair(seq_num, std::move(message)));
            }
            else
            {
                auto it = messages_.find(seq_num);
                if (it == messages_.end())
                {
                    messages_.insert(std::make_pair(seq_num, std::move(message)));
                }
            }
        }
    }
    return rv;
}

inline bool ReliableInputStream::pop_message(InputMessagePtr& message)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = messages_.find(last_handled_ + 1);
    if (it != messages_.end())
    {
        last_handled_ += 1;
        message = std::move(messages_.at(last_handled_));
        messages_.erase(last_handled_);
        rv = true;
    }
    return rv;
}

inline void ReliableInputStream::update_from_heartbeat(SeqNum first_available, SeqNum last_available)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (last_handled_ + 1 < first_available)
    {
        last_handled_ = first_available;
    }
    if (last_announced_ < last_available)
    {
        last_announced_ = last_available;
    }
}

inline SeqNum ReliableInputStream::get_first_unacked()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return last_handled_ + 1;
}

inline std::array<uint8_t, 2> ReliableInputStream::get_nack_bitmap()
{
    std::array<uint8_t, 2> bitmap = {0, 0};
    std::lock_guard<std::mutex> lock(mtx_);
    for (uint16_t i = 0; i < 8; i++)
    {
        if (last_handled_ + SeqNum(i) < last_announced_)
        {
            auto it = messages_.find(last_handled_ + SeqNum(i + 1));
            if (it == messages_.end())
            {
                bitmap.at(1) = bitmap.at(1) | (0x01 << i);
            }
        }
        if (last_handled_ + SeqNum(i + 8) < last_announced_)
        {
            auto it = messages_.find(last_handled_ + SeqNum(i + 9));
            if (it == messages_.end())
            {
                bitmap.at(0) = bitmap.at(0) | (0x01 << i);
            }
        }
    }
    return bitmap;
}

inline void ReliableInputStream::reset()
{
    std::lock_guard<std::mutex> lock(mtx_);
    last_handled_ = UINT16_MAX;
    last_announced_ = UINT16_MAX;
    messages_.clear();
}

inline void ReliableInputStream::push_fragment(InputMessagePtr& message)
{
    std::lock_guard<std::mutex> lock(mtx_);

    /* Add header in case. */
    if (fragment_msg_.empty())
    {
        std::array<uint8_t, 8> raw_header;
        uint8_t header_size = message->get_raw_header(raw_header);
        fragment_msg_.insert(fragment_msg_.begin(), std::begin(raw_header), std::begin(raw_header) + header_size);
    }

    /* Append fragment. */
    size_t position = fragment_msg_.size();
    size_t fragment_size = message->get_subheader().submessage_length();
    fragment_msg_.resize(position + fragment_size);
    message->get_raw_payload(fragment_msg_.data() + position, fragment_size);

    /* Check if last message. */
    fragment_message_available_ = (0 != (dds::xrce::FLAG_LAST_FRAGMENT & message->get_subheader().flags()));
}

inline bool ReliableInputStream::pop_fragment_message(InputMessagePtr& message)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    if (fragment_message_available_)
    {
        message.reset(new InputMessage(fragment_msg_.data(), fragment_msg_.size()));
        fragment_msg_.clear();
        fragment_message_available_ = false;
        return true;
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_CLIENT_SESSION_STREAM_INPUT_STREAM_HPP_
