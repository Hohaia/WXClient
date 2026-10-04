cmake_minimum_required(VERSION 4.4)
project(WXClient)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall -Wextra -Werror=parentheses)

find_package(OpenSSL REQUIRED)

# shared CORE backend
add_library(wxclient_core OBJECT

        src/core/control.cpp
        src/core/controller_api.cpp
        src/core/helpers.cpp
        src/core/logger.cpp
        src/core/protocol.cpp
        src/core/status.cpp
        src/core/workflow.cpp

        include/core/control.h
        include/core/controller_api.h
        include/core/helpers.h
        include/core/logger.h
        include/core/protocol.h
        include/core/status.h
        include/core/workflow.h

        include/core/lookup/table_names.h
        include/core/lookup/command/area_control_tables.h
        include/core/lookup/command/door_control_tables.h
        include/core/lookup/command/input_control_tables.h
        include/core/lookup/command/output_control_tables.h
        include/core/lookup/command/control_command_types.h
        include/core/lookup/command/control_command_tables.h
        include/core/lookup/status/area_status_tables.h
        include/core/lookup/status/door_status_tables.h
        include/core/lookup/status/input_status_tables.h
        include/core/lookup/status/output_status_tables.h
        include/core/lookup/status/trouble_input_status_tables.h
        include/core/lookup/status/status_code_types.h
        include/core/lookup/status/status_code_tables.h

        include/vendor/httplib/httplib.h
)
target_include_directories(wxclient_core PUBLIC
        ${CMAKE_SOURCE_DIR}/include/core
        ${CMAKE_SOURCE_DIR}/include/core/lookup
        ${CMAKE_SOURCE_DIR}/include/core/lookup/command
        ${CMAKE_SOURCE_DIR}/include/core/lookup/status
)
target_include_directories(wxclient_core SYSTEM PUBLIC
        ${CMAKE_SOURCE_DIR}/include/vendor/httplib
)
target_link_libraries(wxclient_core PUBLIC
        OpenSSL::SSL OpenSSL::Crypto
)
target_compile_definitions(wxclient_core PUBLIC
        CPPHTTPLIB_OPENSSL_SUPPORT
)

# CLI frontend
add_executable(WXClient_CLI
        src/frontend/cli/main.cpp

        src/frontend/cli/console.cpp
        src/frontend/cli/session.cpp

        include/frontend/cli/console.h
        include/frontend/cli/session.h
)
target_include_directories(WXClient_CLI PRIVATE
        ${CMAKE_SOURCE_DIR}/include/frontend/cli
)
target_link_libraries(WXClient_CLI PRIVATE
        wxclient_core
)

# GUI frontend
add_executable(WXClient_GUI
        src/frontend/gui/main.cpp

        src/frontend/gui/session.cpp

        include/frontend/gui/session.h
)
target_include_directories(WXClient_GUI PRIVATE
        ${CMAKE_SOURCE_DIR}/include/frontend/gui
)
target_link_libraries(WXClient_GUI PRIVATE
        wxclient_core
)