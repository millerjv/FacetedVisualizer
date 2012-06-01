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
  set(MODULE_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/4.1/Extensions/FacetedVisualizer")
  set(MODULE_CATEGORY "Informatics")
  set(MODULE_STATUS "Beta")
  set(MODULE_DESCRIPTION "Faceted visualizer that combines ontology with a 3D atlas")
  set(MODULE_DEPENDS NA) # Specified as a space separated list or 'NA' if any
  set(MODULE_CONTRIBUTORS "Harini Veeraraghavan (GE)")
  set(MODULE_ICONURL "https://github.com/millerjv/FacetedVisualizer/blob/960b9540cb84431dae1258b7234af0ced462e815/Resources/Icons/FacetedVisualizer.png")
  set(MODULE_SCREENSHOTURLS "http://www.slicer.org/slicerWiki/images/5/50/ComplexQueryWithImage.jpg http://www.slicer.org/slicerWiki/images/0/00/ComplexQueryAPI.jpg http://www.slicer.org/slicerWiki/images/f/f0/LimbicSystemWithImage.jpg")
  set(MODULE_ACKNOWLEDGEMENTS "This work is part of the Neuroimage Analysis Center (NAC), an NIBIB National Resource Center, Grant P41 EB015902")

  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})

  set(MODULE_LICENSE_FILE "https://github.com/millerjv/FacetedVisualizer/blob/a4691acfe0ea72e321cbfe8cd3ca588f4f0ecd96/License.txt")
  set(MODULE_README_FILE "https://github.com/millerjv/FacetedVisualizer/blob/a81d0515c60576577692c2165eb8267c8b71e82c/README.md")
endif()


# Module name (usually identique to the directory name)
#set(qt_module_name "FacetedVisualizer")
#set(qt_module_title "Faceted Atlas Visualizer")
set(qt_module_title "Faceted Visualizer")

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

