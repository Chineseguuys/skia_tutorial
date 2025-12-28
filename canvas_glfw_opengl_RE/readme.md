# skia cache load 堆栈解析

> 我们写的例子当中，最终 canvas 调用的是 drawRect() 函数或者 drawRRect() 函数。

```
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0xa7e449
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0xa5e900
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0xa5e5ee
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0xa42c78
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0xa67743
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x7b9684
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x7b771a
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x7b7686
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x945634
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x94fc00
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x974d78
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x74de72
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x775027
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x774b1e
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x776565 
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrDirectContext::flush(GrFlushInfo const&)+0xf5
```

解析之后的堆栈为：

```
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrGLProgramBuilder::CreateProgram(GrDirectContext*, GrProgramDesc const&, GrProgramInfo const&, GrGLPrecompiledProgram const*)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrGLGpu::ProgramCache::findOrCreateProgramImpl(GrDirectContext*, GrProgramDesc const&, GrProgramInfo const&, GrThreadSafePipelineBuilder::Stats::ProgramCacheResult*)  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrGLGpu::ProgramCache::findOrCreateProgram(GrDirectContext*, GrProgramInfo const&)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrGLGpu::flushGLState(GrRenderTarget*, bool, GrProgramInfo const&)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrGLOpsRenderPass::onBindPipeline(GrProgramInfo const&, SkRect const&)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrOpsRenderPass::bindPipeline(GrProgramInfo const&, SkRect const&)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrOpFlushState::bindPipeline(GrProgramInfo const&, SkRect const&)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrOpFlushState::bindPipelineAndScissorClip(GrProgramInfo const&, SkRect const&)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : skgpu::ganesh::FillRRectOp::(anonymous namespace)::FillRRectOpImpl::onExecute(GrOpFlushState*, SkRect const&)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrOp::execute(GrOpFlushState*, SkRect const&)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : skgpu::ganesh::OpsTask::onExecute(GrOpFlushState*)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrRenderTask::execute(GrOpFlushState*)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrDrawingManager::executeRenderTasks(GrOpFlushState*)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrDrawingManager::flush(SkSpan<GrSurfaceProxy*>, SkSurfaces::BackendSurfaceAccess, GrFlushInfo const&, skgpu::MutableTextureState const*)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrDrawingManager::flushSurfaces(SkSpan<GrSurfaceProxy*>, SkSurfaces::BackendSurfaceAccess, GrFlushInfo const&, skgpu::MutableTextureState const*) 
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrDirectContext::flush(GrFlushInfo const&)+0xf5
```

# skia cache save 堆栈解析

```
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0xa7f052
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0xa80a85
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0xa7e4d3
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0xa5e900
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0xa5e5ee
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0xa42c78
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0xa67743
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x7b9684
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x7b771a
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x7b7686
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x945634
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x94fc00
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x974d78
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x74de72
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x775027
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x774b1e
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : +0x776565
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrDirectContext::flush(GrFlushInfo const&)+0xf5
```

解析之后的结果为：

```
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrGLProgramBuilder::storeShaderInCache(SkSL::ProgramInterface const&, unsigned int, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const*, bool, SkSL::ProgramSettings*)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrGLProgramBuilder::finalize(GrGLPrecompiledProgram const*)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrGLProgramBuilder::CreateProgram(GrDirectContext*, GrProgramDesc const&, GrProgramInfo const&, GrGLPrecompiledProgram const*)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrGLGpu::ProgramCache::findOrCreateProgramImpl(GrDirectContext*, GrProgramDesc const&, GrProgramInfo const&, GrThreadSafePipelineBuilder::Stats::ProgramCacheResult*)  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrGLGpu::ProgramCache::findOrCreateProgram(GrDirectContext*, GrProgramInfo const&)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrGLGpu::flushGLState(GrRenderTarget*, bool, GrProgramInfo const&)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrGLOpsRenderPass::onBindPipeline(GrProgramInfo const&, SkRect const&)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrOpsRenderPass::bindPipeline(GrProgramInfo const&, SkRect const&)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrOpFlushState::bindPipeline(GrProgramInfo const&, SkRect const&)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrOpFlushState::bindPipelineAndScissorClip(GrProgramInfo const&, SkRect const&)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : skgpu::ganesh::FillRRectOp::(anonymous namespace)::FillRRectOpImpl::onExecute(GrOpFlushState*, SkRect const&)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrOp::execute(GrOpFlushState*, SkRect const&)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : skgpu::ganesh::OpsTask::onExecute(GrOpFlushState*)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrRenderTask::execute(GrOpFlushState*)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrDrawingManager::executeRenderTasks(GrOpFlushState*)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrDrawingManager::flush(SkSpan<GrSurfaceProxy*>, SkSurfaces::BackendSurfaceAccess, GrFlushInfo const&, skgpu::MutableTextureState const*)
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrDrawingManager::flushSurfaces(SkSpan<GrSurfaceProxy*>, SkSurfaces::BackendSurfaceAccess, GrFlushInfo const&, skgpu::MutableTextureState const*) 
  /home/yanjiangha/mirrors/sda1_doc/temp_storage/skia/out/Shared/libskia.so : GrDirectContext::flush(GrFlushInfo const&)+0xf5
```


