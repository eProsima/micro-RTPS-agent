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

#include <uxr/agent/client/ProxyClient.hpp>
#include <uxr/agent/publisher/Publisher.hpp>
#include <uxr/agent/subscriber/Subscriber.hpp>
#include <uxr/agent/datareader/DataReader.hpp>
#include <uxr/agent/datawriter/DataWriter.hpp>
#include <uxr/agent/topic/Topic.hpp>
#include <uxr/agent/logger/Logger.hpp>

#ifdef PROFILE_FAST_MIDDLEWARE
#include <uxr/agent/middleware/fast/FastMiddleware.hpp>
#endif

#ifdef PROFILE_CED_MIDDLEWARE
#include <uxr/agent/middleware/ced/CedMiddleware.hpp>
#endif

namespace eprosima {
namespace uxr {

ProxyClient::ProxyClient(
        const dds::xrce::CLIENT_Representation& representation,
        Middleware::Kind middleware_kind)
    : representation_(representation)
    , objects_()
    , session_(SessionInfo{representation.client_key(), representation.session_id(), representation.mtu()})
{
    switch (middleware_kind)
    {
#ifdef PROFILE_FAST_MIDDLEWARE
        case Middleware::Kind::FAST:
        {
            middleware_.reset(new FastMiddleware());
            break;
        }
#endif
#ifdef PROFILE_CED_MIDDLEWARE
        case Middleware::Kind::CED:
        {
            middleware_.reset(new CedMiddleware(conversion::clientkey_to_raw(representation.client_key())));
            break;
        }
#endif
    }
}

dds::xrce::ResultStatus ProxyClient::create_object(
        const dds::xrce::CreationMode& creation_mode,
        const dds::xrce::ObjectPrefix& objectid_prefix,
        const dds::xrce::ObjectVariant& object_representation)
{
    dds::xrce::ResultStatus result;
    result.status(dds::xrce::STATUS_OK);
    result.implementation_status(0x00);

    /* Compute ObjectId. */
    dds::xrce::ObjectId object_id;
    object_id[0] = objectid_prefix[0];
    object_id[1] = (objectid_prefix[1] & 0xF0) | object_representation._d();

    /* Check whether object exists. */
    std::unique_lock<std::mutex> lock(mtx_);
    auto it = objects_.find(object_id);
    bool exists = (it != objects_.end());

    /* Create object according with creation mode (see Table 7 XRCE). */
    if (!exists)
    {
        if (!create_object(object_id, object_representation))
        {
            result.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
        }
    }
    else
    {
        if (!creation_mode.reuse())
        {
            if (!creation_mode.replace())
            {
                result.status(dds::xrce::STATUS_ERR_ALREADY_EXISTS);

                UXR_AGENT_LOG_WARN(
                    "client_key: 0x{0:08X}, object_id: 0x{1:04X}, error: ALREADY_EXISTS",
                    convertion::clientkey_to_raw(representation_.client_key()),
                    convertion::objectid_to_raw(object_id));
            }
            else
            {
                objects_.erase(object_id);
                if (!create_object(object_id, object_representation))
                {
                    result.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);

                    UXR_AGENT_LOG_WARN(
                        "client_key: 0x{0:08X}, object_id: 0x{1:04X}, error: UNKNOWN_REFERENCE",
                        convertion::clientkey_to_raw(representation_.client_key()),
                        convertion::objectid_to_raw(object_id));
                }
            }
        }
        else
        {
            if (!creation_mode.replace())
            {
                if (it->second->matched(object_representation))
                {
                    result.status(dds::xrce::STATUS_OK_MATCHED);
                }
                else
                {
                    result.status(dds::xrce::STATUS_ERR_MISMATCH);

                    UXR_AGENT_LOG_WARN(
                        "client_key: 0x{0:08X}, object_id: 0x{1:04X}, error: MISMATCH",
                        convertion::clientkey_to_raw(representation_.client_key()),
                        convertion::objectid_to_raw(object_id));
                }
            }
            else
            {
                if (it->second->matched(object_representation))
                {
                    result.status(dds::xrce::STATUS_OK_MATCHED);
                }
                else
                {
                    objects_.erase(object_id);
                    if (!create_object(object_id, object_representation))
                    {
                        result.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);

                        UXR_AGENT_LOG_WARN(
                            "client_key: 0x{0:08X}, object_id: 0x{1:04X}, error: UNKNOWN_REFERENCE",
                            convertion::clientkey_to_raw(representation_.client_key()),
                            convertion::objectid_to_raw(object_id));
                    }
                }
            }
        }
    }

