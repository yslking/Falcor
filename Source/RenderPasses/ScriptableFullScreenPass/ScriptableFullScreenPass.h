/***************************************************************************
 # Copyright (c) 2015-21, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#pragma once
#include "Falcor.h"
#include "RenderGraph/BasePasses/FullScreenPass.h"
#include "RenderGraph/BasePasses/ComputePass.h"
#include <string>

namespace ScriptableFullScreenPassStatic {
    void regScriptableFullScreenPass(pybind11::module& m);
};

using namespace Falcor;

class ResourceDesc
{
public:

    template<typename EnumType>
    static Gui::DropdownList getDropdownList(const std::vector<std::string>& nameList)
    {
        static std::once_flag flag;
        static Gui::DropdownList out;
        std::call_once(flag, [pOut=&out, &nameList]() {
            for (size_t i = 0; i < nameList.size(); i++) {
                pOut->push_back({ (uint32_t) i, nameList[i] });
            }
        });
        return out;
    }

    enum class Type
    {
        Texture1D,
        Texture2D,
        Texture3D,
        Texture2DArray,
        TextureCube,
        RawBuffer,
    };
    static const std::vector<std::string> TypeNames;

    static Gui::DropdownList getTypeDropdownList() { return getDropdownList<Type>(TypeNames); }

    enum class View
    {
        RTV_Out,
        RTV_InOut,
        UAV_Out,
        UAV_InOut,
        SRV,
    };
    static const std::vector<std::string> ViewNames;

    static Gui::DropdownList getViewDropdownList() { return getDropdownList<View>(ViewNames); }

    enum class Format 
    {
        Auto,
        Unknown,
        RGBA32F,
        RGBA32U,
        RGBA32I,
        RGBA8Unorm,
        R32F,
        R32U,
        R32I,
        RG32F
    };
    static const std::vector<std::string> FormatNames;

    static Gui::DropdownList getFormatDropdownList() { return getDropdownList<Format>(FormatNames); }

    std::string identifier;
    Type type = Type::Texture2D;
    uint3 size = uint3(1, 1, 1);
    bool autoSized = true;
    uint32_t targetSlot = 0;
    View view = View::SRV;
    Format format = Format::Auto;
    bool clear = false;
    bool optional = true;

    // converter

    static ResourceDesc create(const std::string identifier, Type type, const uint3& size, bool autoSized, uint32_t targetSlot, View view, Format format, bool clear, bool optional)
    {
        return { identifier, type, size, autoSized, targetSlot, view, format, clear, optional };
    }

    static ResourceDesc createDefault()
    {
        return {};
    }
};


struct ExternalTextureDesc
{
    std::string identifier;
    std::string path;
    bool sRGB = false;
    bool createMips = true;

    static ExternalTextureDesc create(const std::string& identifier, const std::string& path, bool sRGB, bool createMips)
    {
        return { identifier, path, sRGB, createMips };
    }

    static ExternalTextureDesc createDefault()
    {
        return {};
    }
};


class ScriptableFullScreenPass : public RenderPass 
{
public:
    using SharedPtr = std::shared_ptr<ScriptableFullScreenPass>;

    static const Info kInfo;

    /** Create a new render pass object.
        \param[in] pRenderContext The render context.
        \param[in] dict Dictionary of serialized parameters.
        \return A new object, or an exception is thrown if creation failed.
    */
    static SharedPtr create(RenderContext* pRenderContext = nullptr, const Dictionary& dict = {});

    virtual Dictionary getScriptingDictionary() override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const Scene::SharedPtr& pScene) override;
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override;
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }

    std::string getShaderPath() { return mShaderPath.empty() ? (mCompute ? sDefaultComputeShaderPath : sDefaultPixelShaderPath) : mShaderPath; }
    void setShaderPath(const std::string& value) { mShaderPath = value; }

private:
    ScriptableFullScreenPass(const Dictionary& dict) : RenderPass(kInfo)
    {
        initializeConstants();
        loadFromDictionary(dict);
    }

    void createResources();
    void loadFromDictionary(const Dictionary&);
    void initializeConstants();

    ShaderVar getRootVar() {
        return mCompute ? pInternalComputePass->getRootVar() : pInternalGraphicsPass->getRootVar();
    }

    FullScreenPass::SharedPtr pInternalGraphicsPass;
    ComputePass::SharedPtr pInternalComputePass;

    Sampler::SharedPtr pSampler;
    uint32_t mInputTexturesCount = 1;
    bool mResourceCreated = false;
    Scene::SharedPtr mpScene;
    uint2 mFrameDim{ 0, 0 };

    // input
    float2 mMousePosition;
    int2 mMouseCoordinate;
    int2 mMouseLastCoordinate;
    bool mMouseLeftButtonDown = false;

    // Dictionary
    std::string mShaderPath;
    std::vector<ResourceDesc> mResources;
    std::vector<ExternalTextureDesc> mExternalTextures;
    std::vector<Texture::SharedPtr> mExternalTextureHandles;
    bool mCompute = false;
    uint3 mThreads = uint3(1, 1, 1);
    bool mAutoThreads = false;

    // static constants
    static const std::string sDefaultPixelShaderPath;
    static const std::string sDefaultComputeShaderPath;

public:
    friend void ScriptableFullScreenPassStatic::regScriptableFullScreenPass(pybind11::module& m);
};

