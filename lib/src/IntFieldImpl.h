//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
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

#include <cstdint>
#include <utility>
#include <vector>

#include "commsdsl/Endian.h"
#include "commsdsl/IntField.h"
#include "commsdsl/Units.h"
#include "FieldImpl.h"

namespace commsdsl
{

class IntFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:
    using Type = IntField::Type;

    using ValidRangeInfo = IntField::ValidRangeInfo;
    using ValidRangesList = IntField::ValidRangesList;
    using ScalingRatio = IntField::ScalingRatio;
    using SpecialValueInfo = IntField::SpecialValueInfo;
    using SpecialValues = IntField::SpecialValues;

    IntFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    IntFieldImpl(const IntFieldImpl&) = default;

    Type type() const
    {
        return m_state.m_type;
    }

    Endian endian() const
    {
        return m_state.m_endian;
    }

    std::intmax_t serOffset() const
    {
        return m_state.m_serOffset;
    }

    std::intmax_t minValue() const
    {
        return m_state.m_minValue;
    }

    std::intmax_t maxValue() const
    {
        return m_state.m_maxValue;
    }

    std::intmax_t defaultValue() const
    {
        return m_state.m_defaultValue;
    }

    ScalingRatio scaling() const
    {
        return m_state.m_scaling;
    }

    const ValidRangesList& validRanges() const
    {
        return m_state.m_validRanges;
    }

    const SpecialValues& specialValues() const
    {
        return m_state.m_specials;
    }

    bool validCheckVersion() const
    {
        return m_state.m_validCheckVersion;
    }

    Units units() const
    {
        return m_state.m_units;
    }

    unsigned displayDecimals() const
    {
        return m_state.m_displayDecimals;
    }

    std::intmax_t displayOffset() const
    {
        return m_state.m_displayOffset;
    }

    bool signExt() const
    {
        return m_state.m_signExt;
    }

    static Type parseTypeValue(const std::string& value);

    static std::size_t maxTypeLength(Type t);
    static std::intmax_t minTypeValue(Type t);
    static std::intmax_t maxTypeValue(Type t);
    static std::intmax_t calcMinValue(Type t, std::size_t bitsLen);
    static std::intmax_t calcMaxValue(Type t, std::size_t bitsLen);
    static bool isUnsigned(Type t);
    static bool isBigUnsigned(Type t)
    {
        return (t == Type::Uint64) || (t == Type::Uintvar);
    }

    static bool isTypeUnsigned(IntFieldImpl::Type t);

protected:
    virtual Kind kindImpl() const override final;
    virtual Ptr cloneImpl() const override final;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override final;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override final;
    virtual bool reuseImpl(const FieldImpl& other) override final;
    virtual bool parseImpl() override final;
    virtual std::size_t minLengthImpl() const override final;
    virtual std::size_t maxLengthImpl() const override final;
    virtual std::size_t bitLengthImpl() const override final;
    virtual bool isComparableToValueImpl(const std::string& val) const override final;
    virtual bool isComparableToFieldImpl(const FieldImpl& field) const override final;
    virtual bool strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const override final;
    virtual bool strToFpImpl(const std::string& ref, double& val) const override final;

private:
    bool updateType();
    bool updateEndian();
    bool updateLength();
    bool updateBitLength();
    bool updateSerOffset();
    bool updateMinMaxValues();
    bool updateDefaultValue();
    bool updateScaling();
    bool updateValidCheckVersion();
    bool updateValidRanges();
    bool updateSpecials();
    bool updateUnits();
    bool updateDisplayDecimals();
    bool updateDisplayOffset();
    bool updateSignExt();
    bool checkValidRangeAsAttr(const PropsMap& xmlAttrs);
    bool checkValidRangeAsChild(::xmlNodePtr child);
    bool checkValidRangeProps(const PropsMap& xmlAttrs);
    bool checkValidValueAsAttr(const PropsMap& xmlAttrs);
    bool checkValidValueAsChild(::xmlNodePtr child);
    bool checkValidValueProps(const PropsMap& xmlAttrs);
    bool checkValidMinAsAttr(const PropsMap& xmlAttrs);
    bool checkValidMinAsChild(::xmlNodePtr child);
    bool checkValidMinProps(const PropsMap& xmlAttrs);
    bool checkValidMaxAsAttr(const PropsMap& xmlAttrs);
    bool checkValidMaxAsChild(::xmlNodePtr child);
    bool checkValidMaxProps(const PropsMap& xmlAttrs);
    bool validateValidRangeStr(const std::string& str, std::intmax_t& minVal, std::intmax_t& maxVal);
    bool validateValidValueStr(const std::string& str, const std::string& type, std::intmax_t& val);
    bool strToValue(const std::string& str, std::intmax_t& val) const;

    struct State
    {
        Type m_type = Type::NumOfValues;
        Endian m_endian = Endian_NumOfValues;
        std::size_t m_length = 0U;
        std::size_t m_bitLength = 0U;
        std::intmax_t m_serOffset = 0;
        std::intmax_t m_typeAllowedMinValue = 0;
        std::intmax_t m_typeAllowedMaxValue = 0;
        std::intmax_t m_minValue = 0;
        std::intmax_t m_maxValue = 0;
        std::intmax_t m_defaultValue = 0;
        ScalingRatio m_scaling;
        ValidRangesList m_validRanges;
        SpecialValues m_specials;
        Units m_units = Units::Unknown;
        unsigned m_displayDecimals = 0U;
        std::intmax_t m_displayOffset = 0U;
        bool m_validCheckVersion = false;
        bool m_signExt = true;
    };

    State m_state;
};

} // namespace commsdsl
