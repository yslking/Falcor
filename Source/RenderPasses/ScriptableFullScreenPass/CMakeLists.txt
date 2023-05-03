add_renderpass(ScriptableFullScreenPass)

target_sources(ScriptableFullScreenPass PRIVATE
    ScriptableFullScreenPass.cpp
    ScriptableFullScreenPass.h
    DefaultShader.ps.slang
    DefaultShader.cs.slang
)

target_copy_shaders(ScriptableFullScreenPass RenderPasses/ScriptableFullScreenPass)

target_source_group(ScriptableFullScreenPass "RenderPasses")
