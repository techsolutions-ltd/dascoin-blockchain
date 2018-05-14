/*
 * MIT License
 *
 * Copyright (c) 2018 Tech Solutions Malta LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <graphene/chain/database.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/exceptions.hpp>


namespace graphene { namespace chain {

namespace detail {
	share_type apply_percentage(share_type val, share_type percent);
}  // namespace graphene::chain::detail

class create_license_type_evaluator : public evaluator<create_license_type_evaluator>
{
public:
  typedef create_license_type_operation operation_type;

  void_result do_evaluate(const create_license_type_operation& op);
  object_id_type do_apply(const create_license_type_operation& op);
};

class edit_license_type_evaluator : public evaluator<edit_license_type_evaluator>
{
public:
  typedef edit_license_type_operation operation_type;

  void_result do_evaluate(const edit_license_type_operation& op);
  void_result do_apply(const edit_license_type_operation& op);

private:
  const license_type_object* _license_object;
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
  license_kind _license_kind;
};

class update_license_evaluator : public evaluator<update_license_evaluator>
{
public:
  typedef update_license_operation operation_type;

  void_result do_evaluate(const operation_type& op);
  void_result do_apply(const operation_type& op);

private:
  const license_information_object* _license_information_obj = nullptr;
  uint32_t _index;
};

} } // graphene::chain
