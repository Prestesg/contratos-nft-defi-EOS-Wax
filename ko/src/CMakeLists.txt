project(ko)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( ko ko ko.cpp )
target_include_directories( ko PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( ko ${CMAKE_SOURCE_DIR}/../ricardian )