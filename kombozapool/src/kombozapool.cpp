#include <kombozapool.hpp>
#include <iomanip>
#include <cmath>
#include "atomicdata.hpp"

using namespace std;
using namespace chrono;
using namespace atomicdata;

ACTION kombozapool::initpool( name poolName,uint64_t totalTokens ) {
   eosio::internal_use_do_not_use::require_auth2(KOMBOZACONTRACT.value, "owner"_n.value);
   pools.emplace(_self,[&](auto& rec){
      rec.poolId = pools.available_primary_key();
      rec.poolName = poolName;
      rec.totalTokens = totalTokens;
      rec.actualTokens = totalTokens;
      rec.totalStakedHashpower = 0;
      rec.totalPoolSales = 0;
      rec.totalDaysDistribution = 730;
      rec.startDistrbuitionDate = current_time_point();
      rec.poolActive = false;
   });
}

ACTION kombozapool::setpool( uint64_t poolId,uint64_t totalPoolSales, uint64_t totalDaysDistribution) {
  eosio::internal_use_do_not_use::require_auth2(KOMBOZACONTRACT.value, "owner"_n.value);
  auto itr = pools.find(poolId);
  check( itr != pools.end(), "pool does not exist in table" );

   pools.modify( itr, _self, [&]( auto& row ) {
     row.totalPoolSales = totalPoolSales;
     row.totalDaysDistribution = totalDaysDistribution;
   });
}

ACTION kombozapool::createuser(name userId) {
   require_auth( userId );
   auto itr = users.find(userId.value);
   if(itr == users.end()) {
      users.emplace(_self,[&](auto& rec){
         rec.userId = userId;
         rec.totalHashPower = 0;
         rec.lastClaimDate = current_time_point();
      });
   } else {
      check(false,"user already registred");
   }
}

ACTION kombozapool::unstake(name userId,uint64_t assetId, uint64_t poolId) {
   //PEGA O ID DO USUÁRIO
   require_auth( userId );

   auto itr = stakednfts.find(assetId);

   check(itr != stakednfts.end(),"assetId is not staked");
   
   auto itrPool = pools.find(poolId);
   check( itrPool != pools.end(), "pool does not exist in table" );

   auto itrUsers = users.find(userId.value);
   check(itrUsers != users.end(),"user register not set");
   
   assets_index atomicassets = assets_index(ATOMICASSETSCONTRACT , KOMBOZASTAKEPOOL.value );
   auto itrAtomicassets = atomicassets.find(assetId);
   auto itrNftTemplate = atomicassetstemplates.find(itrAtomicassets->template_id);
   auto itrSchemas = atomicassetsschemas.find(itrNftTemplate->schema_name.value);
   auto nftHashpowerDes = deserialize(itrNftTemplate->immutable_serialized_data,itrSchemas->format) ;
   double nftHashpower = get<double>(nftHashpowerDes["hashpower"]);
  
   action(
        permission_level{KOMBOZACONTRACT, name("active")},
        KOMBOZASTAKEPOOL,
        name("unstake"),
        std::make_tuple(
            assetId,
            userId
        )
      ).send();

   users.modify( itrUsers, _self, [&]( auto& row ) {
      row.totalHashPower -= nftHashpower;
   });

   pools.modify( itrPool, _self, [&]( auto& row ) {
      row.totalStakedHashpower -= nftHashpower;
   });
   
   stakednfts.erase( itr );
}

ACTION kombozapool::stake( name userId,uint64_t assetId, uint64_t poolId) {
   require_auth( userId );
   //VERIFICA SE O USUÁRIO JA ESTÁ CADASTRADO 
   auto itrUsers = users.find(userId.value);
   check(itrUsers != users.end(),"user register not set");

   //VERIFICA SE O POOL CHAMADA EXISTE
   auto itrPool = pools.find(poolId);
   check( itrPool != pools.end(), "pool does not exist in table" );

   // VALIDA SE NFT EXISTE NA ATOMICASSETS   
   assets_index atomicassets = assets_index(ATOMICASSETSCONTRACT , userId.value );
   auto itrAtomicassets = atomicassets.find(assetId);
   check(itrAtomicassets != atomicassets.end(),"nft doesn't exist");

   //VALIDA SE A COLEÇÃO DO NFT É VALIDA
   name collectionName = itrAtomicassets->collection_name;
   check(collectionName == KOMBOZA_COLLECTION_NAME,"nft doesn't belong to valid collections");

   //VERIFICA SE O NFT JA ESTÁ NA POOL
   auto itrStakedNfts = stakednfts.find(assetId);
   check(itrStakedNfts == stakednfts.end(),"nft already staked");

   //TRANSFERE O NFT PARA CARTEIRA DE NFT'S DA POOL
   vector<uint64_t> asset_ids;
   asset_ids.push_back(assetId);
   action(
        permission_level{userId, name("active")},
        name("atomicassets"),
        name("transfer"),
        std::make_tuple(
            userId,
            KOMBOZASTAKEPOOL,
            asset_ids,
            string("")
        )
      ).send();

   //ADICIONA NA TABELA REGISTRO NA TABELA STAKED NFT'S
   if (itrStakedNfts == stakednfts.end()){
      stakednfts.emplace(_self,[&](auto& rec){
         rec.assetId = assetId;
         rec.userId = userId;
         rec.poolId = itrPool->poolId;
         rec.created = current_time_point();
         rec.updated = current_time_point();
         rec.lastClaimDate= current_time_point();
      });
      auto itrNftTemplate = atomicassetstemplates.find(itrAtomicassets->template_id);
      auto itrSchemas = atomicassetsschemas.find(itrNftTemplate->schema_name.value);
      auto nftHashpowerDes = deserialize(itrNftTemplate->immutable_serialized_data,itrSchemas->format) ;
      double nftHashpower = get<double>(nftHashpowerDes["hashpower"]);
      users.modify( itrUsers, _self, [&]( auto& row ) {
         row.totalHashPower += nftHashpower;
      });

      pools.modify( itrPool, _self, [&]( auto& row ) {
         row.totalStakedHashpower += nftHashpower;
      });
   }
}

