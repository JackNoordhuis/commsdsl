#include "Generator.h"

#include "commsdsl/version.h"

#include "Test.h"
#include "Cmake.h"

#include <boost/filesystem.hpp>

namespace bf = boost::filesystem;

namespace commsdsl2test
{

const std::string& Generator::fileGeneratedComment()
{
    static const std::string Str =
        "// Generated by commsdsl2test v" + std::to_string(commsdsl::versionMajor()) +
        '.' + std::to_string(commsdsl::versionMinor()) + '.' +
        std::to_string(commsdsl::versionPatch()) + '\n';
    return Str;
}

bool Generator::writeImpl()
{
    return 
        Test::write(*this) &&
        Cmake::write(*this);
}

bool Generator::createDirectoryImpl(const std::string& path)
{
    boost::system::error_code ec;
    bf::create_directories(path, ec);
    if (ec) {
        logger().error("Failed to create directory \"" + path + "\" with reason: " + ec.message());
        return false;
    }

    return true;
}

} // namespace commsdsl2test
