//
// Copyright 2019 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "ToolsQtField.h"

#include "commsdsl/gen/RefField.h"

namespace commsdsl2tools_qt
{

class ToolsQtGenerator;
class ToolsQtRefField final : public commsdsl::gen::RefField, public ToolsQtField
{
    using Base = commsdsl::gen::RefField;
    using ToolsBase = ToolsQtField;
public:
    explicit ToolsQtRefField(ToolsQtGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

protected:
    // Base overrides
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

    // ToolsBase overrides
    virtual IncludesList toolsExtraSrcIncludesImpl() const override;
    virtual std::string toolsDefFuncBodyImpl() const override;

private:
    std::string toolsExtraPropsInternal() const;
    
    ToolsQtField* m_toolsReferenceField = nullptr;
};

} // namespace commsdsl2tools_qt
