#include <kostakedpool.hpp>

ACTION kostakedpool::unstake( uint64_t assetId,name userId ) {
   //VERIFICA SE É UM USUÁRIO CADASTRADO
   name from = get_self();
   require_auth(KOMBOZACONTRACT);
   //eosio::internal_use_do_not_use::require_auth2(KOMBOZACONTRACT.value, "active"_n.value);
   //TRANSFERE O NFT PARA CARTEIRA DE NFT'S DA POOL
   vector<uint64_t> asset_ids;
   asset_ids.push_back(assetId);

   action(
        permission_level{from, name("active")},
        name("atomicassets"),
        name("transfer"),
        std::make_tuple(
            KOMBOZASTAKEPOOL,
            userId,
            asset_ids,
            string("")
        )
      ).send();
}