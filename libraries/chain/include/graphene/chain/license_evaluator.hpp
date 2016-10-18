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

class license_type_create_evaluator : public evaluator<license_type_create_evaluator>
{
public:
  typedef license_type_create_operation operation_type;

  void_result do_evaluate(const license_type_create_operation& op);

  void_result do_apply(const license_type_create_operation& op);
};

class license_type_edit_evaluator : public evaluator<license_type_edit_evaluator>
{
public:
  typedef license_type_edit_operation operation_type;

  void_result do_evaluate(const license_type_edit_operation& op);

  void_result do_apply(const license_type_edit_operation& op);
};

class license_type_delete_evaluator : public evaluator<license_type_delete_evaluator>
{
public:
  typedef license_type_delete_operation operation_type;

  void_result do_evaluate(const license_type_delete_operation& op);

  void_result do_apply(const license_type_delete_operation& op);
};

class license_request_evaluator : public evaluator<license_request_evaluator>
{
public:
  typedef license_request_operation operation_type;

  void_result do_evaluate(const license_request_operation& op);

  void_result do_apply(const license_request_operation& op);

private:
  const account_object* _account_obj = nullptr;
};

class license_approve_evaluator : public evaluator<license_approve_evaluator>
{
public:
  typedef license_approve_operation operation_type;

  void_result do_evaluate(const license_approve_operation& op);

  void_result do_apply(const license_approve_operation& op);

private:
  const account_object* _account_obj = nullptr;
  const license_type_object* _license_obj = nullptr;
  const license_request_object* _request_obj = nullptr;
};

class license_deny_evaluator : public evaluator<license_deny_evaluator>
{
public:
  typedef license_deny_operation operation_type;

  void_result do_evaluate(const license_deny_operation& op);

  void_result do_apply(const license_deny_operation& op);

private:
  const license_request_object* _request_obj = nullptr;
};

} } // graphene::chain
