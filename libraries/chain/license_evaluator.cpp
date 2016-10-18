/**
 * DASCOIN!
 */
#include <graphene/chain/license_evaluator.hpp>

namespace graphene { namespace chain {

//////////////////////
// Private methods: //
//////////////////////

void assert_license_authenticator(const database& db, account_id_type account)
{
	FC_ASSERT( account == db.get_global_properties().authorities.license_authenticator );
}

void assert_license_issuer(const database& db, account_id_type account)
{
  FC_ASSERT( account == db.get_global_properties().authorities.license_issuer );
}

////////////////////////////
// License type creation: //
////////////////////////////

void_result license_type_create_evaluator::do_evaluate(const license_type_create_operation& op)
{ try {

  assert_license_authenticator(db(), op.license_authentication_account);
  return void_result();

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result license_type_create_evaluator::do_apply(const license_type_create_operation& op)
{ try {

  db().create_license_type( op.name, op.amount, op.upgrades, op.policy_flags );

  return void_result();

} FC_CAPTURE_AND_RETHROW( (op) ) }

///////////////////////////
// License type editing: //
///////////////////////////

void_result license_type_edit_evaluator::do_evaluate(const license_type_edit_operation& op)
{ try {

  assert_license_authenticator(db(), op.license_authentication_account);
  return void_result();

}  FC_CAPTURE_AND_RETHROW( (op)) }

void_result license_type_edit_evaluator::do_apply(const license_type_edit_operation& op)
{ try {

  db().modify( db().get(op.license), [&]( license_type_object& lic ) {
    if (op.name.valid()) lic.name = *op.name;
    if (op.amount.valid()) lic.amount = *op.amount;
    if (op.upgrades.valid()) lic.upgrades = *op.upgrades;
    if (op.policy_flags.valid()) lic.policy_flags = *op.policy_flags;
  });

  return void_result();

} FC_CAPTURE_AND_RETHROW( (op) ) }

////////////////////////////
// License type deletion: //
////////////////////////////

void_result license_type_delete_evaluator::do_evaluate(const license_type_delete_operation& op)
{ try {

  assert_license_authenticator( db(), op.license_authentication_account );
  return void_result();

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result license_type_delete_evaluator::do_apply(const license_type_delete_operation& op)
{ try {

  // TODO: to do a meaningfull delete, all accounts with a license must have the license_id removed!
  return void_result();

} FC_CAPTURE_AND_RETHROW( (op) ) }

////////////////////////////
// License issue request: //
////////////////////////////

void_result license_request_evaluator::do_evaluate(const license_request_operation& op)
{ try {

  const auto& _db = db();
  const auto& acc = op.account(_db);
  const auto& new_lic = op.license(_db);

  assert_license_issuer( _db, op.license_issuing_account );

  if ( acc.license.valid() )
  {
    const auto& cur_lic = (*acc.license)(_db);

    ilog( "Current license for ${a} account: ${n} id = ${id}, flags = ${f}",
      ("a", acc.name)
      ("n", cur_lic.name)
      ("id", cur_lic.id)
      ("f", cur_lic.policy_flags) );

    if ( acc.is_chartered )
      FC_ASSERT( !new_lic.is_chartered() );  // A chartered account can only have one license!
    FC_ASSERT( cur_lic < new_lic );
  }

  ilog( "New license for ${a} account: ${n} id = ${id}, flags = ${f}",
    ("a", acc.name)
    ("n", new_lic.name)
    ("id", new_lic.id)
    ("f", new_lic.policy_flags) );

  _account_obj = &acc;
  return void_result();

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result license_request_evaluator::do_apply(const license_request_operation& op)
{ try {

  auto& _db = db();

  _db.create<license_request_object>([&] (license_request_object &o) {
    o.license_issuing_account = op.license_issuing_account;
    o.account = op.account;
    o.license = op.license;
    o.expiration = fc::time_point::now() + fc::minutes(2);  // TODO: Final value here.
  });

  return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

///////////////////////////////
// License request approval: //
///////////////////////////////

void_result license_approve_evaluator::do_evaluate(const license_approve_operation& op)
{ try {

  const auto& _db = db();

  assert_license_authenticator( _db, op.license_authentication_account );

  _request_obj = &op.request(_db);
  _account_obj = &_request_obj->account(_db);
  _license_obj = &_request_obj->license(_db);

  return void_result();

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result license_approve_evaluator::do_apply(const license_approve_operation& op)
{ try {
  auto& _db = db();

  _db.modify( *_account_obj, [&](account_object& a) {
    a.license = _license_obj->id;

    ilog( "Applying license object ${n} for account ${a}", ("n", _license_obj->name)("a", a.name) );

    if ( _license_obj->is_chartered() )
    {
      a.is_chartered = true;
      // Chartered licenses get a account level frequency lock.
      a.account_frequency = _request_obj->account_frequency;
    }
  });

  // Chartered licenses do not receive the cycle balance.
  if ( !_license_obj->is_chartered() )
    _db.adjust_cycle_balance( _account_obj->id, _license_obj->amount, { _license_obj->upgrades });

  return void_result();

} FC_CAPTURE_AND_RETHROW( (op) ) }

/////////////////////////////
// License request denial: //
/////////////////////////////

void_result license_deny_evaluator::do_evaluate(const license_deny_operation& op)
{ try {

  const auto& _db = db();

  assert_license_authenticator( _db, op.license_authentication_account );

  _request_obj = &op.request(_db);

  return void_result();

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result license_deny_evaluator::do_apply(const license_deny_operation& op)
{ try {

  // TODO: if additional processing is required, do it in database::deny_license_request().
  db().remove(*_request_obj);

  return void_result();

} FC_CAPTURE_AND_RETHROW( (op) ) }

} } // namespace graphene::chain
