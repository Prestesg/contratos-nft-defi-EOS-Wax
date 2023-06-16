#include <eosio/eosio.hpp>
#include <eosio/system.hpp>

using namespace eosio;
using namespace std;
CONTRACT kostakedpool : public contract {
   public:
      using contract::contract;

      kostakedpool(name receiver, name code, datastream<const char*> ds)
      : contract(receiver,code,ds){}
      
      //CONSTANTES DO PROJETO 
      static constexpr name KOMBOZACONTRACT = name("kombozapool2");
      static constexpr name KOMBOZASTAKEPOOL = name("kostakepool3");
      //AÇÃO PRIVADA PARA SER EXECUTADA SOMENTE PELA KOMBOZAPOOL
      ACTION unstake( uint64_t assetId, name userId);

      using unstake_action = action_wrapper<"unstake"_n, &kostakedpool::unstake>;
};