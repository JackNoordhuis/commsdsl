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

#include <map>
#include <string>
#include <vector>

#include "commsdsl/CommsdslApi.h"

namespace commsdsl
{

namespace parse
{

class AliasImpl;
class COMMSDSL_API Alias
{
public:
    using AttributesMap = std::multimap<std::string, std::string>;
    using ElementsList = std::vector<std::string>;

    explicit Alias(const AliasImpl* impl);
    Alias(const Alias& other);
    ~Alias();

    const std::string& name() const;
    const std::string& description() const;
    const std::string& fieldName() const;

    const AttributesMap& extraAttributes() const;
    const ElementsList& extraElements() const;

protected:
    const AliasImpl* m_pImpl;
};

} // namespace parse

} // namespace commsdsl
