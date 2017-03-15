/**
 * DASCOIN!
 */
#pragma once

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
  object_id_type do_apply(const operation_type& op);

private:
  account_id_type _issuer_id;
  const account_object* _account_obj = nullptr;
  const license_information_object* _license_information_obj = nullptr;
  const license_type_object* _new_license_obj = nullptr;
};

} } // graphene::chain
