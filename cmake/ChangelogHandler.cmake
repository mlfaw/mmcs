
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
include(CreateCResources)
resources_file_raw("${GEN_FOLDER}/Changelog.zip" "${GEN_FOLDER}/Changelog")
