// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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


#include <uxr/agent/object/XRCEObject.hpp>

namespace eprosima {
namespace uxr {

XRCEObject::~XRCEObject() {}

dds::xrce::ObjectId XRCEObject::get_id() const
{
    return id_;
}

uint16_t XRCEObject::get_raw_id() const
{
    return uint16_t((id_[0] << 8) + id_[1]);
}

} // namespace uxr
} // namespace eprosima

