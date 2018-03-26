/**
 * DASCOIN!
*/

#include <graphene/chain/database.hpp>

namespace graphene { namespace chain {

share_type database::cycles_to_dascoin(share_type cycles, share_type frequency) const
{
  return (cycles * DASCOIN_DEFAULT_ASSET_PRECISION * DASCOIN_FREQUENCY_PRECISION) / frequency;
}

share_type database::dascoin_to_cycles(share_type dascoin, share_type frequency) const
{
  return dascoin * frequency / (DASCOIN_DEFAULT_ASSET_PRECISION * DASCOIN_FREQUENCY_PRECISION);
}

void database::perform_chain_authority_check(const string& auth_type_name, account_id_type auth_id,
                                             const account_object& acc_obj) const
{
  FC_ASSERT( acc_obj.id == auth_id,
             "Operation must be signed by ${auth_type} authority '${auth_name}', signed by '${a}' instead'",
             ("auth_type", auth_type_name)
             ("auth_name", auth_id(*this).name)
             ("a", acc_obj.name)
          );
}

void database::remove_limit_from_all_vaults()
{
   const auto& accounts_by_id = get_index_type<account_index>().indices().get<by_id>();

   for(auto itr = accounts_by_id.begin(); itr != accounts_by_id.end(); itr++)
   {
      if(itr->kind == account_kind::vault)
      {
         modify(*itr,[](account_object& account_obj){
            account_obj.disable_vault_to_wallet_limit = true;
         });
      }
   }

}

} }  // namespace graphene::chain
