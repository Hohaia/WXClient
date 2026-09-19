cmake_minimum_required(VERSION 4.4)
project(WXClient)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenSSL REQUIRED)

# CORE backend
add_library(wxclient_core OBJECT

        src/core/controller_api.cpp
        src/core/helpers.cpp
        src/core/logger.cpp
        src/core/workflow.cpp

        include/core/controller_api.h
        include/core/helpers.h
        include/core/logger.h
        include/core/workflow.h

        include/core/menu_tables.h

        include/core/httplib.h
)
target_include_directories(wxclient_core PUBLIC
        ${CMAKE_SOURCE_DIR}/include/core
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
target_link_libraries(WXClient_CLI PRIVATE wxclient_core)

# GUI frontend
add_executable(WXClient_GUI
        src/frontend/gui/main.cpp

        src/frontend/gui/session.cpp

        include/frontend/gui/session.h
)
target_include_directories(WXClient_GUI PRIVATE
        ${CMAKE_SOURCE_DIR}/include/frontend/gui
)
target_link_libraries(WXClient_GUI PRIVATE wxclient_core)