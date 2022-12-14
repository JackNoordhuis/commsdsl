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

#include "EmscriptenField.h"

// #include "EmscriptenComms.h"
#include "EmscriptenDataBuf.h"
#include "EmscriptenGenerator.h"
// #include "EmscriptenOptionalField.h"
#include "EmscriptenProtocolOptions.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <algorithm>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;


namespace commsdsl2emscripten
{

EmscriptenField::EmscriptenField(commsdsl::gen::Field& field) :
    m_field(field)
{
}

EmscriptenField::~EmscriptenField() = default;

const EmscriptenField* EmscriptenField::cast(const commsdsl::gen::Field* field)
{
    if (field == nullptr) {
        return nullptr;
    }

    auto* emscriptenField = dynamic_cast<const EmscriptenField*>(field);    
    assert(emscriptenField != nullptr);
    return emscriptenField;
}

EmscriptenField::EmscriptenFieldsList EmscriptenField::emscriptenTransformFieldsList(const commsdsl::gen::Field::FieldsList& fields)
{
    EmscriptenFieldsList result;
    result.reserve(fields.size());
    for (auto& fPtr : fields) {
        assert(fPtr);

        auto* emscriptenField = 
            const_cast<EmscriptenField*>(
                dynamic_cast<const EmscriptenField*>(fPtr.get()));

        assert(emscriptenField != nullptr);
        result.push_back(emscriptenField);
    }

    return result;    
}

std::string EmscriptenField::emscriptenRelHeaderPath() const
{
    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    return generator.emscriptenRelHeaderFor(m_field);
}

bool EmscriptenField::emscriptenIsVersionOptional() const
{
    return comms::isVersionOptionaField(m_field, m_field.generator());
}

bool EmscriptenField::emscriptenWrite() const
{
    if (!comms::isGlobalField(m_field)) {
        // Skip write for non-global fields,
        // The code generation will be driven by other means        
        return true;
    }

    if (!m_field.isReferenced()) {
        // Code for not referenced does not exist
        return true;
    }

    return 
        emscriptenWriteHeaderInternal() &&
        emscriptenWriteSrcInternal();
}

std::string EmscriptenField::emscriptenHeaderClass() const
{
    static const std::string Templ = 
        "#^#MEMBERS#$#\n"
        "#^#DEF#$#\n"
    ;

    util::ReplacementMap repl = {
        {"MEMBERS", emscriptenHeaderMembersInternal()},
        {"DEF", emscriptenHeaderClassInternal()},
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenField::emscriptenTemplateScope() const
{
    auto& gen = EmscriptenGenerator::cast(m_field.generator());
    auto commsScope = comms::scopeFor(m_field, gen);
    std::string optionsParams = "<" + EmscriptenProtocolOptions::emscriptenClassName(gen) + ">";

    if (comms::isGlobalField(m_field)) {
        return commsScope + optionsParams;
    }

    using Elem = commsdsl::gen::Elem;

    auto formScopeFunc = 
        [&commsScope, &gen, &optionsParams](const Elem* parent, const std::string& suffix)
        {
            auto optLevelScope = comms::scopeFor(*parent, gen) + suffix;
            assert(optLevelScope.size() < commsScope.size());
            assert(std::equal(optLevelScope.begin(), optLevelScope.end(), commsScope.begin()));
            
            return optLevelScope + optionsParams + commsScope.substr(optLevelScope.size());
        };

    
    Elem* parent = m_field.getParent();
    while (parent != nullptr)  {
        auto elemType = parent->elemType();

        if (elemType == Elem::Type_Interface) {
            return commsScope;
        }        

        if ((elemType == Elem::Type_Field) && (comms::isGlobalField(*parent))) {
            return formScopeFunc(parent, strings::membersSuffixStr());
        }        

        if (elemType == Elem::Type_Message) {
            return formScopeFunc(parent, strings::fieldsSuffixStr());
        }

        if (elemType == Elem::Type_Frame) {
            return formScopeFunc(parent, strings::layersSuffixStr());
        }        

        parent = parent->getParent();
    }

    assert(false); // Should not happen
    return commsScope;
}

std::string EmscriptenField::emscriptenSourceCode() const
{
    static const std::string Templ = 
        "#^#MEMBERS#$#\n"
        "#^#VALUE_ACC#$#\n"
        "#^#EXTRA#$#\n"
        "#^#COMMON#$#\n"
        "#^#BIND#$#\n";       

    util::ReplacementMap repl = {
        {"MEMBERS", emscriptenSourceMembersInternal()},
        {"VALUE_ACC", emscriptenSourceValueAccImpl()},
        {"EXTRA", emscriptenSourceExtraPublicFuncsImpl()},
        {"COMMON", emscriptenSourceCommonPublicFuncsInternal()},
        {"BIND", emscriptenSourceBindInternal()},
    };

    return util::processTemplate(Templ, repl);
}

void EmscriptenField::emscriptenHeaderAddExtraIncludes(StringsList& incs) const
{
    emscriptenHeaderAddExtraIncludesImpl(incs);
}

void EmscriptenField::emscriptenHeaderAddExtraIncludesImpl(StringsList& incs) const
{
    static_cast<void>(incs);
}

std::string EmscriptenField::emscriptenHeaderValueAccImpl() const
{
    static const std::string Templ = 
        "const ValueType& getValue() const;\n"
        "void setValue(const ValueType& val) const;\n";

    return Templ;
}

std::string EmscriptenField::emscriptenHeaderExtraPublicFuncsImpl() const
{
    return strings::emptyString();
}

std::string EmscriptenField::emscriptenSourceValueAccImpl() const
{
    static const std::string Templ = 
        "const #^#CLASS_NAME#$#::ValueType& #^#CLASS_NAME#$#::getValue() const\n"
        "{\n"
        "    return Base::getValue();\n"
        "}\n\n"
        "void #^#CLASS_NAME#$#::setValue(const ValueType& val) const\n"
        "{\n"
        "    Base::setValue(val);\n"
        "}\n";

    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", generator.emscriptenClassName(m_field)}
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenField::emscriptenSourceExtraPublicFuncsImpl() const
{
    return strings::emptyString();
}

std::string EmscriptenField::emscriptenSourceBindValueAccImpl() const
{
    static const std::string Templ = 
        ".function(\"getValue\", &#^#CLASS_NAME#$#::getValue)\n"
        ".function(\"setValue\", &#^#CLASS_NAME#$#::setValue)";

    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", generator.emscriptenClassName(m_field)}
    };

    if (emscriptenIsVersionOptional()) {
        repl["CLASS_NAME"].append(strings::versionOptionalFieldSuffixStr());
    }

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenField::emscriptenSourceBindFuncsImpl() const
{
    return strings::emptyString();
}

std::string EmscriptenField::emscriptenSourceBindExtraImpl() const
{
    return strings::emptyString();
}

void EmscriptenField::emscriptenAssignMembers(const commsdsl::gen::Field::FieldsList& fields)
{
    m_members = emscriptenTransformFieldsList(fields);
}

bool EmscriptenField::emscriptenWriteHeaderInternal() const
{
    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    auto filePath = generator.emscriptenAbsHeaderFor(m_field);
    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!generator.createDirectory(dirPath)) {
        return false;
    }       

    auto& logger = generator.logger();
    logger.info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "#^#CLASS#$#\n"
        "#^#APPEND#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"INCLUDES", emscriptenHeaderIncludesInternal()},
        {"CLASS", emscriptenHeaderClass()},
        {"APPEND", util::readFileContents(generator.emspriptenInputAbsHeaderFor(m_field) + strings::appendFileSuffixStr())}
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good(); 
}

bool EmscriptenField::emscriptenWriteSrcInternal() const
{
    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    auto filePath = generator.emscriptenAbsSourceFor(m_field);
    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!generator.createDirectory(dirPath)) {
        return false;
    }       

    auto& logger = generator.logger();
    logger.info("Generating " + filePath);

    std::ofstream stream(filePath);
    if (!stream) {
        logger.error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#^#INCLUDES#$#\n"
        "#^#CODE#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"INCLUDES", emscriptenSourceIncludesInternal()},
        {"CODE", emscriptenSourceCode()},
    };
    
    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    return stream.good(); 
}

std::string EmscriptenField::emscriptenHeaderIncludesInternal() const
{
    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    StringsList includes = {
        comms::relHeaderPathFor(m_field, generator),
        EmscriptenDataBuf::emscriptenRelHeader(generator)
    };

    EmscriptenProtocolOptions::emscriptenAddInclude(generator, includes);

    for (auto* m : m_members) {
        m->emscriptenHeaderAddExtraIncludes(includes);
    }

    comms::prepareIncludeStatement(includes);
    auto result = util::strListToString(includes, "\n", "\n");
    result.append(util::readFileContents(generator.emspriptenInputAbsHeaderFor(m_field) + strings::incFileSuffixStr()));
    return result;
}

std::string EmscriptenField::emscriptenHeaderClassInternal() const
{
    auto& generator = EmscriptenGenerator::cast(m_field.generator());

    std::string publicCode;
    std::string protectedCode;
    std::string privateCode;
    if (comms::isGlobalField(m_field)) {
        auto inputCodePrefix = generator.emspriptenInputAbsHeaderFor(m_field);
        publicCode = util::readFileContents(inputCodePrefix + strings::publicFileSuffixStr());
        protectedCode = util::readFileContents(inputCodePrefix + strings::protectedFileSuffixStr());
        privateCode = util::readFileContents(inputCodePrefix + strings::privateFileSuffixStr());
    }

    if (!protectedCode.empty()) {
        static const std::string TemplTmp = 
            "protected:\n"
            "    #^#CODE#$#\n";

        util::ReplacementMap replTmp = {
            {"CODE", std::move(protectedCode)}
        };

        protectedCode = util::processTemplate(TemplTmp, replTmp);
    }

    if (!privateCode.empty()) {
        static const std::string TemplTmp = 
            "private:\n"
            "    #^#CODE#$#\n";

        util::ReplacementMap replTmp = {
            {"CODE", std::move(privateCode)}
        };

        privateCode = util::processTemplate(TemplTmp, replTmp);
    }    

    static const std::string Templ = 
        "class #^#CLASS_NAME#$##^#SUFFIX#$# : public #^#COMMS_CLASS#$##^#SUFFIX#$#\n"
        "{\n"
        "    using Base = #^#COMMS_CLASS#$##^#SUFFIX#$#;\n"
        "public:\n"
        "    #^#CLASS_NAME#$##^#SUFFIX#$#() = default;\n"
        "    #^#CLASS_NAME#$##^#SUFFIX#$#(const #^#CLASS_NAME#$##^#SUFFIX#$#&) = default;\n"
        "    ~#^#CLASS_NAME#$##^#SUFFIX#$#() = default;\n\n"
        "    #^#VALUE_ACC#$#\n"
        "    #^#EXTRA#$#\n"
        "    #^#COMMON#$#\n"
        "    #^#PUBLIC#$#\n"
        "#^#PROTECTED#$#\n"
        "#^#PRIVATE#$#\n"
        "};\n";

    util::ReplacementMap repl = {
        {"COMMS_CLASS", emscriptenTemplateScope()},
        {"CLASS_NAME", generator.emscriptenClassName(m_field)},
        {"COMMON", emscriptenHeaderCommonPublicFuncsInternal()},
        {"VALUE_ACC", emscriptenHeaderValueAccImpl()},
        {"EXTRA", emscriptenHeaderExtraPublicFuncsImpl()},
        {"PUBLIC", std::move(publicCode)},
        {"PROTECTED", std::move(protectedCode)},
        {"PRIVATE", std::move(privateCode)}
    };

    if (!emscriptenIsVersionOptional()) {
        return util::processTemplate(Templ, repl);
    }

    repl["SUFFIX"] = strings::versionOptionalFieldSuffixStr();
    repl["FIELD"] = util::processTemplate(Templ, repl);

    static const std::string OptTempl = 
        "#^#FIELD#$#\n"
        "class #^#CLASS_NAME#$# : public #^#COMMS_CLASS#$#\n"
        "{\n"
        "    using Base = #^#COMMS_CLASS#$#;\n"
        "public:\n"
        "    #^#CLASS_NAME#$##^#SUFFIX#$#* field();\n\n"
        "    #^#COMMON#$#\n"
        "};\n";


    return util::processTemplate(OptTempl, repl);
}

std::string EmscriptenField::emscriptenHeaderCommonPublicFuncsInternal() const
{
    static const std::string Templ = 
        "comms::ErrorStatus read(const #^#DATA_BUF#$#& buf);\n"
        "comms::ErrorStatus write(#^#DATA_BUF#$#& buf) const;\n"
        "bool refresh();\n"
        "std::size_t length();\n"
        "bool valid() const;\n"
        "const char* name() const;"
        ;

    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    util::ReplacementMap repl = {
        {"DATA_BUF", EmscriptenDataBuf::emscriptenClassName(generator)}
    };
    
    return util::processTemplate(Templ, repl);
}

std::string EmscriptenField::emscriptenSourceIncludesInternal() const
{
    StringsList includes = {
        "<emscripten/bind.h>",
        emscriptenRelHeaderPath()
    };

    comms::prepareIncludeStatement(includes);
    return util::strListToString(includes, "\n", "\n");
}

std::string EmscriptenField::emscriptenSourceCommonPublicFuncsInternal() const
{
    static const std::string Templ = 
        "comms::ErrorStatus #^#CLASS_NAME#$##^#SUFFIX#$#::read(const #^#DATA_BUF#$#& buf)\n"
        "{\n"
        "    return Base::read(buf.begin(), buf.size());\n"
        "}\n\n"
        "comms::ErrorStatus #^#CLASS_NAME#$##^#SUFFIX#$#::write(#^#DATA_BUF#$#& buf) const\n"
        "{\n"
        "    return Base::write(buf.begin(), buf.size());\n"
        "}\n\n"
        "bool #^#CLASS_NAME#$##^#SUFFIX#$#::refresh()\n"
        "{\n"
        "    return Base::refresh();\n"
        "}\n\n"
        "std::size_t #^#CLASS_NAME#$##^#SUFFIX#$#::length()\n"
        "{\n"
        "    return Base::length();\n"
        "}\n\n"
        "bool #^#CLASS_NAME#$##^#SUFFIX#$#::valid() const\n"
        "{\n"
        "    return Base::valid();\n"
        "}\n\n"
        "const char* #^#CLASS_NAME#$##^#SUFFIX#$#::name() const"
        "{\n"
        "    return Base::name();\n"
        "}\n\n"        
        ;

    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", generator.emscriptenClassName(m_field)},
        {"DATA_BUF", EmscriptenDataBuf::emscriptenClassName(generator)}
    };

    if (!emscriptenIsVersionOptional()) {
        return util::processTemplate(Templ, repl);
    }    


    static const std::string OptTempl = 
        "#^#FIELD#$#\n"
        "#^#CLASS_NAME#$##^#SUFFIX#$#* #^#CLASS_NAME#$#::field()\n"
        "{\n"
        "    return static_cast<#^#CLASS_NAME#$##^#SUFFIX#$#*>(&(Base::field()));\n"
        "}\n\n"
        "#^#COMMON#$#\n";

    util::ReplacementMap optRepl = {
        {"CLASS_NAME", generator.emscriptenClassName(m_field)},
        {"COMMON", util::processTemplate(Templ, repl)},
        {"SUFFIX", strings::versionOptionalFieldSuffixStr()}
    };

    repl["SUFFIX"] = strings::versionOptionalFieldSuffixStr();
    optRepl["FIELD"] = util::processTemplate(Templ, repl);
    
    return util::processTemplate(OptTempl, optRepl);
}

std::string EmscriptenField::emscriptenSourceBindInternal() const
{
    static const std::string Templ = 
        "EMSCRIPTEN_BINDINGS(#^#CLASS_NAME#$##^#SUFFIX#$#) {\n"
        "    emscripten::class_<#^#CLASS_NAME#$##^#SUFFIX#$#>(\"#^#CLASS_NAME#$##^#SUFFIX#$#\")\n"
        "        .constructor<>()\n"
        "        .constructor<const #^#CLASS_NAME#$##^#SUFFIX#$#&>()\n"
        "        #^#VALUE_ACC#$#\n"
        "        #^#FUNCS#$#\n"
        "        #^#COMMON#$#\n"
        "        #^#CUSTOM#$#\n"
        "        ;\n"
        "    #^#EXTRA#$#\n"
        "}\n"
        ;

    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", generator.emscriptenClassName(m_field)},
        {"VALUE_ACC", emscriptenSourceBindValueAccImpl()},
        {"EXTRA", emscriptenSourceBindFuncsImpl()},
        {"COMMON", emscriptenSourceBindCommonInternal()},
        {"CUSTOM", util::readFileContents(generator.emspriptenInputAbsSourceFor(m_field) + strings::bindFileSuffixStr())},
        {"EXTRA", emscriptenSourceBindExtraImpl()},
    };

