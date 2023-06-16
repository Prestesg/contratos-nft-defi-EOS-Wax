project(kostakedpool)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( kostakedpool kostakedpool kostakedpool.cpp )
target_include_directories( kostakedpool PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( kostakedpool ${CMAKE_SOURCE_DIR}/../ricardian )