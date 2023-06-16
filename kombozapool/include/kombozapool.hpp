#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/asset.hpp>
#include "atomicdata.hpp"
using namespace eosio;
using namespace std;
using namespace atomicdata;
CONTRACT kombozapool : public contract {
   public:
      using contract::contract;

      kombozapool(name receiver, name code, datastream<const char*> ds)
      : contract(receiver,code,ds)
      ,stakednfts(receiver,receiver.value)
      ,pools(receiver,receiver.value)
      ,users(receiver,receiver.value){}

      TABLE stakednfts_table {
         uint64_t assetId;
         name userId;
         uint64_t poolId;
         time_point_sec created;
         time_point_sec updated;
         time_point_sec lastClaimDate;
         uint64_t primary_key() const { return assetId; }
         uint64_t by_secondary( ) const { return userId.value; }
      };
      
      TABLE users_table {
         name userId;
         uint64_t totalHashPower;
         time_point_sec lastClaimDate;
         uint64_t primary_key() const { return userId.value; }
      };

      TABLE pools_table {
         uint64_t poolId ;
         name poolName ;
         double totalTokens;
         double actualTokens;
         float totalStakedHashpower;
         uint64_t totalPoolSales;
         uint64_t totalDaysDistribution;
         time_point_sec startDistrbuitionDate;
         bool poolActive;
         uint64_t primary_key() const { return poolId; }
      };    

      TABLE assets {
         uint64_t asset_id ;
         name collection_name;
         name schema_name;
         uint32_t template_id;
         name ram_payer;
         uint8_t immutable_serialized_data;
         uint8_t mutable_serialized_data;
         uint64_t primary_key() const { return asset_id; }
      };    

      TABLE templates {
        int32_t  template_id;
        name schema_name;
        bool transferable;
        bool burnable;
        uint32_t max_supply;
        uint32_t issued_supply;
        vector <uint8_t> immutable_serialized_data;
        uint64_t primary_key() const { return (uint64_t) template_id; }
      };

      TABLE schemas {
        name schema_name;
        vector <FORMAT> format;
        uint64_t primary_key() const { return schema_name.value; }
      };


      //CONSTANTES DO PROJETO 
      static constexpr name KOMBOZACONTRACT = name("kombozapool2");
      static constexpr name KOMBOZASTAKEPOOL = name("kostakepool3");
      static constexpr name KOMBOZA_COLLECTION_NAME = name("kombozaaarte");
      static constexpr name KOTOKENSCONTRACT = name("kotokenspool");
      static constexpr name ATOMICASSETSCONTRACT = name("atomicassets");

      //AÇÕES PÚBLICAS
      ACTION stake( name userId, uint64_t assetId,uint64_t poolId );
      ACTION unstake( name userId,uint64_t assetId, uint64_t poolId);
      ACTION claim( name userId,uint64_t poolId );
      ACTION createuser(name userId);
      
      //AÇÕES PRIVADAS 
      ACTION initpool( name poolName,uint64_t totalTokens );
      ACTION setpool( uint64_t poolId,uint64_t totalPoolSales, uint64_t totalDaysDistribution );

      //MULTI INDEX TABLES TYPEDEFS
      typedef multi_index<name("users"), users_table> users_index;
      typedef multi_index<name("pools2"), pools_table> pools_index;
      typedef multi_index<name("stakednfts3"), stakednfts_table, indexed_by<name("byuser"), const_mem_fun<stakednfts_table, uint64_t, &stakednfts_table::by_secondary>>> stakednfts_index;
      typedef multi_index<name("assets"), assets> assets_index;
      typedef multi_index <name("templates"), templates> templates_index;
      typedef multi_index <name("schemas"), schemas> schemas_index;

      //ACTIONS WWAPPERS
      using stake_action = action_wrapper<"stake"_n, &kombozapool::stake>;
      using unstake_action = action_wrapper<"unstake"_n, &kombozapool::unstake>;
      using claim_action = action_wrapper<"claim"_n, &kombozapool::claim>;
      using createuser_action = action_wrapper<"createuser"_n, &kombozapool::createuser>;

      //ADMIN ACTIONS WRAPPERS
      using initpool_action = action_wrapper<"initpool"_n, &kombozapool::initpool>;
      using setpool_action = action_wrapper<"setpool"_n, &kombozapool::setpool>;

      //ATOMICASSETS CONTRACT TABLES
      templates_index atomicassetstemplates = templates_index(ATOMICASSETSCONTRACT , KOMBOZA_COLLECTION_NAME.value);
      schemas_index atomicassetsschemas = schemas_index(ATOMICASSETSCONTRACT , KOMBOZA_COLLECTION_NAME.value);

      //CONTRACTS TABLES
      stakednfts_index stakednfts;
      users_index users;
      pools_index pools;
};