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

#include "SwigBundleField.h"

#include "SwigGenerator.h"

namespace commsdsl2swig
{

SwigBundleField::SwigBundleField(SwigGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

bool SwigBundleField::prepareImpl()
{
    return 
        Base::prepareImpl() &&
        prepareInternal();
}

bool SwigBundleField::writeImpl() const
{
    return swigWrite();
}

bool SwigBundleField::prepareInternal()
{
    m_members = swigTransformFieldsList(members());
    return true;
}


} // namespace commsdsl2swig
