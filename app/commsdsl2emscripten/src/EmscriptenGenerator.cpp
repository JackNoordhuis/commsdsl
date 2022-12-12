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

#include "EmscriptenGenerator.h"

// #include "Emscripten.h"
#include "EmscriptenBitfieldField.h"
#include "EmscriptenBundleField.h"
// #include "EmscriptenChecksumLayer.h"
// #include "EmscriptenCmake.h"
#include "EmscriptenComms.h"
// #include "EmscriptenCustomLayer.h"
#include "EmscriptenDataBuf.h"
#include "EmscriptenDataField.h"
#include "EmscriptenEnumField.h"
#include "EmscriptenFloatField.h"
// #include "EmscriptenFrame.h"
// #include "EmscriptenIdLayer.h"
#include "EmscriptenInterface.h"
#include "EmscriptenIntField.h"
#include "EmscriptenListField.h"
#include "EmscriptenMessage.h"
// #include "EmscriptenMsgHandler.h"
#include "EmscriptenMsgId.h"
// #include "EmscriptenNamespace.h"
#include "EmscriptenOptionalField.h"
// #include "EmscriptenPayloadLayer.h"
#include "EmscriptenProtocolOptions.h"
#include "EmscriptenRefField.h"
// #include "EmscriptenSchema.h"
#include "EmscriptenSetField.h"
// #include "EmscriptenSizeLayer.h"
#include "EmscriptenStringField.h"
// #include "EmscriptenSyncLayer.h"
// #include "EmscriptenValueLayer.h"
#include "EmscriptenVariantField.h"
#include "EmscriptenVersion.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/version.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <filesystem>
#include <map>

namespace comms = commsdsl::gen::comms;
namespace fs = std::filesystem;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2emscripten
{

EmscriptenGenerator::EmscriptenGenerator()
{
    Base::setAllInterfacesReferencedByDefault(false);
    Base::setAllMessagesReferencedByDefault(false);
}    

const std::string& EmscriptenGenerator::fileGeneratedComment()
{
    static const std::string Str =
        "// Generated by commsdsl2emscripten v" + std::to_string(commsdsl::versionMajor()) +
        '.' + std::to_string(commsdsl::versionMinor()) + '.' +
        std::to_string(commsdsl::versionPatch()) + '\n';
    return Str;
}

// std::string EmscriptenGenerator::emscriptenInputCodePathForFile(const std::string& name) const
// {
//     return getCodeDir() + '/' + name;
// }

std::string EmscriptenGenerator::emscriptenClassName(const Elem& elem) const
{
    bool addMainNamespace = m_mainNamespaceInNamesForced || (schemas().size() > 1U); 
    auto str = comms::scopeFor(elem, *this, addMainNamespace);
    return emscriptenScopeToName(str);
}

std::string EmscriptenGenerator::emscriptenScopeNameForRoot(const std::string& name) const
{
    bool addMainNamespace = m_mainNamespaceInNamesForced || (schemas().size() > 1U); 
    auto str = comms::scopeForRoot(name, *this, addMainNamespace);
    return emscriptenScopeToName(str);
}

std::string EmscriptenGenerator::emscriptenProtocolClassNameForRoot(const std::string& name) const
{
    bool addMainNamespace = m_mainNamespaceInNamesForced || (schemas().size() > 1U); 
    auto schemaIdx = currentSchemaIdx();
    auto* thisGen = const_cast<EmscriptenGenerator*>(this);
    thisGen->chooseProtocolSchema();
    auto str = comms::scopeForRoot(name, *this, addMainNamespace);
    thisGen->chooseCurrentSchema(schemaIdx);
    return emscriptenScopeToName(str);
}

std::string EmscriptenGenerator::emscriptenScopeToName(const std::string& scope)
{
    return util::strReplace(scope, "::", "_");
}

bool EmscriptenGenerator::createCompleteImpl()
{
    return 
        emscriptenReferenceRequestedInterfaceInternal() &&
        emscriptenReferenceRequestedMessagesInternal();
}

bool EmscriptenGenerator::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    bool result = emscriptenPrepareDefaultInterfaceInternal();

    if (!result) {
        return false;
    }

    if (m_forcedInterface.empty()) {
        return true;
    }
    
    auto* iFace = findInterface(m_forcedInterface);
    if (iFace == nullptr) {
        logger().error("The selected forced interface \"" + m_forcedInterface + "\" hasn't been found");
        return false;
    }

