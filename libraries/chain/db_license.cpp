/**
 * DASCOIN!
 */

#include <graphene/chain/database.hpp>

#include <graphene/chain/license_objects.hpp>  // TODO: move!
#include <graphene/chain/license_evaluator.hpp>

namespace graphene { namespace chain {

void database::create_license_type(
    const string& name,
    const share_type amount,
    const uint8_t upgrades,
    const uint32_t flags)
{
    create<license_type_object>([&]( license_type_object& lic )
    {
      lic.name = name;
      lic.amount = amount;
      lic.upgrades = upgrades;
      lic.policy_flags = flags;
    });
}

} }  // namespace graphhene::chain
