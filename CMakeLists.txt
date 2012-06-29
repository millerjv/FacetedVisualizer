cmake_minimum_required(VERSION 2.8.7)

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  set(EXTENSION_NAME FacetedVisualizer)
  set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/4.1/Extensions/FacetedVisualizer")
  set(EXTENSION_CATEGORY "Informatics")
  set(EXTENSION_CONTRIBUTORS "Harini Veeraraghavan (GE)")
  set(EXTENSION_DESCRIPTION "Faceted search visualizer that combines an ontology with a 3D atlas")
  set(EXTENSION_ICONURL "https://github.com/millerjv/FacetedVisualizer/raw/960b9540cb84431dae1258b7234af0ced462e815/Resources/Icons/FacetedVisualizer.png")
  set(EXTENSION_SCREENSHOTURLS "http://www.slicer.org/slicerWiki/images/5/50/ComplexQueryWithImage.jpg http://www.slicer.org/slicerWiki/images/0/00/ComplexQueryAPI.jpg http://www.slicer.org/slicerWiki/images/f/f0/LimbicSystemWithImage.jpg")
  set(EXTENSION_LICENSE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/License.txt)
  set(EXTENSION_README_FILE ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
endif()

#-----------------------------------------------------------------------------
set(MODULE_NAME FacetedVisualizer)
set(MODULE_TITLE "Faceted Visualizer")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
endif()

#-----------------------------------------------------------------------------
add_subdirectory(Logic)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  )

set(MODULE_SRCS
  qSlicerFacetedVisualizerModule.cxx
  qSlicerFacetedVisualizerModule.h
  qSlicerFacetedVisualizerModuleWidget.cxx
  qSlicerFacetedVisualizerModuleWidget.h
  )

set(MODULE_MOC_SRCS
  qSlicerFacetedVisualizerModule.h
  qSlicerFacetedVisualizerModuleWidget.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicerFacetedVisualizerModule.ui
  )

set(MODULE_TARGET_LIBRARIES
  vtkSlicerFacetedVisualizerModuleLogic
  )

INCLUDE_DIRECTORIES( ${VTK_SOURCE_DIR}/Utilities/vtksqlite )

set(MODULE_RESOURCES
  Resources/qSlicerFacetedVisualizerModule.qrc
  )

#-----------------------------------------------------------------------------
slicerMacroBuildQtModule(
  NAME ${MODULE_NAME}
  TITLE ${MODULE_TITLE}
  EXPORT_DIRECTIVE ${MODULE_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES} 
  SRCS ${MODULE_SRCS}
  MOC_SRCS ${MODULE_MOC_SRCS}
  UI_SRCS ${MODULE_UI_SRCS}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  RESOURCES ${MODULE_RESOURCES}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  include(${Slicer_EXTENSION_CPACK})
endif()

