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


namespace
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
        pybind11::class_<ScriptableFullScreenPass, RenderPass, ScriptableFullScreenPass::SharedPtr> pass(m, "ScriptableFullScreenPass");
        pass.def_property("shaderPath", &ScriptableFullScreenPass::getShaderPath, &ScriptableFullScreenPass::setShaderPath);
        //pass.def_property("numberOfTargets", &ScriptableFullScreenPass::getNumberOfTargets, &ScriptableFullScreenPass::setNumberOfTargets);

        pybind11::class_<ResourceDesc> resourceDesc(m, "ResourceDesc");
        resourceDesc.def(pybind11::init(&ResourceDesc::create));
        resourceDesc.def(pybind11::init(&ResourceDesc::createDefault));
        // static ResourceDesc create(const std::string identifier, Type type, const uint3& size, bool autoSized, uint32_t targetSlot, View view, Format format, bool clear)
        
        resourceDesc.def("__repr__", [m, resourceDesc](const ResourceDesc& desc) {
            return fmt::format(
                "ResourceDesc({}, {}, {}, {}, {}, {}, {}, {})",
                pyRepr(desc.identifier),
                pyRepr(desc.type),
                pyRepr(desc.size),
                pyRepr(desc.autoSized),
                pyRepr(desc.targetSlot),
                pyRepr(desc.view),
                pyRepr(desc.format),
                pyRepr(desc.clear)
            );
        });
        
        //resourceType.def_readwrite("type", &ResourceDesc::type);
        //resourceType.def_readwrite("size", &ResourceDesc::size);
        //resourceType.def_readwrite("identifier", &ResourceDesc::identifier);
        //resourceType.def_readwrite("autoSized", &ResourceDesc::autoSized);
        //resourceType.def_readwrite("view", &ResourceDesc::view);
        //resourceType.def_readwrite("targetSlot", &ResourceDesc::targetSlot);

        registerEnum<ResourceDesc::Type>(resourceDesc, "Type", ResourceDesc::TypeNames);
        registerEnum<ResourceDesc::View>(resourceDesc, "View", ResourceDesc::ViewNames);
        registerEnum<ResourceDesc::Format>(resourceDesc, "Format", ResourceDesc::FormatNames);

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
    ScriptBindings::registerBinding(regScriptableFullScreenPass);
}

ScriptableFullScreenPass::SharedPtr ScriptableFullScreenPass::create(RenderContext* pRenderContext, const Dictionary& dict)
{
    SharedPtr pPass = SharedPtr(new ScriptableFullScreenPass(dict));
    return pPass;
}

const RenderPass::Info ScriptableFullScreenPass::kInfo = { "ScriptableFullScreenPass", "no desc.." };
const std::string ScriptableFullScreenPass::sDefaultPixelShaderPath = "RenderPasses/ScriptableFullScreenPass/DefaultShader.ps.slang";
const std::string ScriptableFullScreenPass::sDefaultComputeShaderPath = "RenderPasses/ScriptableFullScreenPass/DefaultShader.cs.slang";

const std::vector<std::string> ResourceDesc::TypeNames = { "Texture1D", "Texture2D", "Texture3D", "TextureCube", "RawBuffer", };
const std::vector<std::string> ResourceDesc::ViewNames = { "RTV_Out", "RTV_InOut", "UAV_Out", "UAV_InOut", "SRV", };
const std::vector<std::string> ResourceDesc::FormatNames = { "Auto", "Unknown", "RGBA32F", "RGBA32U", "RGBA8Unorm", };

static std::string kShaderPath = "kShaderPath";
static std::string kResources = "kResources";
static std::string kThreads = "kThreads";
static std::string kCompute = "kCompute";
static std::string kAutoThreads = "kAutoThreads";

Dictionary ScriptableFullScreenPass::getScriptingDictionary()
{
    Dictionary out;
    out[kShaderPath] = mShaderPath;
    out[kResources] = mResources;
    out[kThreads] = mThreads;
    out[kCompute] = mCompute;
    out[kAutoThreads] = mAutoThreads;
    return out;
}

void ScriptableFullScreenPass::loadFromDictionary(const Dictionary& dict)
{
    mShaderPath = dict.get(kShaderPath, mShaderPath);
    mResources = dict.get(kResources, mResources);
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
        case ResourceDesc::Format::RGBA8Unorm:
            field->format(ResourceFormat::RGBA8Unorm);
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
    if (mpScene)
        defineList = mpScene->getSceneDefines();
    else
        defineList = Scene::getDefaultSceneDefines();

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
    
    auto group = widget.group("Resources");
    {
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
            subgroup.dropdown("Format", ResourceDesc::getFormatDropdownList(), (uint32_t&)res.format);
            subgroup.checkbox("Auto Sized", res.autoSized);
            if (res.type != ResourceDesc::Type::Texture2D)
                res.autoSized = false;
            if(!res.autoSized)
                subgroup.var("Size", res.size, 1);
            subgroup.checkbox("Clear", res.clear);
            if (subgroup.button("Delete"))
                toDelete.insert(i);
        }

        auto old = mResources;
        mResources.clear();
        for (size_t i = 0; i < old.size(); i++)
            if(!toDelete.count(i))
                mResources.push_back(old[i]);
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


