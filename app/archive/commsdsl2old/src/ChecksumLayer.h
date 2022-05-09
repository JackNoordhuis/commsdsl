//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/Layer.h"

#include "Layer.h"
#include "common.h"

namespace commsdsl2old
{

class ChecksumLayer final : public Layer
{
    using Base = Layer;
public:
    ChecksumLayer(Generator& generator, commsdsl::parse::Layer layer) : Base(generator, layer) {}

protected:
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        std::string& prevLayer,
        bool& hasInputMessages) const override;
    virtual bool rearangeImpl(LayersList& layers, bool& success) override;

private:
    commsdsl::parse::ChecksumLayer checksumLayerDslObj() const
    {
        return commsdsl::parse::ChecksumLayer(dslObj());
    }

    std::string getAlg() const;

    bool m_rearanged = false;
};

inline
LayerPtr createChecksumLayer(Generator& generator, commsdsl::parse::Layer layer)
{
    return std::make_unique<ChecksumLayer>(generator, layer);
}

} // namespace commsdsl2old