    return true;
}

bool EmscriptenGenerator::writeImpl()
{
    for (auto idx = 0U; idx < schemas().size(); ++idx) {
        chooseCurrentSchema(idx);
        bool result = 
            EmscriptenMsgId::emscriptenWrite(*this) &&
            EmscriptenVersion::emscriptenWrite(*this);

        if (!result) {
            return false;
        }
    }

    return 
        EmscriptenComms::emscriptenWrite(*this) &&
        EmscriptenDataBuf::emscriptenWrite(*this) &&
        EmscriptenProtocolOptions::emscriptenWrite(*this) &&
        // EmscriptenMsgHandler::emscriptenWrite(*this) &&
        // EmscriptenCmake::emscriptenWrite(*this) &&
        emscriptenWriteExtraFilesInternal();

}

void EmscriptenGenerator::emscriptenSetMainNamespaceInNamesForced(bool value)
{
    m_mainNamespaceInNamesForced = value;
}

void EmscriptenGenerator::emscriptenSetForcedInterface(const std::string& value)
{
    m_forcedInterface = value;
}

void EmscriptenGenerator::emscriptenSetHasProtocolVersion(bool value)
{
    m_hasProtocolVersion = value;
}

void EmscriptenGenerator::emscriptenSetMessagesListFile(const std::string& value)
{
    m_messagesListFile = value;
}

void EmscriptenGenerator::emscriptenSetForcedPlatform(const std::string& value)
{
    m_forcedPlatform = value;
}

bool EmscriptenGenerator::emscriptenHasProtocolVersion() const
{
    return m_hasProtocolVersion;
}

const EmscriptenInterface* EmscriptenGenerator::emscriptenMainInterface() const
{
    do {
        if (m_forcedInterface.empty()) {
            break;
        }

        auto iFace = findInterface(m_forcedInterface);
        if (iFace == nullptr) {
            break;
        }

        return static_cast<const EmscriptenInterface*>(iFace);
    } while (false);

    auto allInterfaces = getAllInterfaces();
    if (allInterfaces.empty()) {
        return nullptr;
    }
    return static_cast<const EmscriptenInterface*>(allInterfaces.front());
}

EmscriptenInterface* EmscriptenGenerator::emscriptenMainInterface()
{
    return const_cast<EmscriptenInterface*>(static_cast<const EmscriptenGenerator*>(this)->emscriptenMainInterface());
}

// EmscriptenGenerator::SchemaPtr EmscriptenGenerator::createSchemaImpl(commsdsl::parse::Schema dslObj, Elem* parent)
// {
//     return std::make_unique<EmscriptenSchema>(*this, dslObj, parent);
// }

// EmscriptenGenerator::NamespacePtr EmscriptenGenerator::createNamespaceImpl(commsdsl::parse::Namespace dslObj, Elem* parent)
// {
//     return std::make_unique<EmscriptenNamespace>(*this, dslObj, parent);
// }

EmscriptenGenerator::InterfacePtr EmscriptenGenerator::createInterfaceImpl(commsdsl::parse::Interface dslObj, Elem* parent)
{
    return std::make_unique<EmscriptenInterface>(*this, dslObj, parent);
}

EmscriptenGenerator::MessagePtr EmscriptenGenerator::createMessageImpl(commsdsl::parse::Message dslObj, Elem* parent)
{
    return std::make_unique<EmscriptenMessage>(*this, dslObj, parent);
}

// EmscriptenGenerator::FramePtr EmscriptenGenerator::createFrameImpl(commsdsl::parse::Frame dslObj, Elem* parent)
// {
//     return std::make_unique<EmscriptenFrame>(*this, dslObj, parent);
// }

EmscriptenGenerator::FieldPtr EmscriptenGenerator::createIntFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<EmscriptenIntField>(*this, dslObj, parent);
}

EmscriptenGenerator::FieldPtr EmscriptenGenerator::createEnumFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<EmscriptenEnumField>(*this, dslObj, parent);
}

EmscriptenGenerator::FieldPtr EmscriptenGenerator::createSetFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<EmscriptenSetField>(*this, dslObj, parent);
}

EmscriptenGenerator::FieldPtr EmscriptenGenerator::createFloatFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<EmscriptenFloatField>(*this, dslObj, parent);
}

EmscriptenGenerator::FieldPtr EmscriptenGenerator::createBitfieldFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<EmscriptenBitfieldField>(*this, dslObj, parent);
}

