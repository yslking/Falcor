/***************************************************************************
 # Copyright (c) 2015-22, NVIDIA CORPORATION. All rights reserved.
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
#include "ScriptableFullScreenPass.h"
#include "RenderGraph/RenderPassLibrary.h"
#include "RenderGraph/RenderPassUtils.h"

static std::string kShaderPath = "kShaderPath";
static std::string kResources = "kResources";
static std::string kExternalTextures = "kExternalResources";
static std::string kThreads = "kThreads";
static std::string kCompute = "kCompute";
static std::string kAutoThreads = "kAutoThreads";

namespace ScriptableFullScreenPassStatic
{

    static std::string pyRepr(const std::string& val)
    {
        // todo: escape
        return "\"" + val + "\"";
    }

    static std::string pyRepr(bool val)
    {
        return val ? "True" : "False";
    }

    static std::string pyRepr(uint32_t val)
    {
        return std::to_string(val);
    }

    static std::string pyRepr(const uint3& val)
    {
        return fmt::format("uint3({}, {}, {})", val.x, val.y, val.z);
    }

    static std::string pyRepr(ResourceDesc::Type val)
    {
        return "ResourceDesc.Type." + ResourceDesc::TypeNames[(uint32_t)val];
    }

    static std::string pyRepr(ResourceDesc::View val)
    {
        return "ResourceDesc.View." + ResourceDesc::ViewNames[(uint32_t) val];
    }

    static std::string pyRepr(ResourceDesc::Format val)
    {
        return "ResourceDesc.Format." + ResourceDesc::FormatNames[(uint32_t) val];
    }

    template <typename EnumType>
    void registerEnum(const pybind11::handle& parent, const char* name, const std::vector<std::string>& nameList)
    {
        pybind11::enum_<EnumType> pyenum(parent, name);
        for(size_t i = 0; i < nameList.size(); i++) {
            pyenum.value(nameList[i].c_str(), (EnumType)i);
        }
        pyenum.export_values();
    }

    void regScriptableFullScreenPass(pybind11::module& m)
    {
        pybind11::class_<ExternalTextureDesc> externalTextureDesc(m, "ExternalTextureDesc");
        externalTextureDesc.def_readwrite("identifier", &ExternalTextureDesc::identifier);
        externalTextureDesc.def_readwrite("path", &ExternalTextureDesc::path);
        externalTextureDesc.def_readwrite("sRGB", &ExternalTextureDesc::sRGB);
        externalTextureDesc.def_readwrite("createMips", &ExternalTextureDesc::createMips);
        ExternalTextureDesc externalTextureDescDefault;
        externalTextureDesc.def(
            pybind11::init(&ExternalTextureDesc::create),
            pybind11::arg("identifier") = externalTextureDescDefault.identifier,
            pybind11::arg("path") = externalTextureDescDefault.path,
            pybind11::arg("sRGB") = externalTextureDescDefault.sRGB,
            pybind11::arg("createMips") = externalTextureDescDefault.createMips
        );
        externalTextureDesc.def("clone", [](ExternalTextureDesc self) { return self; });
        externalTextureDesc.def("__repr__", [m, externalTextureDesc](const ExternalTextureDesc& desc) {
            return fmt::format(
                "ExternalTextureDesc(identifier={}, path={}, sRGB={}, createMips={})",
                pyRepr(desc.identifier),
                pyRepr(desc.path),
                pyRepr(desc.sRGB),
                pyRepr(desc.createMips)
            );
        });

        pybind11::class_<ResourceDesc> resourceDesc(m, "ResourceDesc");
        registerEnum<ResourceDesc::Type>(resourceDesc, "Type", ResourceDesc::TypeNames);
        registerEnum<ResourceDesc::View>(resourceDesc, "View", ResourceDesc::ViewNames);
        registerEnum<ResourceDesc::Format>(resourceDesc, "Format", ResourceDesc::FormatNames);

        ResourceDesc defaultDesc = ResourceDesc::createDefault();
        resourceDesc.def(
            pybind11::init(&ResourceDesc::create),
            pybind11::arg("identifier") = defaultDesc.identifier,
            pybind11::arg("type") = defaultDesc.type,
            pybind11::arg("size") = defaultDesc.size,
            pybind11::arg("autoSized") = defaultDesc.autoSized,
            pybind11::arg("targetSlot") = defaultDesc.targetSlot,
            pybind11::arg("view") = defaultDesc.view,
            pybind11::arg("format") = defaultDesc.format,
            pybind11::arg("clear") = defaultDesc.clear,
            pybind11::arg("optional") = defaultDesc.optional
        );

        resourceDesc.def_readwrite("identifier", &ResourceDesc::identifier);
        resourceDesc.def_readwrite("type", &ResourceDesc::type);
        resourceDesc.def_readwrite("size", &ResourceDesc::size);
        resourceDesc.def_readwrite("autoSized", &ResourceDesc::autoSized);
        resourceDesc.def_readwrite("targetSlot", &ResourceDesc::targetSlot);
        resourceDesc.def_readwrite("view", &ResourceDesc::view);
        resourceDesc.def_readwrite("format", &ResourceDesc::format);
        resourceDesc.def_readwrite("clear", &ResourceDesc::clear);
        resourceDesc.def_readwrite("optional", &ResourceDesc::optional);
        resourceDesc.def("clone", [](ResourceDesc self) { return self; });

        resourceDesc.def("__repr__", [m, resourceDesc](const ResourceDesc& desc) {
            return fmt::format(
                "ResourceDesc(identifier={}, type={}, size={}, autoSized={}, targetSlot={}, view={}, format={}, clear={}, optional={})",
                pyRepr(desc.identifier),
                pyRepr(desc.type),
                pyRepr(desc.size),
                pyRepr(desc.autoSized),
                pyRepr(desc.targetSlot),
                pyRepr(desc.view),
                pyRepr(desc.format),
                pyRepr(desc.clear),
                pyRepr(desc.optional)
            );
        });

        pybind11::class_<ScriptableFullScreenPass, RenderPass, ScriptableFullScreenPass::SharedPtr> pass(m, "ScriptableFullScreenPass");

        pass.def_readwrite("shaderPath", &ScriptableFullScreenPass::mShaderPath);
        pass.def_readwrite("resources", &ScriptableFullScreenPass::mResources);
        pass.def_readwrite("externalTextures", &ScriptableFullScreenPass::mExternalTextures);
        pass.def_readwrite("threads", &ScriptableFullScreenPass::mThreads);
        pass.def_readwrite("compute", &ScriptableFullScreenPass::mCompute);
        pass.def_readwrite("autoThreads", &ScriptableFullScreenPass::mAutoThreads);
    }
}

// Don't remove this. it's required for hot-reload to function properly
extern "C" FALCOR_API_EXPORT const char* getProjDir()
{
    return PROJECT_DIR;
}

extern "C" FALCOR_API_EXPORT void getPasses(Falcor::RenderPassLibrary& lib)
{
    lib.registerPass(ScriptableFullScreenPass::kInfo, ScriptableFullScreenPass::create);
    ScriptBindings::registerBinding(ScriptableFullScreenPassStatic::regScriptableFullScreenPass);
}

ScriptableFullScreenPass::SharedPtr ScriptableFullScreenPass::create(RenderContext* pRenderContext, const Dictionary& dict)
{
    SharedPtr pPass = SharedPtr(new ScriptableFullScreenPass(dict));
    return pPass;
}

const RenderPass::Info ScriptableFullScreenPass::kInfo = { "ScriptableFullScreenPass", "no desc.." };
const std::string ScriptableFullScreenPass::sDefaultPixelShaderPath = "RenderPasses/ScriptableFullScreenPass/DefaultShader.ps.slang";
const std::string ScriptableFullScreenPass::sDefaultComputeShaderPath = "RenderPasses/ScriptableFullScreenPass/DefaultShader.cs.slang";

const std::vector<std::string> ResourceDesc::TypeNames = { "Texture1D", "Texture2D", "Texture3D", "Texture2DArray", "TextureCube", "RawBuffer", };
const std::vector<std::string> ResourceDesc::ViewNames = { "RTV_Out", "RTV_InOut", "UAV_Out", "UAV_InOut", "SRV", };
const std::vector<std::string> ResourceDesc::FormatNames = { "Auto", "Unknown", "RGBA32F", "RGBA32U", "RGBA32I", "RGBA8Unorm", "R32F", "R32U", "R32I", "RG32F"};

Dictionary ScriptableFullScreenPass::getScriptingDictionary()
{
    Dictionary out;
    out[kShaderPath] = mShaderPath;
    out[kResources] = mResources;
    out[kExternalTextures] = mExternalTextures;
    out[kThreads] = mThreads;
    out[kCompute] = mCompute;
    out[kAutoThreads] = mAutoThreads;
    return out;
}

void ScriptableFullScreenPass::loadFromDictionary(const Dictionary& dict)
{
    mShaderPath = dict.get(kShaderPath, mShaderPath);
    mResources = dict.get(kResources, mResources);
    mExternalTextures = dict.get(kExternalTextures, mExternalTextures);
    mThreads = dict.get(kThreads, mThreads);
    mCompute = dict.get(kCompute, mCompute);
    mAutoThreads = dict.get(kAutoThreads, mAutoThreads);
}

RenderPassReflection ScriptableFullScreenPass::reflect(const CompileData& compileData)
{
    // Define the required resources here
    RenderPassReflection reflector;

    for (auto& res : mResources)
    {
        if (res.identifier.empty()) continue;

        RenderPassReflection::Field* field{ nullptr };
        switch (res.view)
        {
        case ResourceDesc::View::RTV_Out:
            field = &reflector.addOutput(res.identifier, ".");
            break;
        case ResourceDesc::View::RTV_InOut:
            field = &reflector.addInputOutput(res.identifier, ".");
            break;
        case ResourceDesc::View::UAV_Out:
            field = &reflector.addOutput(res.identifier, ".");
            break;
        case ResourceDesc::View::UAV_InOut:
            field = &reflector.addInputOutput(res.identifier, ".");
            break;
        case ResourceDesc::View::SRV:
            field = &reflector.addInput(res.identifier, ".");
            break;
        default:
            throw std::runtime_error("unknown view");
        }

        if(res.optional)
            field->flags(RenderPassReflection::Field::Flags::Optional);

        switch (res.type)
        {
        case ResourceDesc::Type::Texture1D:
            field->texture1D(res.size.x);
            break;
        case ResourceDesc::Type::Texture2D:
            if(!res.autoSized) field->texture2D(res.size.x, res.size.y);
            break;
        case ResourceDesc::Type::Texture3D:
            field->texture3D(res.size.x, res.size.y, res.size.z);
            break;
        case ResourceDesc::Type::Texture2DArray:
            field->texture2D(res.size.x, res.size.y, 1, 1, res.size.z);
            break;
        case ResourceDesc::Type::RawBuffer:
            field->rawBuffer(res.size.x);
            break;
        default:
            throw std::runtime_error("not supported yet");
        }

        switch (res.format)
        {
        case ResourceDesc::Format::Unknown:
            field->format(ResourceFormat::Unknown);
            break;
        case ResourceDesc::Format::RGBA32F:
            field->format(ResourceFormat::RGBA32Float);
            break;
        case ResourceDesc::Format::RGBA32U:
            field->format(ResourceFormat::RGBA32Uint);
            break;
        case ResourceDesc::Format::RGBA32I:
            field->format(ResourceFormat::RGBA32Int);
            break;
        case ResourceDesc::Format::RGBA8Unorm:
            field->format(ResourceFormat::RGBA8Unorm);
            break;
        case ResourceDesc::Format::R32F:
            field->format(ResourceFormat::R32Float);
            break;
        case ResourceDesc::Format::R32U:
            field->format(ResourceFormat::R32Uint);
            break;
        case ResourceDesc::Format::R32I:
            field->format(ResourceFormat::R32Int);
            break;
        case ResourceDesc::Format::RG32F:
            field->format(ResourceFormat::RG32Float);
            break;
        case ResourceDesc::Format::Auto:
            // field->format(compileData.defaultTexFormat);
            break;
        default:
            throw std::runtime_error("unknown format");
        }

        if (res.view == ResourceDesc::View::UAV_InOut || res.view == ResourceDesc::View::UAV_Out)
            field->bindFlags(ResourceBindFlags::UnorderedAccess);
    }

    return reflector;
}

void ScriptableFullScreenPass::compile(RenderContext* pRenderContext, const CompileData& compileData)
{
    mFrameDim = compileData.defaultTexDims;
}

void ScriptableFullScreenPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    if (mCompute && pInternalComputePass == nullptr) mResourceCreated = false;
    if (!mCompute && pInternalGraphicsPass == nullptr) mResourceCreated = false;

    if (!mResourceCreated)
    {
        createResources();
        mResourceCreated = true;
    }

    try
    {
        bool hasValidTarget = false;

        std::vector<Texture::SharedPtr> targets;
        for (auto& res : mResources)
        {
            if (res.identifier.empty()) continue;

            if (res.view == ResourceDesc::View::RTV_Out || res.view == ResourceDesc::View::RTV_InOut)
            {
                targets.resize(res.targetSlot + 1);
                auto texture = renderData.getTexture(res.identifier);
                if (texture != nullptr)
                {
                    if (res.clear) pRenderContext->clearTexture(texture.get());

                    hasValidTarget = true;
                    targets[res.targetSlot] = texture;
                }
            }
            else if (res.view == ResourceDesc::View::UAV_Out || res.view == ResourceDesc::View::UAV_InOut)
            {
                auto resource = renderData.getResource(res.identifier);
                if (resource)
                {
                    auto uav = resource->getUAV();
                    if (uav != nullptr)
                    {
                        if (res.clear) pRenderContext->clearUAV(uav.get(), uint4(0, 0, 0, 0));
                        setTopLevelItem(getRootVar(), res.identifier, uav);
                    }
                }
            }
            else if (res.view == ResourceDesc::View::SRV)
            {
                auto resource = renderData.getResource(res.identifier);
                if (resource)
                {
                    auto srv = resource->getSRV();
                    if (srv != nullptr)
                    {
                        setTopLevelItem(getRootVar(), res.identifier, srv);
                    }
                }
            }
        }

        for (size_t i = 0; i < mExternalTextureHandles.size(); i++)
        {
            if (i >= mExternalTextures.size()) break;
            auto tex = mExternalTextureHandles[i];
            if(tex == nullptr) continue;
            auto desc = mExternalTextures[i];
            setTopLevelItem(getRootVar(), desc.identifier, tex);
        }

        setConstantBufferItem(getRootVar(), "PerFrameCB", "iMousePosition", mMousePosition);
        setConstantBufferItem(getRootVar(), "PerFrameCB", "iMouseCoordinate", mMouseCoordinate);
        setConstantBufferItem(getRootVar(), "PerFrameCB", "iMouseLastCoordinate", mMouseLastCoordinate);
        setConstantBufferItem(getRootVar(), "PerFrameCB", "iMouseLeftButtonDown", mMouseLeftButtonDown);
        setConstantBufferItem(getRootVar(), "PerFrameCB", "iTime", (float)gpFramework->getGlobalClock().getTime());
        setConstantBufferItem(getRootVar(), "PerFrameCB", "iDeltaTime", (float)gpFramework->getGlobalClock().getDelta());
        if (auto scene = mpScene)
        {

            if (auto camera = scene->getCamera())
            {
                auto& cameraData = camera->getCameraData();
                setConstantBufferItem(getRootVar(), "PerFrameCB", "iCameraPosition", cameraData.posW);
                setConstantBufferItem(getRootVar(), "PerFrameCB", "iCameraU", cameraData.cameraU);
                setConstantBufferItem(getRootVar(), "PerFrameCB", "iCameraV", cameraData.cameraV);
                setConstantBufferItem(getRootVar(), "PerFrameCB", "iCameraW", cameraData.cameraW);
                setConstantBufferItem(getRootVar(), "PerFrameCB", "iFrameDim", mFrameDim);
            }

            auto lights = scene->getLights();
            setConstantBufferItem(getRootVar(), "PerFrameCB", "iLightCount", uint32_t(lights.size()));

            if(auto lightsBuffer = mpScene->getLightsBuffer())
            {
                setTopLevelItem(getRootVar(), "iLights", lightsBuffer);
            }
        }

        if (!mCompute)
        {
            Fbo::SharedPtr fbo = hasValidTarget ?
                Fbo::create({ targets }, nullptr) :
                Fbo::create2D(renderData.getDefaultTextureDims().x, renderData.getDefaultTextureDims().y, ResourceFormat::RGBA32Float);

            setConstantBufferItem(getRootVar(), "PerFrameCB", "iResolution", uint2(fbo->getWidth(), fbo->getHeight()));

            pInternalGraphicsPass->execute(pRenderContext, fbo);
        }
        else
        {
            uint3 threads = mThreads;
            if (mAutoThreads) threads = uint3(renderData.getDefaultTextureDims().x, renderData.getDefaultTextureDims().y, 1);

            setConstantBufferItem(getRootVar(), "PerFrameCB", "iThreads", threads);
            pInternalComputePass->execute(pRenderContext, threads);
        }
    }
    catch (...)
    {
        pInternalGraphicsPass = nullptr;
        createResources();
    }

}

void ScriptableFullScreenPass::createResources()
{
    Program::DefineList defineList;
    //if (mpScene)
    //{
    //    defineList = mpScene->getSceneDefines();
    //    defineList.add(mpScene->getHitInfo().getDefines());
    //}
    //else
    //{
    //    defineList = Scene::getDefaultSceneDefines();
    //    defineList.add(HitInfo().getDefines());
    //}

    try
    {
        if(mCompute) pInternalComputePass = ComputePass::create(getShaderPath(), "main", defineList);
        else pInternalGraphicsPass = FullScreenPass::create(getShaderPath(), defineList, 0);
    }
    catch(...)
    {
        if (pInternalGraphicsPass == nullptr)
        {
            if(mCompute) pInternalComputePass = ComputePass::create(sDefaultComputeShaderPath, "main", defineList);
            else pInternalGraphicsPass = FullScreenPass::create(sDefaultPixelShaderPath, defineList, 0);
        }
    }

    if (mCompute) pInternalGraphicsPass = nullptr;
    else pInternalComputePass = nullptr;

    if (pSampler == nullptr)
    {
        Sampler::Desc samplerDesc;
        samplerDesc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear);
        samplerDesc.setAddressingMode(Sampler::AddressMode::Clamp, Sampler::AddressMode::Clamp, Sampler::AddressMode::Clamp);
        pSampler = Sampler::create(samplerDesc);
    }

    mExternalTextureHandles.clear();
    for(auto texDesc: mExternalTextures)
    {
        mExternalTextureHandles.push_back(Texture::createFromFile(texDesc.path, texDesc.createMips, texDesc.sRGB));
    }
}

void ScriptableFullScreenPass::renderUI(Gui::Widgets& widget)
{
    bool reload = widget.button("Reload");

    widget.textbox("Shader Path", this->mShaderPath);
    widget.checkbox("Compute", this->mCompute);
    if (this->mCompute)
    {
        widget.checkbox("Auto Threads", this->mAutoThreads);
        if (!this->mAutoThreads)
        {
            widget.var("Threads", this->mThreads, 1);
        }
    }
   
    {
        auto group = widget.group("Resources");

        if (group.button("Add"))
        {
            mResources.push_back({});
        }

        std::set<size_t> toDelete;

        for (size_t i = 0; i < mResources.size(); i++)
        {
            auto& res = mResources[i];
            auto subgroup = group.group(std::to_string(i));
            subgroup.textbox("Identifier", res.identifier);
            subgroup.dropdown("Type", ResourceDesc::getTypeDropdownList(), (uint32_t&)res.type);
            subgroup.dropdown("View", ResourceDesc::getViewDropdownList(), (uint32_t&)res.view);
            if (res.view == ResourceDesc::View::RTV_InOut || res.view == ResourceDesc::View::RTV_Out)
                subgroup.var<uint32_t>("Target Slot", res.targetSlot, 0, 7);
            
            subgroup.dropdown("Format", ResourceDesc::getFormatDropdownList(), (uint32_t&)res.format);
            subgroup.checkbox("Auto Sized", res.autoSized);
            if (res.type != ResourceDesc::Type::Texture2D)
                res.autoSized = false;
            if(!res.autoSized)
                subgroup.var("Size", res.size, 1);
            subgroup.checkbox("Clear", res.clear);
            subgroup.checkbox("Optional", res.optional);
            if (subgroup.button("Delete"))
                toDelete.insert(i);
        }

        auto old = mResources;
        mResources.clear();
        for (size_t i = 0; i < old.size(); i++)
            if(!toDelete.count(i))
                mResources.push_back(old[i]);
    }


    {
        auto group = widget.group("External Textures");

        if (group.button("Add"))
        {
            mExternalTextures.push_back({});
        }

        std::set<size_t> toDelete;

        for (size_t i = 0; i < mExternalTextures.size(); i++)
        {
            auto& res = mExternalTextures[i];
            auto subgroup = group.group(std::to_string(i));
            subgroup.textbox("Identifier", res.identifier);
            subgroup.textbox("Path", res.path);
            subgroup.checkbox("sRGB", res.sRGB);
            subgroup.checkbox("Create Mips", res.createMips);
            if (subgroup.button("Delete"))
                toDelete.insert(i);
        }

        auto old = mExternalTextures;
        mExternalTextures.clear();
        for (size_t i = 0; i < old.size(); i++)
            if (!toDelete.count(i))
                mExternalTextures.push_back(old[i]);
    }

    if (reload)
    {
        createResources();
        requestRecompile();
    }
}

void ScriptableFullScreenPass::setScene(RenderContext* pRenderContext, const Scene::SharedPtr& pScene)
{
    mpScene = pScene;
}

bool ScriptableFullScreenPass::onMouseEvent(const MouseEvent& mouseEvent)
{
    if (mouseEvent.type == MouseEvent::Type::Move)
    {
        mMousePosition = mouseEvent.pos;

        mMouseLastCoordinate = mMouseCoordinate;
        mMouseCoordinate = mouseEvent.screenPos;
    }

    if (mouseEvent.type == MouseEvent::Type::ButtonDown)
    {
        if (mouseEvent.button == Input::MouseButton::Left)
        {
            mMouseLeftButtonDown = true;
        }
    }

    if (mouseEvent.type == MouseEvent::Type::ButtonUp)
    {
        if (mouseEvent.button == Input::MouseButton::Left)
        {
            mMouseLeftButtonDown = false;
        }
    }

    return false;
}

void ScriptableFullScreenPass::initializeConstants()
{
}


