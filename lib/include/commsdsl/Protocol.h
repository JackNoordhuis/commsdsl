#pragma once

#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <limits>

#include "CommsdslApi.h"
#include "ErrorLevel.h"
#include "Schema.h"
#include "Namespace.h"
#include "Field.h"

namespace commsdsl
{

class ProtocolImpl;
class COMMSDSL_API Protocol
{
public:
    using ErrorReportFunction = std::function<void (ErrorLevel, const std::string&)>;
    using NamespacesList = std::vector<Namespace>;
    using MessagesList = Namespace::MessagesList;
    using PlatformsList = Message::PlatformsList;

    Protocol();
    ~Protocol();

    void setErrorReportCallback(ErrorReportFunction&& cb);

    bool parse(const std::string& input);
    bool validate();

    Schema schema() const;
    NamespacesList namespaces() const;

    static constexpr unsigned notYetDeprecated()
    {
        return std::numeric_limits<unsigned>::max();
    }

    Field findField(const std::string& externalRef) const;

    MessagesList allMessages() const;

    void addExpectedExtraPrefix(const std::string& value);

    const PlatformsList& platforms() const;

private:
    std::unique_ptr<ProtocolImpl> m_pImpl;
};

} // namespace commsdsl