EmscriptenGenerator::FieldPtr EmscriptenGenerator::createBundleFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<EmscriptenBundleField>(*this, dslObj, parent);
}

EmscriptenGenerator::FieldPtr EmscriptenGenerator::createStringFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<EmscriptenStringField>(*this, dslObj, parent);
}

EmscriptenGenerator::FieldPtr EmscriptenGenerator::createDataFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<EmscriptenDataField>(*this, dslObj, parent);
}

EmscriptenGenerator::FieldPtr EmscriptenGenerator::createListFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<EmscriptenListField>(*this, dslObj, parent);
}

EmscriptenGenerator::FieldPtr EmscriptenGenerator::createRefFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<EmscriptenRefField>(*this, dslObj, parent);
}

EmscriptenGenerator::FieldPtr EmscriptenGenerator::createOptionalFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<EmscriptenOptionalField>(*this, dslObj, parent);
}

EmscriptenGenerator::FieldPtr EmscriptenGenerator::createVariantFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<EmscriptenVariantField>(*this, dslObj, parent);
}

// EmscriptenGenerator::LayerPtr EmscriptenGenerator::createCustomLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
// {
//     return std::make_unique<EmscriptenCustomLayer>(*this, dslObj, parent);
// }

// EmscriptenGenerator::LayerPtr EmscriptenGenerator::createSyncLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
// {
//     return std::make_unique<EmscriptenSyncLayer>(*this, dslObj, parent);
// }

// EmscriptenGenerator::LayerPtr EmscriptenGenerator::createSizeLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
// {
//     return std::make_unique<EmscriptenSizeLayer>(*this, dslObj, parent);
// }

// EmscriptenGenerator::LayerPtr EmscriptenGenerator::createIdLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
// {
//     return std::make_unique<EmscriptenIdLayer>(*this, dslObj, parent);
// }

// EmscriptenGenerator::LayerPtr EmscriptenGenerator::createValueLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
// {
//     return std::make_unique<EmscriptenValueLayer>(*this, dslObj, parent);
// }

// EmscriptenGenerator::LayerPtr EmscriptenGenerator::createPayloadLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
// {
//     return std::make_unique<EmscriptenPayloadLayer>(*this, dslObj, parent);
// }

// EmscriptenGenerator::LayerPtr EmscriptenGenerator::createChecksumLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
// {
//     return std::make_unique<EmscriptenChecksumLayer>(*this, dslObj, parent);
// }

bool EmscriptenGenerator::emscriptenWriteExtraFilesInternal() const
{
    auto& inputDir = getCodeDir();
    if (inputDir.empty()) {
        return true;
    }

    auto& outputDir = getOutputDir();
    auto pos = inputDir.size();
    auto endIter = fs::recursive_directory_iterator();
    for (auto iter = fs::recursive_directory_iterator(inputDir); iter != endIter; ++iter) {
        if (!iter->is_regular_file()) {
            continue;
        }
        

        auto srcPath = iter->path();
        auto ext = srcPath.extension().string();

        static const std::string ReservedExt[] = {
            strings::replaceFileSuffixStr(),
            strings::extendFileSuffixStr(),
            strings::publicFileSuffixStr(),
            strings::incFileSuffixStr(),
            strings::appendFileSuffixStr(),
            strings::prependFileSuffixStr(),
        };        
        auto extIter = std::find(std::begin(ReservedExt), std::end(ReservedExt), ext);
        if (extIter != std::end(ReservedExt)) {
            continue;
        }

        auto pathStr = srcPath.string();
        auto posTmp = pos;
        while (posTmp < pathStr.size()) {
            if (pathStr[posTmp] == fs::path::preferred_separator) {
                ++posTmp;
                continue;
            }
            break;
        }

        if (pathStr.size() <= posTmp) {
            continue;
        }

        std::string relPath(pathStr, posTmp);
        auto& protSchema = protocolSchema();
        auto schemaNs = util::strToName(protSchema.schemaName());
        do {
            if (protSchema.mainNamespace() == schemaNs) {
                break;
            }

            auto srcPrefix = (fs::path(strings::includeDirStr()) / schemaNs).string();
            if (!util::strStartsWith(relPath, srcPrefix)) {
                break;
            }

            auto dstPrefix = (fs::path(strings::includeDirStr()) / protSchema.mainNamespace()).string();
            relPath = dstPrefix + std::string(relPath, srcPrefix.size());
        } while (false);

        auto destPath = fs::path(outputDir) / relPath;
        logger().info("Copying " + destPath.string());

        if (!createDirectory(destPath.parent_path().string())) {
            return false;
        }

        std::error_code ec;
        fs::copy_file(srcPath, destPath, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            logger().error("Failed to copy with reason: " + ec.message());
            return false;
        }

        if (protSchema.mainNamespace() != schemaNs) {
            // The namespace has changed

            auto destStr = destPath.string();
            std::ifstream stream(destStr);
            if (!stream) {
                logger().error("Failed to open " + destStr + " for modification.");
                return false;
            }

            std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            stream.close();

            util::strReplace(content, "namespace " + schemaNs, "namespace " + protSchema.mainNamespace());
            std::ofstream outStream(destStr, std::ios_base::trunc);
            if (!outStream) {
                logger().error("Failed to modify " + destStr + ".");
                return false;
            }

            outStream << content;
            logger().info("Updated " + destStr + " to have proper main namespace.");
        }
    }
    return true;
}

