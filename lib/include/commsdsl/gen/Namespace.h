//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#pragma once

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/parse/Namespace.h"
#include "commsdsl/gen/Elem.h"
#include "commsdsl/gen/Field.h"
#include "commsdsl/gen/Frame.h"
#include "commsdsl/gen/Interface.h"
#include "commsdsl/gen/Message.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class NamespaceImpl;
class COMMSDSL_API Namespace : public Elem
{
    using Base = Elem;
public:
    using Ptr = std::unique_ptr<Namespace>;
    using NamespacesList = std::vector<Ptr>;
    using FieldsList = Field::FieldsList;
    using InterfacesList = std::vector<InterfacePtr>;
    using MessagesList = std::vector<MessagePtr>;
    using FramesList = std::vector<FramePtr>;
    using InterfacesAccessList = std::vector<const Interface*>;
    using MessagesAccessList = std::vector<const Message*>;
    using FramesAccessList = std::vector<const Frame*>;

    explicit Namespace(Generator& generator, commsdsl::parse::Namespace dslObj, Elem* parent = nullptr);
    virtual ~Namespace();

    bool createAll();
    bool prepare();
    bool write();

    commsdsl::parse::Namespace dslObj() const;

    const NamespacesList& namespaces() const;
    const FieldsList& fields() const;
    const InterfacesList& interfaces() const;
    const MessagesList& messages() const;
    const FramesList& frames() const;

    const Field* findMessageIdField() const;
    const Field* findField(const std::string& externalRef) const;

    InterfacesAccessList getAllInterfaces() const;
    MessagesAccessList getAllMessages() const;
    FramesAccessList getAllFrames() const;

    Generator& generator();

protected:    
    virtual Type elemTypeImpl() const override final;
    virtual bool writeImpl();

private:
    std::unique_ptr<NamespaceImpl> m_impl;
};

using NamespacePtr = Namespace::Ptr;

} // namespace gen

} // namespace commsdsl
