//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "CommsGenerator.h"

#include "CommsBitfieldField.h"
#include "CommsBundleField.h"
#include "CommsChecksumLayer.h"
#include "CommsCmake.h"
#include "CommsCustomLayer.h"
#include "CommsDataField.h"
#include "CommsDefaultOptions.h"
#include "CommsDispatch.h"
#include "CommsDoxygen.h"
#include "CommsEnumField.h"
#include "CommsFieldBase.h"
#include "CommsFloatField.h"
#include "CommsFrame.h"
#include "CommsInputMessages.h"
#include "CommsIntField.h"
#include "CommsListField.h"
#include "CommsIdLayer.h"
#include "CommsInterface.h"
#include "CommsMessage.h"
#include "CommsMsgId.h"
#include "CommsNamespace.h"
#include "CommsOptionalField.h"
#include "CommsPayloadLayer.h"
#include "CommsRefField.h"
#include "CommsSetField.h"
#include "CommsSizeLayer.h"
#include "CommsSyncLayer.h"
#include "CommsStringField.h"
#include "CommsValueLayer.h"
#include "CommsVariantField.h"
#include "CommsVersion.h"

#include "commsdsl/version.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <type_traits>

namespace fs = std::filesystem;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

const std::string MinCommsVersion("5.0.0");    

const std::string& CommsGenerator::fileGeneratedComment()
{
    static const std::string Str =
        "// Generated by commsdsl2comms v" + std::to_string(commsdsl::versionMajor()) +
        '.' + std::to_string(commsdsl::versionMinor()) + '.' +
        std::to_string(commsdsl::versionPatch()) + '\n';
    return Str;
}

CommsGenerator::CustomizationLevel CommsGenerator::getCustomizationLevel() const
{
    return m_customizationLevel;
}

void CommsGenerator::setCustomizationLevel(const std::string& value)
{
    if (value.empty()) {
        return;
    }

    static const std::string Map[] = {
        /* Full */ "full",
        /* Limited */ "limited",
        /* None */ "none",        
    };
    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == static_cast<unsigned>(CustomizationLevel::NumOfValues));

    auto iter = std::find(std::begin(Map), std::end(Map), value);
    if (iter == std::end(Map)) {
        logger().warning("Unknown customization level \"" + value + "\", using default.");
        return;
    }

    m_customizationLevel = static_cast<CustomizationLevel>(std::distance(std::begin(Map), iter));
}

const std::string& CommsGenerator::getProtocolVersion() const
{
    return m_protocolVersion;
}

void CommsGenerator::setProtocolVersion(const std::string& value)
{
    m_protocolVersion = value;
}

const std::vector<std::string>& CommsGenerator::getExtraInputBundles() const
{
    return m_extraInputBundles;
}

void CommsGenerator::setExtraInputBundles(const std::vector<std::string>& inputBundles)
{
    m_extraInputBundles = inputBundles;
}

const CommsGenerator::ExtraMessageBundlesList& CommsGenerator::extraMessageBundles() const
{
    return m_extraMessageBundles;
}

const std::string& CommsGenerator::minCommsVersion()
{
    return MinCommsVersion;
}

bool CommsGenerator::prepareImpl()
{
    if (!Base::prepareImpl()) {
        return false;
    }

    return 
        prepareDefaultInterfaceInternal() &&
        prepareExtraMessageBundlesInternal();
}

CommsGenerator::NamespacePtr CommsGenerator::createNamespaceImpl(commsdsl::parse::Namespace dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsNamespace>(*this, dslObj, parent);
}

CommsGenerator::InterfacePtr CommsGenerator::createInterfaceImpl(commsdsl::parse::Interface dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsInterface>(*this, dslObj, parent);
}

CommsGenerator::MessagePtr CommsGenerator::createMessageImpl(commsdsl::parse::Message dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsMessage>(*this, dslObj, parent);
}

CommsGenerator::FramePtr CommsGenerator::createFrameImpl(commsdsl::parse::Frame dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsFrame>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createIntFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsIntField>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createEnumFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsEnumField>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createSetFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsSetField>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createFloatFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsFloatField>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createBitfieldFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsBitfieldField>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createBundleFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsBundleField>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createStringFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsStringField>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createDataFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsDataField>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createListFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsListField>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createRefFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsRefField>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createOptionalFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsOptionalField>(*this, dslObj, parent);
}

CommsGenerator::FieldPtr CommsGenerator::createVariantFieldImpl(commsdsl::parse::Field dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsVariantField>(*this, dslObj, parent);
}