bool EmscriptenGenerator::emscriptenPrepareDefaultInterfaceInternal()
{
    auto allInterfaces = getAllInterfaces();
    if (!allInterfaces.empty()) {
        return true;
    }

    auto* defaultNamespace = addDefaultNamespace();
    auto* interface = defaultNamespace->addDefaultInterface();
    if (interface == nullptr) {
        logger().error("Failed to create default interface");
        return false;
    }

    return true;
}

bool EmscriptenGenerator::emscriptenReferenceRequestedInterfaceInternal()
{
    auto* mainInterface = emscriptenMainInterface();
    if (mainInterface != nullptr) {
        mainInterface->setReferenced(true);
    }

    return true;
}

bool EmscriptenGenerator::emscriptenReferenceRequestedMessagesInternal()
{
    if ((m_messagesListFile.empty()) && (m_forcedPlatform.empty())) {
        referenceAllMessages();
        return true;
    }

    if ((!m_messagesListFile.empty()) && (!m_forcedPlatform.empty())) {
        logger().error("Cannot force platform messages together with explicit message list.");
        return false;
    }    

    if (!m_messagesListFile.empty()) {
        return emscriptenProcessMessagesListFileInternal();
    }

    if (!m_forcedPlatform.empty()) {
        return emscriptenProcessForcedPlatformInternal();
    }    

    return true;
}

bool EmscriptenGenerator::emscriptenProcessMessagesListFileInternal()
{
    std::ifstream stream(m_messagesListFile);
    if (!stream) {
        logger().error("Failed to open messages list file: \"" + m_messagesListFile + "\".");
        return false;
    }

    std::string contents(std::istreambuf_iterator<char>(stream), (std::istreambuf_iterator<char>()));
    auto lines = util::strSplitByAnyChar(contents, "\n\r");

    for (auto& l : lines) {
        auto* m = findMessage(l);
        if (m == nullptr) {
            logger().error("Failed to fined message \"" + l + "\" listed in \"" + m_messagesListFile + "\".");
            return false;
        }

        m->setReferenced(true);
    }

    return true;
}

bool EmscriptenGenerator::emscriptenProcessForcedPlatformInternal()
{
    bool validPlatform = false;

    assert(!m_forcedPlatform.empty());
    for (auto* m : getAllMessages()) {
        assert(m != nullptr);
        auto& s = schemaOf(*m);
        auto& schemaPlatforms = s.dslObj().platforms();
        auto iter = std::find(schemaPlatforms.begin(), schemaPlatforms.end(), m_forcedPlatform);
        if (iter == schemaPlatforms.end()) {
            continue;
        }

        validPlatform = true;

        auto* emscriptenM = const_cast<EmscriptenMessage*>(EmscriptenMessage::cast(m));
        auto& messagePlatforms = emscriptenM->dslObj().platforms();

        bool messageSupported = 
            (messagePlatforms.empty()) || 
            (std::find(messagePlatforms.begin(), messagePlatforms.end(), m_forcedPlatform) != messagePlatforms.end());

        if (messageSupported) {
            emscriptenM->setReferenced(true);
        }
    }
    
    if (!validPlatform) {
        logger().error("Unknown platform: \"" + m_forcedPlatform + "\".");
        return false;
    }

    return true;
}

} // namespace commsdsl2emscripten
