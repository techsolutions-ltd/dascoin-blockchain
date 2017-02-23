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

  void_result do_evaluate(const issue_license_operation& op);
  object_id_type do_apply(const issue_license_operation& op);

private:
  const license_type_object* new_license_obj_ = nullptr;
  const account_object* account_obj_ = nullptr;
};

class deny_license_evaluator : public evaluator<deny_license_evaluator>
{
public:
  typedef deny_license_operation operation_type;

  void_result do_evaluate(const deny_license_operation& op);
  void_result do_apply(const deny_license_operation& op);

private:
  const account_object* account_obj_ = nullptr;
  const license_request_object* request_obj_ = nullptr;
};

} } // graphene::chain
