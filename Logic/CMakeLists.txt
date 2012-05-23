project(vtkSlicer${qt_module_name}ModuleLogic)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#

set(module_logic_name "${PROJECT_NAME}")

# The header '${module_logic_name}Export.h' will be automatically configured.
set(module_logic_export_directive "VTK_SLICER_FACETEDVISUALIZER_MODULE_LOGIC_EXPORT")

# Additional directories to include
set(module_logic_include_directories
  
  INCLUDE_DIRECTORIES( ${VTK_SOURCE_DIR}/Utilities/vtksqlite )
  
  )

# Source files
set(module_logic_SRCS
  vtkSlicerFacetedVisualizerLogic.cxx
  vtkSlicerFacetedVisualizerLogic.h
  )

# Additional Target libraries
set(module_logic_target_libraries
  ${ITK_LIBRARIES}
  )
  
#INCLUDE_DIRECTORIES( ${VTK_SOURCE_DIR}/Utilities/vtksqlite )

SlicerMacroBuildModuleLogic(
  NAME ${module_logic_name}
  EXPORT_DIRECTIVE ${module_logic_export_directive}
  INCLUDE_DIRECTORIES ${module_logic_include_directories}
  SRCS ${module_logic_SRCS}
  TARGET_LIBRARIES ${module_logic_target_libraries}
  )
