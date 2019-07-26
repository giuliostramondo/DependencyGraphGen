# If we don't need RTTI or EH, there's no reason to export anything
# from the hello plugin.
if( NOT LLVM_REQUIRES_RTTI )
  if( NOT LLVM_REQUIRES_EH )
      set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/DependencyGraphGen.exports)
  endif()
endif()

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS Core Support)
endif()

link_libraries(sqlite3)

add_llvm_library( LLVMDependencyGraph MODULE BUILDTREE_ONLY
  register_file_model.cpp
  resource_database_interface.cpp
  DependencyGraphGen.cpp
  DependencyGraph.cpp
  Architecture.cpp
  mem_comp_paramJSON.cpp
  Graph_Utils.cpp
  DEPENDS 
  intrinsics_gen
  PLUGIN_TOOL
  opt
  )
