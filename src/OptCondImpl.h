#pragma once

#include <memory>
#include <vector>

#include "bbmp/OptCond.h"
#include "XmlWrap.h"
#include "Logger.h"
#include "FieldImpl.h"

namespace bbmp
{

class OptCondImpl
{
public:
    using Ptr = std::unique_ptr<OptCondImpl>;
    using Kind = OptCond::Kind;
    using FieldsList = std::vector<FieldImplPtr>;

    OptCondImpl() = default;
    OptCondImpl(const OptCondImpl&) = default;
    OptCondImpl(OptCondImpl&&) = default;

    Kind kind() const
    {
        return kindImpl();
    }

    Ptr clone() const
    {
        return cloneImpl();
    }

    bool verify(const FieldsList& fields)
    {
        return verifyImpl(fields);
    }

protected:
    virtual Kind kindImpl() const = 0;
    virtual Ptr cloneImpl() const = 0;
    virtual bool verifyImpl(const FieldsList& fields) = 0;
};

class OptCondExprImpl : public OptCondImpl
{
public:
    OptCondExprImpl() = default;
    OptCondExprImpl(const OptCondExprImpl&) = default;
    OptCondExprImpl(OptCondExprImpl&&) = default;

    bool parse(const std::string& expr);

protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual bool verifyImpl(const FieldsList& fields) override;

private:
    bool hasUpdatedValue();
    bool checkComparison(const std::string& expr, const std::string& op);
    bool checkBool(const std::string& expr);
    FieldImpl* findField(
        const FieldsList& fields,
        const std::string& name,
        std::size_t& remPos);
    bool verifyBitCheck(const FieldsList& fields);

    std::string m_left;
    std::string m_op;
    std::string m_right;
};

class OptCondListImpl : public OptCondImpl
{
    using Base = OptCondImpl;
public:
    using Type = OptCondList::Type;
    using List = std::vector<Ptr>;

    OptCondListImpl() = default;
    OptCondListImpl(const OptCondListImpl& other);
    OptCondListImpl(OptCondListImpl&&) = default;

    bool parse(::xmlNodePtr node, Logger& logger);

protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual bool verifyImpl(const FieldsList& fields) override;

private:
    List m_conds;
    Type m_type = Type::NumOfValues;
};

using OptCondImplPtr = OptCondImpl::Ptr;

} // namespace bbmp
