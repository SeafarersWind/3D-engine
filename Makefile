BUILD_NAME := game.exe
SRC_NAME := main.c

BUILD_DIR := 
SRC_DIR := 

BUILD := $(BUILD_DIR)$(BUILD_NAME)
SRC := $(SRC_DIR)$(SRC_NAME)

$(BUILD): $(SRC) $(BUILD_DIR)
	@gcc $(SRC) -o $(BUILD) -ggdb3 -Iinclude -Iinclude/GLFW/include -Iinclude/GLFW/deps -Iinclude/cglm-0.9.4/include \
	glad_gl.c -lglfw3 -lgdi32 -lassimp -lstdc++ \
	-Linclude/GLFW/build -Linclude/cglm-0.9.4/build -Linclude/assimp-5.4.2/build && \
	echo $(BUILD_NAME) rebuilt!

$(BUILD_DIR):
	@mkdir $@

include/GLFW/build:
	@cd include/GLFW && \
	gcc -c src/context.c src/init.c src/win32_thread.c src/window.c src/input.c src/win32_monitor.c src/monitor.c src/vulkan.c src/win32_init.c src/win32_window.c src/win32_time.c src/win32_joystick.c src/wgl_context.c src/egl_context.c src/osmesa_context.c -I .. -I deps -I deps/glad -I deps/mingw -D_GLFW_WIN32 && \
	rm -f build/* && mv *.o build && ar rcs build/libglfw3.a build/*.o && rm -f build/*.o && \
	echo GLFW rebuilt!

include/assimp-5.4.2/build:
	@cd include/assimp-5.4.2 && \
	gcc -c code/Common/Assimp.cpp code/Common/DefaultLogger.cpp code/Common/AssertHandler.cpp code/Common/Importer.cpp code/Common/ImporterRegistry.cpp code/Common/Scene.cpp code/Common/Version.cpp code/Common/IOSystem.cpp code/Common/DefaultIOSystem.cpp code/Common/DefaultIOStream.cpp code/Common/PostStepRegistry.cpp code/Common/BaseImporter.cpp code/Common/BaseProcess.cpp code/Common/Exceptional.cpp code/Common/ScenePreprocessor.cpp code/Common/SpatialSort.cpp code/Common/SceneCombiner.cpp code/Common/material.cpp code/Common/VertexTriangleAdjacency.cpp code/Common/ZipArchiveIOSystem.cpp code/Common/SkeletonMeshBuilder.cpp code/Common/CreateAnimMesh.cpp code/CApi/CInterfaceIOWrapper.cpp code/Material/MaterialSystem.cpp code/Geometry/GeometryUtils.cpp code/AssetLib/Obj/ObjFileImporter.cpp code/AssetLib/Obj/ObjFileParser.cpp code/AssetLib/Obj/ObjFileMtlImporter.cpp code/AssetLib/Collada/ColladaHelper.cpp code/AssetLib/Collada/ColladaLoader.cpp code/AssetLib/Collada/ColladaParser.cpp code/PostProcessing/ConvertToLHProcess.cpp code/PostProcessing/ValidateDataStructure.cpp code/PostProcessing/RemoveVCProcess.cpp code/PostProcessing/RemoveRedundantMaterials.cpp code/PostProcessing/FindInstancesProcess.cpp code/PostProcessing/OptimizeGraph.cpp code/PostProcessing/TextureTransform.cpp code/PostProcessing/ScaleProcess.cpp code/PostProcessing/PretransformVertices.cpp code/PostProcessing/FindDegenerates.cpp code/PostProcessing/SortByPTypeProcess.cpp code/PostProcessing/FindInvalidDataProcess.cpp code/PostProcessing/OptimizeMeshes.cpp code/PostProcessing/SplitByBoneCountProcess.cpp code/PostProcessing/SplitLargeMeshes.cpp code/PostProcessing/GenVertexNormalsProcess.cpp code/PostProcessing/CalcTangentsProcess.cpp code/PostProcessing/DeboneProcess.cpp code/PostProcessing/LimitBoneWeightsProcess.cpp code/PostProcessing/ImproveCacheLocality.cpp code/PostProcessing/ProcessHelper.cpp code/PostProcessing/EmbedTexturesProcess.cpp code/PostProcessing/ComputeUVMappingProcess.cpp code/PostProcessing/ArmaturePopulate.cpp code/PostProcessing/TriangulateProcess.cpp code/PostProcessing/FixNormalsStep.cpp code/PostProcessing/DropFaceNormalsProcess.cpp code/PostProcessing/GenFaceNormalsProcess.cpp code/PostProcessing/JoinVerticesProcess.cpp code/PostProcessing/GenBoundingBoxesProcess.cpp contrib/unzip/unzip.c contrib/unzip/ioapi.c contrib/zlib/inflate.c contrib/zlib/crc32.c contrib/zlib/adler32.c contrib/zlib/zutil.c contrib/zlib/inftrees.c contrib/zlib/inffast.c -Iinclude -Icode -Icontrib -Icontrib/pugixml/src -Icontrib/rapidjson/include -Icontrib/utf8cpp/source -Icontrib/openddlparser/include -Icontrib/unzip -Icontrib/zlib -DASSIMP_BUILD_NO_X_IMPORTER -DASSIMP_BUILD_NO_AMF_IMPORTER -DASSIMP_BUILD_NO_3DS_IMPORTER -DASSIMP_BUILD_NO_M3D_IMPORTER -DASSIMP_BUILD_NO_MD3_IMPORTER -DASSIMP_BUILD_NO_MD2_IMPORTER -DASSIMP_BUILD_NO_PLY_IMPORTER -DASSIMP_BUILD_NO_MDL_IMPORTER -DASSIMP_BUILD_NO_ASE_IMPORTER -DASSIMP_BUILD_NO_HMP_IMPORTER -DASSIMP_BUILD_NO_SMD_IMPORTER -DASSIMP_BUILD_NO_MDC_IMPORTER -DASSIMP_BUILD_NO_MD5_IMPORTER -DASSIMP_BUILD_NO_STL_IMPORTER -DASSIMP_BUILD_NO_LWO_IMPORTER -DASSIMP_BUILD_NO_DXF_IMPORTER -DASSIMP_BUILD_NO_NFF_IMPORTER -DASSIMP_BUILD_NO_RAW_IMPORTER -DASSIMP_BUILD_NO_SIB_IMPORTER -DASSIMP_BUILD_NO_OFF_IMPORTER -DASSIMP_BUILD_NO_AC_IMPORTER -DASSIMP_BUILD_NO_BVH_IMPORTER -DASSIMP_BUILD_NO_IRRMESH_IMPORTER -DASSIMP_BUILD_NO_IRR_IMPORTER -DASSIMP_BUILD_NO_Q3D_IMPORTER -DASSIMP_BUILD_NO_B3D_IMPORTER -DASSIMP_BUILD_NO_TERRAGEN_IMPORTER -DASSIMP_BUILD_NO_CSM_IMPORTER -DASSIMP_BUILD_NO_3D_IMPORTER -DASSIMP_BUILD_NO_LWS_IMPORTER -DASSIMP_BUILD_NO_OGRE_IMPORTER -DASSIMP_BUILD_NO_OPENGEX_IMPORTER -DASSIMP_BUILD_NO_MS3D_IMPORTER -DASSIMP_BUILD_NO_COB_IMPORTER -DASSIMP_BUILD_NO_BLEND_IMPORTER -DASSIMP_BUILD_NO_Q3BSP_IMPORTER -DASSIMP_BUILD_NO_NDO_IMPORTER -DASSIMP_BUILD_NO_IFC_IMPORTER -DASSIMP_BUILD_NO_XGL_IMPORTER -DASSIMP_BUILD_NO_FBX_IMPORTER -DASSIMP_BUILD_NO_ASSBIN_IMPORTER -DASSIMP_BUILD_NO_GLTF_IMPORTER -DASSIMP_BUILD_NO_C4D_IMPORTER -DASSIMP_BUILD_NO_3MF_IMPORTER -DASSIMP_BUILD_NO_X3D_IMPORTER -DASSIMP_BUILD_NO_MMD_IMPORTER -DASSIMP_BUILD_NO_M3D_IMPORTER -DASSIMP_BUILD_NO_IQM_IMPORTER && \
	rm -f build/* && mv *.o build && ar rcs build/libassimp.a build/*.o && rm -f build/*.o && \
	echo assimp-5.4.2 rebuilt!

.PHONY: clean
clean:

.PHONY: play
play: $(BUILD)
	@$(BUILD)