CommsGenerator::LayerPtr CommsGenerator::createCustomLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsCustomLayer>(*this, dslObj, parent);
}

CommsGenerator::LayerPtr CommsGenerator::createSyncLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsSyncLayer>(*this, dslObj, parent);
}

CommsGenerator::LayerPtr CommsGenerator::createSizeLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsSizeLayer>(*this, dslObj, parent);
}

CommsGenerator::LayerPtr CommsGenerator::createIdLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsIdLayer>(*this, dslObj, parent);
}

CommsGenerator::LayerPtr CommsGenerator::createValueLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsValueLayer>(*this, dslObj, parent);
}

CommsGenerator::LayerPtr CommsGenerator::createPayloadLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsPayloadLayer>(*this, dslObj, parent);
}

CommsGenerator::LayerPtr CommsGenerator::createChecksumLayerImpl(commsdsl::parse::Layer dslObj, Elem* parent)
{
    return std::make_unique<commsdsl2comms::CommsChecksumLayer>(*this, dslObj, parent);
}

bool CommsGenerator::writeImpl() 
{
    return 
        CommsCmake::write(*this) &&
        CommsMsgId::write(*this) &&
        CommsFieldBase::write(*this) &&
        CommsVersion::write(*this) &&
        CommsInputMessages::write(*this) &&
        CommsDefaultOptions::write(*this) &&
        CommsDispatch::write(*this) &&
        CommsDoxygen::write(*this) &&
        commsWriteExtraFilesInternal();
}

bool CommsGenerator::prepareDefaultInterfaceInternal()
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

bool CommsGenerator::prepareExtraMessageBundlesInternal()
{
    m_extraMessageBundles.reserve(m_extraInputBundles.size());
    for (auto& b : m_extraInputBundles) {
        std::string name;
        std::string path = b;

        auto sepPos = b.find_first_of('@');
        if (sepPos != std::string::npos) {
            name.assign(b, 0, sepPos);
            path.erase(path.begin(), path.begin() + sepPos + 1);
        }

        if (name.empty()) {
            name = fs::path(path).stem().string();
            logger().debug("Bundle name from path: " + path + " --> " + name);
        }

        if (name.empty()) {
            logger().error("Failed to idenity bundle name for " + b);
            return false;
        }

        std::ifstream stream(path);
        if (!stream) {
            logger().error("Failed to read extra messages bundle file " + path);
            return false;
        }

        std::string contents(std::istreambuf_iterator<char>(stream), (std::istreambuf_iterator<char>()));
        auto lines = util::strSplitByAnyChar(contents, "\n\r");
        MessagesAccessList messages;
        messages.reserve(lines.size());

        for (auto& l : lines) {
            auto* m = findMessage(l);
            if (m == nullptr) {
                logger().error("Failed to fined message \"" + l + "\" for bundle " + name);
                return false;
            }

            messages.push_back(m);
        }

        logger().debug("Extra message bundle: " + name);
        m_extraMessageBundles.emplace_back(std::move(name), std::move(messages));
    };
    return true;
}


bool CommsGenerator::commsWriteExtraFilesInternal() const
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
            strings::protectedFileSuffixStr(),
            strings::privateFileSuffixStr(),
            strings::valueFileSuffixStr(),
            strings::readFileSuffixStr(),
            strings::writeFileSuffixStr(),
            strings::lengthFileSuffixStr(),
            strings::validFileSuffixStr(),
            strings::refreshFileSuffixStr(),
            strings::nameFileSuffixStr(),
            strings::incFileSuffixStr(),
            strings::appendFileSuffixStr(),
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
        auto schemaNs = util::strToName(currentSchema().schemaName());
        do {
            if (currentSchema().mainNamespace() == schemaNs) {
                break;
            }

            auto srcPrefix = (fs::path(strings::includeDirStr()) / schemaNs).string();
            if (!util::strStartsWith(relPath, srcPrefix)) {
                break;
            }

            auto dstPrefix = (fs::path(strings::includeDirStr()) / currentSchema().mainNamespace()).string();
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

        if (currentSchema().mainNamespace() != schemaNs) {
            // The namespace has changed

            auto destStr = destPath.string();
            std::ifstream stream(destStr);
            if (!stream) {
                logger().error("Failed to open " + destStr + " for modification.");
                return false;
            }

            std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
            stream.close();

            util::strReplace(content, "namespace " + schemaNs, "namespace " + currentSchema().mainNamespace());
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

} // namespace commsdsl2comms
