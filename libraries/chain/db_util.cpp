/**
 * DASCOIN!
*/

#include <graphene/chain/database.hpp>

namespace graphene { namespace chain {

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

} }  // namespace graphene::chain