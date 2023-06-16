project(kombozapool)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( kombozapool kombozapool kombozapool.cpp )
target_include_directories( kombozapool PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( kombozapool ${CMAKE_SOURCE_DIR}/../ricardian )