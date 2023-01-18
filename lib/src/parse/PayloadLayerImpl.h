//
// Copyright 2018 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "LayerImpl.h"

namespace commsdsl
{

namespace parse
{

class PayloadLayerImpl final : public LayerImpl
{
    using Base = LayerImpl;
public:
    PayloadLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol);

protected:
    virtual Kind kindImpl() const override;
    virtual bool verifyImpl(const LayersList& layers) override;
    virtual bool mustHaveFieldImpl() const override;

};

} // namespace parse

} // namespace commsdsl