    if (!emscriptenIsVersionOptional()) {
        return util::processTemplate(Templ, repl);
    }

    repl["SUFFIX"] = strings::versionOptionalFieldSuffixStr();

    static const std::string OptTempl = 
        "#^#FIELD#$#\n"
        "EMSCRIPTEN_BINDINGS(#^#CLASS_NAME#$#) {\n"
        "    emscripten::class_<#^#CLASS_NAME#$#>(\"#^#CLASS_NAME#$#\")\n"
        "        .constructor<>()\n"
        "        .constructor<const #^#CLASS_NAME#$#&>()\n"
        "        .function(\"field\", &#^#CLASS_NAME#$#::field, emscripten::allow_raw_pointers())\n"
        "        #^#COMMON#$#\n"
        "        ;\n"
        "}\n"; 

    util::ReplacementMap optRepl = {
        {"CLASS_NAME", generator.emscriptenClassName(m_field)},
        {"COMMON", emscriptenSourceBindCommonInternal(true)},
        {"FIELD", util::processTemplate(Templ, repl)}
    };

    return util::processTemplate(OptTempl, optRepl);
}

std::string EmscriptenField::emscriptenSourceBindCommonInternal(bool skipVersionOptCheck) const
{
    static const std::string Templ = 
        ".function(\"read\", &#^#CLASS_NAME#$#::read)\n"
        ".function(\"write\", &#^#CLASS_NAME#$#::write)\n"
        ".function(\"refresh\", &#^#CLASS_NAME#$#::refresh)\n"
        ".function(\"length\", &#^#CLASS_NAME#$#::length)\n"
        ".function(\"valid\", &#^#CLASS_NAME#$#::valid)"
        ;

    auto& generator = EmscriptenGenerator::cast(m_field.generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", generator.emscriptenClassName(m_field)},
    };

    if ((!skipVersionOptCheck) && (emscriptenIsVersionOptional())) {
        repl["CLASS_NAME"].append(strings::versionOptionalFieldSuffixStr());
    }

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenField::emscriptenHeaderMembersInternal() const
{
    util::StringsList members;
    for (auto* m : m_members) {
        auto str = m->emscriptenHeaderClass();
        if (!str.empty()) {
            members.push_back(std::move(str));
        }
    }

    return util::strListToString(members, "\n", "");
}

std::string EmscriptenField::emscriptenSourceMembersInternal() const
{
    util::StringsList members;
    for (auto* m : m_members) {
        auto str = m->emscriptenSourceCode();
        if (!str.empty()) {
            members.push_back(std::move(str));
        }
    }

    return util::strListToString(members, "\n", "");    
}

} // namespace commsdsl2emscripten
