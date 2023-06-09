# -----------------
# Directories       
# ----------------- 
SET (SRC_DIR "${CMAKE_SOURCE_DIR}/source")
SET (BIN_DIR bin)
SET (INSTALL_DIR install)
SET (CONTENT_DIR content)

SET (OUTPUT_BINDIR "${PROJECT_BINARY_DIR}/${BIN_DIR}/${BUILD_TARGET}")
SET (CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/${INSTALL_DIR}" CACHE STRING "InstallPrefix" FORCE)