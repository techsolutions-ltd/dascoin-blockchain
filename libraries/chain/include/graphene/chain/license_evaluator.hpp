/**
 * DASCOIN!
 */
#pragma once

#include <graphene/chain/license_objects.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/exceptions.hpp>


namespace graphene { namespace chain {

class create_license_type_evaluator : public evaluator<create_license_type_evaluator>
{
public:
  typedef create_license_type_operation operation_type;

  void_result do_evaluate(const create_license_type_operation& op);
  object_id_type do_apply(const create_license_type_operation& op);
};

class issue_license_evaluator : public evaluator<issue_license_evaluator>
{
public:
  typedef issue_license_operation operation_type;

  void_result do_evaluate(const operation_type& op);
  void_result do_apply(const operation_type& op);

private:
  account_id_type issuer_id_;
  const license_type_object* new_license_obj_ = nullptr;
  const account_object* account_obj_ = nullptr;
};

} } // graphene::chain
