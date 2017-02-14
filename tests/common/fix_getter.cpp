/**
 * DASCOIN!
 */
#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

#include <graphene/account_history/account_history_plugin.hpp>
#include <graphene/market_history/market_history_plugin.hpp>

#include <graphene/db/simple_index.hpp>

// #include <graphene/chain/account_object.hpp>
// #include <graphene/chain/asset_object.hpp>
// #include <graphene/chain/committee_member_object.hpp>
// #include <graphene/chain/fba_object.hpp>
// #include <graphene/chain/license_objects.hpp>
// #include <graphene/chain/market_object.hpp>
// #include <graphene/chain/vesting_balance_object.hpp>
// #include <graphene/chain/witness_object.hpp>

#include <graphene/utilities/tempdir.hpp>

#include <fc/crypto/digest.hpp>
#include <fc/smart_ref_impl.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "database_fixture.hpp"

namespace graphene { namespace chain {

const global_property_object& database_fixture::get_global_properties() const
{
  return db.get_global_properties();
}

const dynamic_global_property_object& database_fixture::get_dynamic_global_properties() const
{
  return db.get_dynamic_global_properties();
}

const chain_parameters& database_fixture::get_chain_parameters() const
{
  return db.get_global_properties().parameters;
}

account_id_type database_fixture::get_license_administrator_id() const
{
  return db.get_global_properties().authorities.license_administrator;
}

account_id_type database_fixture::get_license_issuer_id() const
{
  return db.get_global_properties().authorities.license_issuer;
}

account_id_type database_fixture::get_license_authenticator_id() const
{
  return db.get_global_properties().authorities.license_authenticator;
}

account_id_type database_fixture::get_webasset_issuer_id() const
{
  return db.get_global_properties().authorities.webasset_issuer;
}

account_id_type database_fixture::get_webasset_authenticator_id() const
{
  return db.get_global_properties().authorities.webasset_authenticator;
}

account_id_type database_fixture::get_cycle_issuer_id() const
{
  return db.get_global_properties().authorities.cycle_issuer;
}

account_id_type database_fixture::get_cycle_authenticator_id() const
{
  return db.get_global_properties().authorities.cycle_authenticator;
}

account_id_type database_fixture::get_registrar_id() const
{
  return db.get_global_properties().authorities.registrar;
}

account_id_type database_fixture::get_pi_validator_id() const
{
  return db.get_global_properties().authorities.pi_validator;
}

account_id_type database_fixture::get_wire_out_handler_id() const
{
  return db.get_global_properties().authorities.wire_out_handler;
}

asset_id_type database_fixture::get_web_asset_id() const
{
  return db.get_web_asset_id();
}

asset_id_type database_fixture::get_dascoin_asset_id() const
{
  return db.get_dascoin_asset_id();
}

frequency_type database_fixture::get_global_frequency() const
{
  return db.get_dynamic_global_properties().frequency;
}

} }  // namespace graphene::chain
