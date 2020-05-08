// Copyright 2017-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/middleware/fastdds/FastDDSMiddleware.hpp>

namespace eprosima {
namespace uxr {

bool FastDDSMiddleware::create_participant_by_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& ref)
{
    bool rv = false;
    std::shared_ptr<FastDDSParticipant> participant(new FastDDSParticipant(domain_id));
    if (participant->create_by_ref(ref))
    {
        participants_.emplace(participant_id, std::move(participant));
        rv = true;
    }
    return rv;
}

bool FastDDSMiddleware::create_participant_by_xml(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& xml)
{
    bool rv = false;
    std::shared_ptr<FastDDSParticipant> participant(new FastDDSParticipant(domain_id));
    if (participant->create_by_xml(xml))
    {
        participants_.emplace(participant_id, std::move(participant));
        rv = true;
    }
    return rv;
}

bool FastDDSMiddleware::create_topic_by_ref(
        uint16_t topic_id,
        uint16_t participant_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSTopic> topic(new FastDDSTopic(it_participant->second));
        if (topic->create_by_ref(ref, topic_id))
        {
            topics_.emplace(topic_id, std::move(topic));
            rv = true;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_topic_by_xml(
        uint16_t topic_id,
        uint16_t participant_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSTopic> topic(new FastDDSTopic(it_participant->second));
        if (topic->create_by_xml(xml, topic_id))
        {
            topics_.emplace(topic_id, std::move(topic));
            rv = true;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_publisher_by_xml(
        uint16_t publisher_id,
        uint16_t participant_id,
        const std::string& xml)
{   
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSPublisher> publisher(new FastDDSPublisher(it_participant->second));
        if (publisher->create_by_xml(xml))
        {
            publishers_.emplace(publisher_id, std::move(publisher));
            rv = true;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_subscriber_by_xml(
        uint16_t subscriber_id,
        uint16_t participant_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSSubscriber> subscriber(new FastDDSSubscriber(it_participant->second));
        if (subscriber->create_by_xml(xml))
        {
            subscribers_.emplace(subscriber_id, std::move(subscriber));
            rv = true;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_datawriter_by_ref(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_publisher = publishers_.find(publisher_id);
    if (publishers_.end() != it_publisher)
    {   
        std::shared_ptr<FastDDSDataWriter> datawriter(new FastDDSDataWriter(it_publisher->second));
        if (datawriter->create_by_ref(ref))
        {
            datawriters_.emplace(datawriter_id, std::move(datawriter));
            rv = true;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_datawriter_by_xml(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_publisher = publishers_.find(publisher_id);
    if (publishers_.end() != it_publisher)
    {   
        std::shared_ptr<FastDDSDataWriter> datawriter(new FastDDSDataWriter(it_publisher->second));
        if (datawriter->create_by_xml(xml))
        {
            datawriters_.emplace(datawriter_id, std::move(datawriter));
            rv = true;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_datareader_by_ref(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_subscriber = subscribers_.find(subscriber_id);
    if (subscribers_.end() != it_subscriber)
    {   
        std::shared_ptr<FastDDSDataReader> datareader(new FastDDSDataReader(it_subscriber->second));
        if (datareader->create_by_ref(ref))
        {
            datareaders_.emplace(datareader_id, std::move(datareader));
            rv = true;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_datareader_by_xml(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_subscriber = subscribers_.find(subscriber_id);
    if (subscribers_.end() != it_subscriber)
    {   
        std::shared_ptr<FastDDSDataReader> datareader(new FastDDSDataReader(it_subscriber->second));
        if (datareader->create_by_xml(xml))
        {
            datareaders_.emplace(datareader_id, std::move(datareader));
            rv = true;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_requester_by_ref(
        uint16_t requester_id,
        uint16_t participant_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSRequester> requester(new FastDDSRequester(it_participant->second));
        if (requester->create_by_ref(ref))
        {
            requesters_.emplace(requester_id, std::move(requester));
            rv = true;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_requester_by_xml(
        uint16_t requester_id,
        uint16_t participant_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSRequester> requester(new FastDDSRequester(it_participant->second));
        if (requester->create_by_xml(xml))
        {
            requesters_.emplace(requester_id, std::move(requester));
            rv = true;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_replier_by_ref(
        uint16_t replier_id,
        uint16_t participant_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSReplier> replier(new FastDDSReplier(it_participant->second));
        if (replier->create_by_ref(ref))
        {
            repliers_.emplace(replier_id, std::move(replier));
            rv = true;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_replier_by_xml(
        uint16_t replier_id,
        uint16_t participant_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSReplier> replier(new FastDDSReplier(it_participant->second));
        if (replier->create_by_xml(xml))
        {
            repliers_.emplace(replier_id, std::move(replier));
            rv = true;
        }
    }
    return rv;
}

bool FastDDSMiddleware::delete_participant(
        uint16_t participant_id)
{
    return (0 != participants_.erase(participant_id));
}

bool FastDDSMiddleware::delete_topic(
        uint16_t topic_id)
{
   return (0 != topics_.erase(topic_id));
}

bool FastDDSMiddleware::delete_publisher(
        uint16_t publisher_id)
{
   return (0 != publishers_.erase(publisher_id));
}

bool FastDDSMiddleware::delete_subscriber(
        uint16_t subscriber_id)
{
   return (0 != subscribers_.erase(subscriber_id));
}

bool FastDDSMiddleware::delete_datawriter(
        uint16_t datawriter_id)
{
   return (0 != datawriters_.erase(datawriter_id));
}

bool FastDDSMiddleware::delete_datareader(
        uint16_t datareader_id)
{
   return (0 != datareaders_.erase(datareader_id));
}

bool FastDDSMiddleware::delete_requester(
        uint16_t requester_id)
{
   return (0 != requesters_.erase(requester_id));
}

bool FastDDSMiddleware::delete_replier(
        uint16_t replier_id)
{
   return (0 != repliers_.erase(replier_id));
}

bool FastDDSMiddleware::write_data(
        uint16_t datawriter_id,
        const std::vector<uint8_t>& data)
{
   bool rv = false;
   auto it = datawriters_.find(datawriter_id);
   if (datawriters_.end() != it)
   {
       rv = it->second->write(data);
   }
   return rv;
}

bool FastDDSMiddleware::write_request(
        uint16_t requester_id,
        uint32_t sequence_number,
        const std::vector<uint8_t>& data)
{
   bool rv = false;
   auto it = requesters_.find(requester_id);
   if (requesters_.end() != it)
   {
       rv = it->second->write(sequence_number, data);
   }
   return rv;
}

bool FastDDSMiddleware::write_reply(
        uint16_t replier_id,
        const std::vector<uint8_t>& data)
{
   bool rv = false;
   auto it = repliers_.find(replier_id);
   if (repliers_.end() != it)
   {
       rv = it->second->write(data);
   }
   return rv;
}

bool FastDDSMiddleware::read_data(
        uint16_t datareader_id,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
   bool rv = false;
   auto it = datareaders_.find(datareader_id);
   if (datareaders_.end() != it)
   {
       rv = it->second->read(data, timeout);
   }
   return rv;
}

bool FastDDSMiddleware::read_request(
        uint16_t replier_id,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
   bool rv = false;
   auto it = repliers_.find(replier_id);
   if (repliers_.end() != it)
   {
       rv = it->second->read(data, timeout);
   }
   return rv;
}

bool FastDDSMiddleware::read_reply(
        uint16_t requester_id,
        uint32_t& sequence_number,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
   bool rv = false;
   auto it = requesters_.find(requester_id);
   if (requesters_.end() != it)
   {
       rv = it->second->read(sequence_number, data, timeout);
   }
   return rv;
}

bool FastDDSMiddleware::matched_participant_from_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& ref) const
{
    bool rv = false;
    auto it = participants_.find(participant_id);
    if (participants_.end() != it)
    {
        rv = (domain_id == it->second->get_domain_id()) && (it->second->match_from_ref(ref));
    }
    return rv;
}

bool FastDDSMiddleware::matched_participant_from_xml(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& xml) const
{
    bool rv = false;
    auto it = participants_.find(participant_id);
    if (participants_.end() != it)
    {
        rv = (domain_id == it->second->get_domain_id()) && (it->second->match_from_xml(xml));
    }
    return rv;
}

bool FastDDSMiddleware::matched_topic_from_ref(
        uint16_t topic_id,
        const std::string& ref) const
{
    bool rv = false;
    auto it = topics_.find(topic_id);
    if (topics_.end() != it)
    {
        rv = it->second->match_from_ref(ref);
    }
    return rv;
}

bool FastDDSMiddleware::matched_topic_from_xml(uint16_t topic_id, const std::string& xml) const
{
    bool rv = false;
    auto it = topics_.find(topic_id);
    if (topics_.end() != it)
    {
        rv = it->second->match_from_xml(xml);
    }
    return rv;
}

bool FastDDSMiddleware::matched_datawriter_from_ref(
        uint16_t datawriter_id,
        const std::string& ref) const
{
   bool rv = false;
   auto it = datawriters_.find(datawriter_id);
   if (datawriters_.end() != it)
   {
       rv = it->second->match_from_ref(ref);
   }
   return rv;
}

bool FastDDSMiddleware::matched_datawriter_from_xml(
        uint16_t datawriter_id,
        const std::string& xml) const
{
   bool rv = false;
   auto it = datawriters_.find(datawriter_id);
   if (datawriters_.end() != it)
   {
       rv = it->second->match_from_xml(xml);
   }
   return rv;
}

bool FastDDSMiddleware::matched_datareader_from_ref(
        uint16_t datareader_id,
        const std::string& ref) const
{
   bool rv = false;
   auto it = datareaders_.find(datareader_id);
   if (datareaders_.end() != it)
   {
       rv = it->second->match_from_ref(ref);
   }
   return rv;
}

bool FastDDSMiddleware::matched_datareader_from_xml(
        uint16_t datareader_id,
        const std::string& xml) const
{
   bool rv = false;
   auto it = datareaders_.find(datareader_id);
   if (datareaders_.end() != it)
   {
       rv = it->second->match_from_xml(xml);
   }
   return rv;
}

bool FastDDSMiddleware::matched_requester_from_ref(
        uint16_t requester_id,
        const std::string& ref) const
{
   bool rv = false;
   auto it = requesters_.find(requester_id);
   if (requesters_.end() != it)
   {
       rv = it->second->match_from_ref(ref);
   }
   return rv;
}

bool FastDDSMiddleware::matched_requester_from_xml(
        uint16_t requester_id,
        const std::string& xml) const
{
   bool rv = false;
   auto it = requesters_.find(requester_id);
   if (requesters_.end() != it)
   {
       rv = it->second->match_from_ref(xml);
   }
   return rv;
}

bool FastDDSMiddleware::matched_replier_from_ref(
        uint16_t requester_id,
        const std::string& ref) const
{
   bool rv = false;
   auto it = repliers_.find(requester_id);
   if (repliers_.end() != it)
   {
       rv = it->second->match_from_ref(ref);
   }
   return rv;
}

bool FastDDSMiddleware::matched_replier_from_xml(
        uint16_t requester_id,
        const std::string& xml) const
{
   bool rv = false;
   auto it = repliers_.find(requester_id);
   if (repliers_.end() != it)
   {
       rv = it->second->match_from_ref(xml);
   }
   return rv;
}

} // namespace uxr
} // namespace eprosima