#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.4)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

#-----------------------------------------------------------------------------
# Module name
#   - Usually identique to the directory name)
#   - You shouldn't call 'project(...)'. See SlicerConfig.cmake for more details.
#-----------------------------------------------------------------------------
set(MODULE_NAME FacetedVisualizer)

#-----------------------------------------------------------------------------
# Prerequisites
#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  set(MODULE_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Slicer4:Developers:Projects:QtSlicer/Tutorials/ExtensionWriting")
  set(MODULE_CATEGORY "Informatics")
  set(MODULE_STATUS "Beta")
  set(MODULE_DESCRIPTION "Faceted visualizer that combines ontology with a 3D atlas")
  set(MODULE_DEPENDS LoadableExtensionTemplate ) # Specified as a space separated list or 'NA' if any
  set(MODULE_CONTRIBUTORS "Harini Veeraraghavan (GE)")
  set(MODULE_ICONURL "http://viewvc.slicer.org/viewvc.cgi/Slicer4/trunk/Extensions/Testing/LoadableExtensionTemplate/Resources/Icons/LoadableExtensionTemplate.png?revision=19437&view=co")
  set(MODULE_SCREENSHOTURLS "http://wiki.slicer.org/slicerWiki/images/4/42/Slicer-r19441-LoadableExtensionTemplate-screenshot.png")

  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})

  set(MODULE_LICENSE_FILE ${Slicer_LICENSE_FILE})
  set(MODULE_README_FILE ${Slicer_README_FILE})

  include(SlicerEnableExtensionTesting)

endif()


# Module name (usually identique to the directory name)
#set(qt_module_name "FacetedVisualizer")
set(qt_module_title "Faceted Atlas Visualizer")

# Build module sub libraries
add_subdirectory(Logic)

set(qt_module_export_directive "Q_SLICER_QTMODULES_FACETEDVISUALIZER_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  )


# Source files
set(qt_module_SRCS
  qSlicerFacetedVisualizerModule.cxx
  qSlicerFacetedVisualizerModule.h
  qSlicerFacetedVisualizerModuleWidget.cxx
  qSlicerFacetedVisualizerModuleWidget.h
  )

# Headers that should run through moc
set(qt_module_MOC_SRCS
  qSlicerFacetedVisualizerModule.h
  qSlicerFacetedVisualizerModuleWidget.h
  )

# UI files
set(qt_module_UI_SRCS
  Resources/UI/qSlicerFacetedVisualizerModule.ui
  )

# Additional Target libraries
set(qt_module_target_libraries
  vtkSlicerFacetedVisualizerModuleLogic
 )

INCLUDE_DIRECTORIES( ${VTK_SOURCE_DIR}/Utilities/vtksqlite )

# Resources
set(qt_module_resources
  Resources/qSlicerFacetedVisualizerModule.qrc
  )

slicerMacroBuildQtModule(
  NAME ${MODULE_NAME}
#  NAME ${qt_module_name}
  TITLE ${qt_module_title}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories} 
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )


if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()


#-----------------------------------------------------------------------------
# Packaging
#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  include(${Slicer_EXTENSION_CPACK})
endif()

