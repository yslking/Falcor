#pragma once

#include <string_view>

namespace Falcor
{
    template <typename T>
    bool setConstantBufferItem(ShaderVar& shaderVar, std::string_view blockName, std::string_view fieldName, const T& value)
    {
        ShaderVar& var = shaderVar.findMember(std::string{blockName});
        if (!var.isValid()) return false;

        ShaderVar& varRes = var.getParameterBlock()->findMember(std::string(fieldName));
        if (!varRes.isValid()) return false;

        return varRes.set(value);
    }

    bool setTopLevelItem(ShaderVar& shaderVar, std::string_view itemName, const Texture::SharedPtr& value)
    {
        ShaderVar& var = shaderVar.findMember(std::string{ itemName });
        if (!var.isValid()) return false;

        return var.setTexture(value);
    }

    bool setTopLevelItem(ShaderVar& shaderVar, std::string_view itemName, const Buffer::SharedPtr& value)
    {
        ShaderVar& var = shaderVar.findMember(std::string{ itemName });
        if (!var.isValid()) return false;

        return var.setBuffer(value);
    }

    bool setTopLevelItem(ShaderVar& shaderVar, std::string_view itemName, const Sampler::SharedPtr& value)
    {
        ShaderVar& var = shaderVar.findMember(std::string{ itemName });
        if (!var.isValid()) return false;

        return var.setSampler(value);
    }

    bool setTopLevelItem(ShaderVar& shaderVar, std::string_view itemName, const UnorderedAccessView::SharedPtr& value)
    {
        ShaderVar& var = shaderVar.findMember(std::string{ itemName });
        if (!var.isValid()) return false;

        return var.setUav(value);
    }

    bool setTopLevelItem(ShaderVar& shaderVar, std::string_view itemName, const ShaderResourceView::SharedPtr& value)
    {
        ShaderVar& var = shaderVar.findMember(std::string{ itemName });
        if (!var.isValid()) return false;

        return var.setSrv(value);
    }
}