ACTION kombozapool::claim(name userId, uint64_t poolId ) {
   require_auth( userId );
   //PESQUISA TODOS OS NFT'S DO USUÁRIO STAKADOS NA POOL
   auto itrStakedNfts = stakednfts.get_index<name("byuser")>();
   //auto itrFindUser = itrStakedNfts.find(userId.value);
   //check(itrFindUser != stakednfts.end(),"nft doesn't exist for claim ");

   //PESQUISA O PESO DO HASHPOWER DE CADA NFT DO USUARIO NA POOL
   auto itr_start = itrStakedNfts.lower_bound(userId.value);
   auto itr_end = itrStakedNfts.upper_bound(userId.value);

   //PESQUISA QUAL A DISTRIBUIÇÃO DIÁRIA - FEITO
   auto itrPool = pools.find(poolId); 
   uint64_t totalStakedHashpower = itrPool->totalStakedHashpower;
   double totalTokens = itrPool->totalTokens;
   double totalPoolSales = itrPool->totalPoolSales;
   double totalDaysDistribution = itrPool->totalDaysDistribution;   

   //FUNÇÃO QUE CALCULA OS DAILY TOKENS
   double totalTokensPerDay = totalTokens / totalDaysDistribution;
   double dailyTokens = totalTokensPerDay * totalPoolSales / 100;

   //ITERA POR TODOS OS VALORES DA TABELA
   double tokenToReceive = 0;
   for (;itr_start != itr_end; itr_start++){
      //CALCULA DIFERENÇA DAS DATAS EM MINUTOS 
      auto diff = current_time_point() - itr_start->lastClaimDate;
      long milliseconds   = (long) (diff.count() / 1000) % 1000;
      long seconds    = (((long) (diff.count() / 1000) - milliseconds)/1000)%60 ;
      long minutes    = (((((long) (diff.count() / 1000) - milliseconds)/1000) - seconds)/60) %60 ;

      //PESQUISA O HASHPOWER DO NFT NA ATOMIC ASSETS
      auto itr = stakednfts.find(itr_start->assetId);
      assets_index atomicassets = assets_index(ATOMICASSETSCONTRACT , KOMBOZASTAKEPOOL.value );
      auto itrAtomicassets = atomicassets.find(itr_start->assetId);
      auto itrNftTemplate = atomicassetstemplates.find(itrAtomicassets->template_id);
      auto itrSchemas = atomicassetsschemas.find(itrNftTemplate->schema_name.value);
      auto nftHashpowerDes = deserialize(itrNftTemplate->immutable_serialized_data,itrSchemas->format) ;
      double nftHashpower = get<double>(nftHashpowerDes["hashpower"]);

      //FUNÇÃO QUE CALCULA O PESO DO NFT PERANTE A POOL 
      float weightNftPercentage =  (100 * nftHashpower) / totalStakedHashpower; 

      //CALCULA QUANTOS TOKENS O USUARIO IRÁ RECEBER - FEITO
      double minuteDistribuitionFromNft = (dailyTokens * weightNftPercentage)/24/60;
      double totalTokens = minutes * minuteDistribuitionFromNft; 
      tokenToReceive += totalTokens;
      //ATUALIZAR REGISTRO DO NFT NO CAMPO LAST CLAIM DATE - FEITO
      //stakednfts.modify( itr, _self, [&]( auto& row ) {
      //   row.lastClaimDate =  current_time_point();
      //});   
      print(nftHashpower,tokenToReceive);
   }
   check(tokenToReceive >= 0,"nothing to be claimed");
   //TRANSFERIR TOKENS PARA O USUARIO - MEDIUM - ?
   //asset assetTransfer(tokenToReceive * 100, symbol("KT",4));

   /*
   action(
        permission_level{KOMBOZACONTRACT, name("active")},
        KOTOKENSCONTRACT,
        name("transfer"),
        std::make_tuple(
            KOTOKENSCONTRACT,
            userId,
            assetTransfer,
            string("")
        )
      ).send();
   
   //ATUALIZA QUANTIDADE ATUAL DE TOKENS NA POOL
   pools.modify( itrPool, _self, [&]( auto& row ) {
     row.actualTokens -= tokenToReceive;
   });

   //ATULIZA LAST CLAIM DATE DO USUARIO
   auto itrUser = users.find(userId.value);
   users.modify( itrUser, _self, [&]( auto& row ) {
     row.lastClaimDate = current_time_point();
   });
   */
}