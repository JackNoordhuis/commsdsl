#pragma once

#include "commsdsl/Layer.h"

#include "Layer.h"
#include "common.h"

namespace commsdsl2comms
{

class IdLayer : public Layer
{
    using Base = Layer;
public:
    IdLayer(Generator& generator, commsdsl::Layer layer) : Base(generator, layer) {}

protected:
    virtual void updateIncludesImpl(IncludesList& includes) const override;
    virtual std::string getClassDefinitionImpl(
        const std::string& scope,
        std::string& prevLayer,
        bool& hasInputMessages) const override;

private:
    commsdsl::IdLayer payloadLayerDslObj() const
    {
        return commsdsl::IdLayer(dslObj());
    }

    std::string getExtraOpt(const std::string& scope) const;
};

inline
LayerPtr createIdLayer(Generator& generator, commsdsl::Layer layer)
{
    return std::make_unique<IdLayer>(generator, layer);
}

} // namespace commsdsl2comms
