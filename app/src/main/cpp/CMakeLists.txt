cmake_minimum_required(VERSION 3.19.0)
set(ANDROID_PLATFORM 31)
set(CIPHER_URL https://github.com/lukas0x1/SML-filehost/releases/download/v1.2.0/libciphered.so)
set(CIPHER_PATH ${CMAKE_BINARY_DIR}/libciphered.so)
file(DOWNLOAD ${CIPHER_URL} ${CIPHER_PATH} STATUS LIBCIPHER_DOWNLOAD_STATUS)

project("userlib")

file(READ ${CMAKE_HOME_DIRECTORY}/config.json MY_JSON_STRING) #reads soname from config
string(JSON CUR_NAME GET ${MY_JSON_STRING} name)
string(LENGTH ${CUR_NAME} CUR_NAME_LEN)
string(SUBSTRING ${CUR_NAME} 3 ${CUR_NAME_LEN} CUR_NAME) #ugly but there's no lazy regex support
string(REPLACE ".so" "" CUR_NAME ${CUR_NAME})

add_library( # Sets the name of the library.
        ${CUR_NAME}
        SHARED
        main.cpp
        spiritshop.cpp
        lights.cpp
        ${CMAKE_SOURCE_DIR}/classes_dex.o
        )

add_custom_command(
        COMMAND ${CMAKE_LINKER}
        ARGS -r -b binary -m aarch64elf classes.dex -o ${CMAKE_SOURCE_DIR}/classes_dex.o
        OUTPUT ${CMAKE_SOURCE_DIR}/classes_dex.o
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating classes_dex.o"
        VERBATIM
        )

set_target_properties(${CUR_NAME}
        PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib )

find_library(
        log-lib
        log)

target_link_libraries(
        ${CUR_NAME}
        "${CIPHER_PATH}"
        ${log-lib}
        )

add_custom_command(
        TARGET ${CUR_NAME}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
        ARGS --add-section .config=${CMAKE_HOME_DIRECTORY}/config.json --set-section-flags .config=readonly,contents,data $<TARGET_FILE:${CUR_NAME}>
)