    return result;
}

dds::xrce::ResultStatus ProxyClient::delete_object(const dds::xrce::ObjectId& object_id)
{
    dds::xrce::ResultStatus result;
    result.status(dds::xrce::STATUS_OK);
    result.implementation_status(0x00);

    std::lock_guard<std::mutex> lock(mtx_);
    auto it = objects_.find(object_id);
    if (it != objects_.end())
    {
        it->second->release(objects_);
        objects_.erase(object_id);

        UXR_AGENT_LOG_DEBUG(
            "client_key: 0x{0:08X}, object_id: 0x{1:04X}",
            convertion::clientkey_to_raw(representation_.client_key()),
            convertion::objectid_to_raw(object_id));
    }
    else
    {
        result.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);

        UXR_AGENT_LOG_WARN(
            "client_key: 0x{0:08X}, object_id: 0x{1:04X}, error: UNKNOWN_REFERENCE",
            convertion::clientkey_to_raw(representation_.client_key()),
            convertion::objectid_to_raw(object_id));
    }

    return result;
}

dds::xrce::ResultStatus ProxyClient::update(
        const dds::xrce::ObjectId& /*object_id*/,
        const dds::xrce::ObjectVariant& /*representation*/)
{
    // TODO (Borja):
    return dds::xrce::ResultStatus{};
}

dds::xrce::ObjectInfo ProxyClient::get_info(const dds::xrce::ObjectId& /*object_id*/)
{
    // TODO (Borja):
    return dds::xrce::ObjectInfo{};
}

std::shared_ptr<XRCEObject> ProxyClient::get_object(const dds::xrce::ObjectId& object_id)
{
    std::shared_ptr<XRCEObject> object;
    std::lock_guard<std::mutex> lock(mtx_);
    auto object_it = objects_.find(object_id);
    if (object_it != objects_.end())
    {
        object = object_it->second;
    }
    return object;
}

Session& ProxyClient::session()
{
    return session_;
}

bool ProxyClient::create_object(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::ObjectVariant& representation)
{
    bool rv = false;
    switch (representation._d())
    {
        case dds::xrce::OBJK_PARTICIPANT:
            rv = create_participant(object_id, representation.participant());
            break;
        case dds::xrce::OBJK_TOPIC:
            rv = create_topic(object_id, representation.topic());
            break;
        case dds::xrce::OBJK_PUBLISHER:
            rv = create_publisher(object_id, representation.publisher());
            break;
        case dds::xrce::OBJK_SUBSCRIBER:
            rv = create_subscriber(object_id, representation.subscriber());
            break;
        case dds::xrce::OBJK_DATAWRITER:
            rv = create_datawriter(object_id, representation.data_writer());
            break;
        case dds::xrce::OBJK_DATAREADER:
            rv = create_datareader(object_id, representation.data_reader());
            break;
        case dds::xrce::OBJK_APPLICATION:
            /* TODO (Julian). */
            break;
        case dds::xrce::OBJK_QOSPROFILE:
            /* TODO (Julian). */
            break;
        case dds::xrce::OBJK_TYPE:
            /* TODO (Julian). */
            break;
        default:
            break;
    }
    return rv;
}

bool ProxyClient::create_participant(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::OBJK_PARTICIPANT_Representation& representation)
{
    bool rv = false;
    if (dds::xrce::OBJK_PARTICIPANT == (object_id[1] & 0x0F))
    {
        if (std::unique_ptr<Participant> participant = Participant::create(object_id, representation, *middleware_))
        {
            if (objects_.emplace(object_id, std::move(participant)).second)
            {

                UXR_AGENT_LOG_DEBUG(
                    "client_key: 0x{0:08X}, object_id: 0x{1:04X}",
                    convertion::clientkey_to_raw(representation_.client_key()),
                    convertion::objectid_to_raw(object_id));
                rv = true;
            }
        }
    }
    return rv;
}

bool ProxyClient::create_topic(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::OBJK_TOPIC_Representation& representation)
{
    bool rv = false;
    if (dds::xrce::OBJK_PARTICIPANT == (representation.participant_id()[1] & 0x0F) &&
        dds::xrce::OBJK_TOPIC == (object_id[1] & 0x0F))
    {
        auto it = objects_.find(representation.participant_id());
        if (it != objects_.end())
        {
            std::shared_ptr<Participant> participant = std::dynamic_pointer_cast<Participant>(it->second);
            if (std::unique_ptr<Topic> topic = Topic::create(object_id, participant, representation))
            {
                if (objects_.emplace(object_id, std::move(topic)).second)
                {
                    UXR_AGENT_LOG_DEBUG(
                        "client_key: 0x{0:08X}, object_id: 0x{1:04X}",
                        convertion::clientkey_to_raw(representation_.client_key()),
                        convertion::objectid_to_raw(object_id));
                    rv = true;
                }
            }
        }
    }
    return rv;
}

