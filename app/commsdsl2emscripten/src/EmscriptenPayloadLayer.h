//
// Copyright 2022 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "EmscriptenLayer.h"

#include "commsdsl/gen/PayloadLayer.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2emscripten
{

class EmscriptenGenerator;
class EmscriptenPayloadLayer final : public commsdsl::gen::PayloadLayer, public EmscriptenLayer
{
    using Base = commsdsl::gen::PayloadLayer;
    using EmscriptenBase = EmscriptenLayer;
public:
    EmscriptenPayloadLayer(EmscriptenGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent);

protected:
    virtual std::string emscriptenHeaderFieldDefImpl() const override;
    virtual std::string emscriptenFieldClassNameImpl() const override;    
    virtual std::string emscriptenSourceFieldBindImpl() const override;
};

} // namespace commsdsl2emscripten
