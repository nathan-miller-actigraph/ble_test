cmake_minimum_required (VERSION 2.6)
project (ble_test)

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -Wno-long-long -D_GNU_SOURCE")
endif()

set(ble_test_VERSION_MAJOR 1)
set(ble_test_VERSION_MINOR 0)
set(ble_test_VERSION_PATCH 0)
set(ble_test_VERSION_STRING ${ble_test_VERSION_MAJOR}.${ble_test_VERSION_MINOR}.${ble_test_VERSION_PATCH})

add_definitions(-DVERSION="5.48")

set(AGBLE_SRC
    ble_test.c
    bluez/src/shared/util.c
    bluez/src/shared/queue.c
    bluez/btio/btio.c
    bluez/lib/sdp.c
    bluez/src/log.c
    bluez/attrib/gatt.c
    bluez/attrib/att.c
    bluez/attrib/utils.c
    bluez/lib/uuid.c
    bluez/src/shared/att.c
    bluez/src/shared/io-glib.c
    bluez/src/shared/crypto.c
    bluez/src/shared/timeout-glib.c
    bluez/attrib/gattrib.c
    bluez/lib/hci.c
    )

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)

add_executable(ble_test ${AGBLE_SRC})
target_include_directories(ble_test PRIVATE
    ${GLIB_INCLUDE_DIRS}
    bluez/
    )

target_link_libraries(ble_test bluetooth ${GLIB_LIBRARIES})
set_target_properties(ble_test PROPERTIES VERSION ${ble_test_VERSION_STRING} SOVERSION ${ble_test_VERSION_MAJOR})

install(TARGETS ble_test DESTINATION bin)