bool ProxyClient::create_publisher(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::OBJK_PUBLISHER_Representation& representation)
{
    /* Check whether participant exists. */
    bool rv = false;
    if (dds::xrce::OBJK_PARTICIPANT == (representation.participant_id()[1] & 0x0F) &&
        dds::xrce::OBJK_PUBLISHER == (object_id.at(1) & 0x0F))
    {
        auto it = objects_.find(representation.participant_id());
        if (it != objects_.end())
        {
            std::shared_ptr<Participant> participant = std::dynamic_pointer_cast<Participant>(it->second);
            if (std::unique_ptr<Publisher> publisher = Publisher::create(object_id, participant, representation))
            {
                if (objects_.emplace(object_id, std::move(publisher)).second)
                {
                    UXR_AGENT_LOG_DEBUG(
                        "client_key: 0x{0:08X}, object_id: 0x{1:04X}",
                        convertion::clientkey_to_raw(representation_.client_key()),
                        convertion::objectid_to_raw(object_id));
                    rv = true;
                }
            }
        }
    }
    return rv;
}

bool ProxyClient::create_subscriber(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::OBJK_SUBSCRIBER_Representation& representation)
{
    /* Check whether participant exists. */
    bool rv = false;
    if (dds::xrce::OBJK_PARTICIPANT == (representation.participant_id()[1] & 0x0F) &&
        dds::xrce::OBJK_SUBSCRIBER == (object_id[1] & 0x0F))
    {
        auto it = objects_.find(representation.participant_id());
        if (it != objects_.end())
        {
            std::shared_ptr<Participant> participant = std::dynamic_pointer_cast<Participant>(it->second);
            if (std::unique_ptr<Subscriber> subscriber = Subscriber::create(object_id, participant, representation))
            {
                if (objects_.emplace(object_id, std::move(subscriber)).second)
                {
                    UXR_AGENT_LOG_DEBUG(
                        "client_key: 0x{0:08X}, object_id: 0x{1:04X}",
                        convertion::clientkey_to_raw(representation_.client_key()),
                        convertion::objectid_to_raw(object_id));
                    rv = true;
                }
            }
        }
    }
    return rv;
}

bool ProxyClient::create_datawriter(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::DATAWRITER_Representation& representation)
{
    bool rv = false;
    if (dds::xrce::OBJK_PUBLISHER == (representation.publisher_id()[1] & 0x0F) &&
        dds::xrce::OBJK_DATAWRITER == (object_id[1] & 0x0F))
    {
        auto it = objects_.find(representation.publisher_id());
        if (it != objects_.end())
        {
            std::shared_ptr<Publisher> publisher = std::dynamic_pointer_cast<Publisher>(it->second);
            if (std::unique_ptr<DataWriter> datawriter = DataWriter::create(object_id, publisher, representation, objects_))
            {
                if (objects_.emplace(object_id, std::move(datawriter)).second)
                {
                    UXR_AGENT_LOG_DEBUG(
                        "client_key: 0x{0:08X}, object_id: 0x{1:04X}",
                        convertion::clientkey_to_raw(representation_.client_key()),
                        convertion::objectid_to_raw(object_id));
                    rv = true;
                }
            }
        }
    }
    return rv;
}

bool ProxyClient::create_datareader(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::DATAREADER_Representation& representation)
{
    bool rv = false;
    if (dds::xrce::OBJK_SUBSCRIBER == (representation.subscriber_id()[1] & 0x0F) &&
        dds::xrce::OBJK_DATAREADER == (object_id[1] & 0x0F))
    {
        auto it = objects_.find(representation.subscriber_id());
        if (it != objects_.end())
        {
            std::shared_ptr<Subscriber> subscriber = std::dynamic_pointer_cast<Subscriber>(it->second);
            if (std::unique_ptr<DataReader> datareader = DataReader::create(object_id, subscriber, representation, objects_))
            {
                if (objects_.emplace(object_id, std::move(datareader)).second)
                {
                    UXR_AGENT_LOG_DEBUG(
                        "client_key: 0x{0:08X}, object_id: 0x{1:04X}",
                        convertion::clientkey_to_raw(representation_.client_key()),
                        convertion::objectid_to_raw(object_id));
                    rv = true;
                }
            }
        }
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima
