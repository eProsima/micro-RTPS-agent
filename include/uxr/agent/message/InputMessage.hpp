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

#ifndef UXR_AGENT_MESSAGE_INPUT_MESSAGE_HPP_
#define UXR_AGENT_MESSAGE_INPUT_MESSAGE_HPP_

#include <uxr/agent/types/MessageHeader.hpp>
#include <uxr/agent/types/SubMessageHeader.hpp>

#include <fastcdr/Cdr.h>
#include <fastcdr/exceptions/Exception.h>

namespace eprosima {
namespace uxr {

class InputMessage
{
public:
    InputMessage(
            uint8_t* buf,
            size_t len)
        : buf_(new uint8_t[len]),
          len_(len),
          header_(),
          subheader_(),
          fastbuffer_(reinterpret_cast<char*>(buf_), len_),
          deserializer_(fastbuffer_)
    {
        memcpy(buf_, buf, len);
        deserialize(header_);
    }

    ~InputMessage()
    {
        delete[] buf_;
    }

    InputMessage(InputMessage&&) = delete;
    InputMessage(const InputMessage&) = delete;
    InputMessage& operator=(InputMessage&&) = delete;
    InputMessage& operator=(const InputMessage&) = delete;

    const dds::xrce::MessageHeader& get_header() const { return header_; }

    const dds::xrce::SubmessageHeader& get_subheader() const { return subheader_; }

    template<class T>
    bool get_payload(T& data);

    uint8_t get_raw_header(std::array<uint8_t, 8>& buf);

    bool get_raw_payload(uint8_t* buf, size_t len);

    bool prepare_next_submessage();

private:
    template<class T>
    bool deserialize(T& data);

    void log_error();

private:
    uint8_t* buf_;
    size_t len_;
    dds::xrce::MessageHeader header_;
    dds::xrce::SubmessageHeader subheader_;
    fastcdr::FastBuffer fastbuffer_;
    fastcdr::Cdr deserializer_;
};

inline bool InputMessage::prepare_next_submessage()
{
    bool rv = false;
    deserializer_.jump((4 - ((deserializer_.getCurrentPosition() - deserializer_.getBufferPointer()) & 3)) & 3);
    if (fastbuffer_.getBufferSize() > deserializer_.getSerializedDataLength())
    {
        rv = deserialize(subheader_);
    }
    return rv;
}

template<class T>
inline bool InputMessage::get_payload(T& data)
{
    bool rv = true;
    try
    {
        data.deserialize(deserializer_);
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException & /*exception*/)
    {
        log_error();
        rv = false;
    }
    return rv;
}

inline uint8_t InputMessage::get_raw_header(std::array<uint8_t, 8>& buf)
{
    uint8_t rv;
    if (128 > header_.session_id())
    {
        memcpy(buf.data(), buf_, 8);
        rv = 8;
    }
    else
    {
        memcpy(buf.data(), buf_, 4);
        rv = 4;
    }
    return rv;
}

inline bool InputMessage::get_raw_payload(uint8_t* buf, size_t len)
{
    bool rv = false;
    if (subheader_.submessage_length() <= len)
    {
        rv = true;
        try
        {
            deserializer_.deserializeArray(buf, subheader_.submessage_length(), fastcdr::Cdr::BIG_ENDIANNESS);
        }
        catch(eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
        {
            log_error();
            rv = false;
        }
    }
    return rv;
}

template<class T>
inline bool InputMessage::deserialize(T& data)
{
    bool rv = true;
    try
    {
        data.deserialize(deserializer_);
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException & /*exception*/)
    {
        log_error();
        rv = false;
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_MESSAGE_INPUT_MESSAGE_HPP_
