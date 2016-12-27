/**
 * DASCOIN!
 */

#include <graphene/chain/database.hpp>

#include <graphene/chain/license_objects.hpp>
#include <graphene/chain/license_evaluator.hpp>

namespace graphene { namespace chain {

object_id_type database::create_license_type(const string& name, share_type amount, const policy_type& policy)
{
  return create<license_type_object>([&]( license_type_object& lic )
  {
    lic.name = name;
    lic.amount = amount;

    // START PARCING LICENSE POLICY:
    //
    // 1) Handle the kind of the license:
    //    TYPE: string
    //    VALUES: standard, charter, pro
    auto it = policy.find("kind");
    if ( it != policy.end() )
    {
      string k = it->second.as_string();
      if ( k == "regular" )
        lic.kind = license_kind::regular;
      else if ( k == "chartered")
        lic.kind = license_kind::chartered;
      else if ( k == "promo" )
        lic.kind = license_kind::promo;
      else
        FC_THROW( "License ${n} cannot have kind of ${k}", ("n", name)("k", k) );
    }
    // 2) Fetch the balance upgrades:
    //    TYPE: vector<uint8_t>
    it = policy.find("balance_upgrades");
    if ( it != policy.end() )
    {
      vector<uint8_t> v;
      fc::from_variant<uint8_t>(it->second, v);
      lic.balance_upgrade.reset(v);
    }
    // 3) Fetch the requeue upgrades:
    //    TYPE: vector<uint8_t>
    it = policy.find("requeue_upgrades");
    if ( it != policy.end() )
    {
      vector<uint8_t> v;
      fc::from_variant<uint8_t>(it->second, v);
      lic.requeue_upgrade.reset(v);
    }
    // 4) Fetch the return upgrades:
    //    TYPE: vector<uint8_t>
    it = policy.find("return_upgrades");
    if ( it != policy.end() )
    {
      vector<uint8_t> v;
      fc::from_variant<uint8_t>(it->second, v);
      lic.return_upgrade.reset(v);
    }
    //
    // DONE PARSING POLICY.

  }).id;
}

void database::fulfill_license_request(const license_request_object& req)
{
  const auto& account_obj = req.account(*this);
  const auto& new_license_obj = req.license(*this);

  // We need to modify the account in order to change the license info.
  modify(account_obj, [&](account_object& a) {
    auto& info = a.license_info;

    // Add the license to the top of the history, so it becomes the new active license:
    info.add_license(new_license_obj.id, req.frequency);

    // Improve all the upgrades to match the new license:
    info.balance_upgrade += new_license_obj.balance_upgrade;
    info.requeue_upgrade += new_license_obj.requeue_upgrade;
    info.return_upgrade += new_license_obj.return_upgrade;

    // The license is no longer pending:
    info.pending_license.reset();
  });

  // For regular licenses, increase the cycle balance for the appropriate amount:
  if ( new_license_obj.kind == license_kind::regular )
    issue_cycles(account_obj.id, new_license_obj.amount);
}

} }  // namespace graphhene::chain
