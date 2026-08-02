cmake_minimum_required(VERSION 3.16)
project(QVTKStandalone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# ---------------------------------------------------------------------------
# VTK (any install; GUISupportQt NOT needed -- we compile the Qt glue ourselves)
# ---------------------------------------------------------------------------
find_package(VTK REQUIRED)
message(STATUS "VTK version: ${VTK_VERSION} (${VTK_DIR})")

# ---------------------------------------------------------------------------
# Qt 5 / Qt 6 switch
#   -DUSE_QT6=ON  -> build with Qt6
#   -DUSE_QT6=OFF -> build with Qt5 (default)
# Qt is located via CMAKE_PREFIX_PATH (set it in the environment or via -D),
# or any other standard Qt discovery mechanism. Nothing is hardcoded here.
# ---------------------------------------------------------------------------
option(USE_QT6 "Use Qt6 instead of Qt5" OFF)

if(USE_QT6)
  find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
  set(QT_VERSION_MAJOR 6)
else()
  find_package(Qt5 REQUIRED COMPONENTS Widgets OpenGL)
  set(QT_VERSION_MAJOR 5)
endif()
message(STATUS "Qt major version: ${QT_VERSION_MAJOR} (prefix: ${CMAKE_PREFIX_PATH})")

# ---------------------------------------------------------------------------
# The copied VTK Qt glue, compiled in your own project (NOT into VTK)
# ---------------------------------------------------------------------------
# Headers are listed explicitly so CMAKE_AUTOMOC picks up every Q_OBJECT
# (QVTKInteractorInternal.h is a private header and is not moc'ed otherwise).
# ---------------------------------------------------------------------------
add_library(qvtk_support STATIC
  src/QVTKOpenGLNativeWidget.cxx
  src/QVTKOpenGLNativeWidget.h
  src/QVTKRenderWindowAdapter.cxx
  src/QVTKRenderWindowAdapter.h
  src/QVTKInteractor.cxx
  src/QVTKInteractor.h
  src/QVTKInteractorAdapter.cxx
  src/QVTKInteractorAdapter.h
  src/QVTKInteractorInternal.h
)

target_include_directories(qvtk_support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# same define VTK's GUISupportQt module uses
target_compile_definitions(qvtk_support PRIVATE QT_NO_KEYWORDS)

target_link_libraries(qvtk_support PUBLIC
  VTK::RenderingOpenGL2   # vtkGenericOpenGLRenderWindow / vtkOpenGLState / FBO
  VTK::RenderingCore      # vtkRenderWindowInteractor
  VTK::CommonCore         # vtkCommand / vtkLogger / vtkNew / vtkSmartPointer
  VTK::InteractionStyle   # vtkInteractorStyleTrackballCamera
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::OpenGL
)
if(QT_VERSION_MAJOR GREATER_EQUAL 6)
  target_link_libraries(qvtk_support PUBLIC Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
endif()

# ---------------------------------------------------------------------------
# Minimal demo executable to verify the whole chain links
# ---------------------------------------------------------------------------
add_executable(qvtk_demo main.cpp)
target_link_libraries(qvtk_demo PRIVATE qvtk_support VTK::IOImage)

# ---------------------------------------------------------------------------
# Static VTK requires object-factory registration (autoinit) so that
# vtkActor::New() / vtkRenderWindow::New() return the OpenGL implementations
# (vtkOpenGLActor / vtkWin32OpenGLRenderWindow ...) instead of the base classes.
# Without this: silent black render / crash.
# If you later link this into your own executable, call vtk_module_autoinit
# on THAT target instead.
# ---------------------------------------------------------------------------
if(TARGET VTK::RenderingOpenGL2)
  vtk_module_autoinit(
    TARGETS qvtk_demo
    MODULES VTK::RenderingOpenGL2 VTK::InteractionStyle)
endif()
