
lib_LTLIBRARIES=libRaytracing.la Raytracing.la

BUILT_SOURCES=\
		FreeCADpov.h \
		RenderCameraPy.cpp \
		RenderFeaturePy.cpp \

libRaytracing_la_BUILT=\
		RenderCameraPy.h \
		RenderFeaturePy.h

libRaytracing_la_SOURCES=\
		AppRaytracingPy.cpp \
		Appearances.cpp \
		Appearances.h \
		LibraryMaterial.cpp \
		LibraryMaterial.h \
		renderer/lux/LuxRender.cpp \
		renderer/lux/LuxRender.h \
		renderer/lux/LuxRenderProcess.cpp \
		renderer/lux/LuxRenderProcess.h \
		PropertyRenderMaterialList.cpp \
		PropertyRenderMaterialList.h \
		PovTools.cpp \
		PovTools.h \
		PreCompiled.cpp \
		PreCompiled.h \
		RayFeature.cpp \
		RayFeature.h \
		RayProject.cpp \
		RayProject.h \
		RaySegment.cpp \
		RaySegment.h \
		Renderer.cpp \
		Renderer.h \
		RenderCamera.cpp \
		RenerCamera.h \
		RenderCamerPyImp.cpp
		RenderLights.h \
		RenderMaterial.cpp \
		RenderMaterial.h \
		RenderFeature.cpp \
		RenderFeature.h \
		RenderFeaturePyImp.cpp \
		Renderer.cpp \
		Renderer.h \
		RenderProcess.cpp \
		RenderProcess.h


FreeCADpov.h: FreeCADpov
	$(PYTHON) $(top_srcdir)/src/Tools/PythonToCPP.py $< $@


# the library search path.
libRaytracing_la_LDFLAGS = -L../../../Base -L../../../App -L../../Part/App -L/usr/X11R6/lib \
		-L$(OCC_LIB) $(all_libraries) -version-info @LIB_CURRENT@:@LIB_REVISION@:@LIB_AGE@
libRaytracing_la_CPPFLAGS = -DAppPartExport= -DAppRaytracingExport= -DFeatureRayExportPov=

libRaytracing_la_LIBADD   = \
		@BOOST_SYSTEM_LIB@ \
		-l@PYTHON_LIB@ \
		-lxerces-c \
		-lFreeCADBase \
		-lFreeCADApp \
		-lTKernel \
		-lTKG2d \
		-lTKG3d \
		-lTKMath \
		-lTKBool \
		-lTKBRep \
		-lTKGeomAlgo \
		-lTKGeomBase \
		-lTKMesh \
		-lPart

%.cpp: %.xml $(top_srcdir)/src/Tools/generateTemplates/templateClassPyExport.py
	$(PYTHON) $(top_srcdir)/src/Tools/generate.py --outputPath $(@D) $<

#--------------------------------------------------------------------------------------
# Loader of libRaytracing

Raytracing_la_SOURCES=\
		AppRaytracing.cpp

# the library search path.
Raytracing_la_LDFLAGS = $(libRaytracing_la_LDFLAGS) -module -avoid-version
Raytracing_la_CPPFLAGS = $(libRaytracing_la_CPPFLAGS)

Raytracing_la_LIBADD   = \
		$(libRaytracing_la_LIBADD) \
		-lRaytracing

Raytracing_la_DEPENDENCIES = libRaytracing.la

#--------------------------------------------------------------------------------------

# set the include path found by configure
AM_CXXFLAGS = -I$(top_srcdir)/src -I$(top_builddir)/src $(all_includes) -I$(OCC_INC)


libdir = $(prefix)/Mod/Raytracing
datadir = $(prefix)/Mod/Raytracing

CLEANFILES = $(BUILT_SOURCES) $(libRaytracing_la_BUILT)

EXTRA_DIST = \
		$(data_DATA) \
		resources/FCAnimation.inc \
		resources/FCAnimation.ini \
		resources/FCAnimation.pov \
		resources/FCSimple.pov \
		CMakeLists.txt \
		FreeCADpov \
		RenderFeaturePy.xml \
		RenderCameraPy.xml
