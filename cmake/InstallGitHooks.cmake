# Install Git hooks during CMake configure
# Usage: include(cmake/InstallGitHooks.cmake)

# Only if inside a git repository
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    message(STATUS "Git repository detected, installing git hooks...")

    set(HOOKS_DIR "${CMAKE_SOURCE_DIR}/.githooks")
    set(GIT_HOOKS_DIR "${CMAKE_SOURCE_DIR}/.git/hooks")

    file(GLOB HOOK_FILES "${HOOKS_DIR}/*")

    foreach(HOOK_FILE ${HOOK_FILES})
        get_filename_component(HOOK_NAME ${HOOK_FILE} NAME)
        set(TARGET_HOOK "${GIT_HOOKS_DIR}/${HOOK_NAME}")

        # Copy hook file
        configure_file(${HOOK_FILE} ${TARGET_HOOK} COPYONLY)

        # Ensure executable permission
        if(NOT WIN32)
            execute_process(COMMAND chmod +x ${TARGET_HOOK})
        endif()

        message(STATUS "Installed git hook: ${HOOK_NAME}")
    endforeach()
endif()
