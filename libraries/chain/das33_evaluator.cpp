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

#include <graphene/chain/das33_evaluator.hpp>
#include <graphene/chain/database.hpp>

namespace graphene { namespace chain {

  void_result das33_project_create_evaluator::do_evaluate( const operation_type& op )
  {
    try {
      const auto& d = db();
      const auto& gpo = d.get_global_properties();

      const auto& authority_obj = op.authority(d);
      d.perform_chain_authority_check("root authority", gpo.authorities.root_administrator, authority_obj);

      const auto& idx = d.get_index_type<das33_project_index>().indices().get<by_project_name>();
      FC_ASSERT(idx.find(op.name) == idx.end(), "Das33 project called ${1} already exists.", ("1", op.name));

      return {};
    } FC_CAPTURE_AND_RETHROW((op))
  }

  object_id_type das33_project_create_evaluator::do_apply( const operation_type& op )
  {
    try {
      auto& d = db();

      return d.create<das33_project_object>([&](das33_project_object& dpo){
	     dpo.name = op.name;
	     dpo.owner = op.owner;
	     dpo.token_id = op.token;
	     dpo.min_to_collect = op.min_to_collect;
	     dpo.collected = 0;
	     dpo.cycles_to_token_ratio = op.ratio;
	     dpo.status = das33_project_status::inactive;
	   }).id;
    } FC_CAPTURE_AND_RETHROW((op))
  }

  void_result das33_project_update_evaluator::do_evaluate( const operation_type& op )
  {
    try {
      const auto& d = db();
      const auto& gpo = d.get_global_properties();

      const auto& authority_obj = op.authority(d);
      d.perform_chain_authority_check("root authority", gpo.authorities.root_administrator, authority_obj);

      const auto& idx = d.get_index_type<das33_project_index>().indices().get<by_project_name>();
      auto project_iterator = idx.find(op.name);
      FC_ASSERT(project_iterator != idx.end(), "Das33 project called ${1} does not exist.", ("1", op.name));
      project_to_update = &(*project_iterator);

      return {};
    } FC_CAPTURE_AND_RETHROW((op))
  }

  void_result das33_project_update_evaluator::do_apply( const operation_type& op )
  {
    try {
      auto& d = db();

      d.modify<das33_project_object>(*project_to_update, [&](das33_project_object& dpo){
	if (op.owner) dpo.owner = *op.owner;
	if (op.min_to_collect) dpo.min_to_collect = op.min_to_collect;
	if (op.ratio) dpo.cycles_to_token_ratio = *op.ratio;
	if (op.status) dpo.status = static_cast<das33_project_status>(*op.status);
      });

      return {};
    } FC_CAPTURE_AND_RETHROW((op))
  }

  void_result das33_project_delete_evaluator::do_evaluate( const operation_type& op )
  {
    try {
      const auto& d = db();
      const auto& gpo = d.get_global_properties();

      const auto& authority_obj = op.authority(d);
      d.perform_chain_authority_check("root authority", gpo.authorities.root_administrator, authority_obj);

      const auto& idx = d.get_index_type<das33_project_index>().indices().get<by_id>();
      auto project_iterator = idx.find(op.project_id);
      FC_ASSERT(project_iterator != idx.end(), "Das33 project with id ${1} does not exist.", ("1", op.project_id));
      project_to_delete = &(*project_iterator);

      //TODO: Add assert that project has no pledges

      return {};
    } FC_CAPTURE_AND_RETHROW((op))
  }

  void_result das33_project_delete_evaluator::do_apply( const operation_type& op )
  {
    try {
      auto& d = db();

      d.remove(*project_to_delete);

      return {};
    } FC_CAPTURE_AND_RETHROW((op))
  }

  void_result das33_pledge_asset_evaluator::do_evaluate(const das33_pledge_asset_operation& op)
  { try {

    const auto& d = db();
//    const auto& account_obj = op.account_id(d);
//
//    // IF  op.license_id.valid()
//
//    // Only vault accounts are allowed to submit cycles:
//    FC_ASSERT( account_obj.is_vault(),
//               "Account '${n}' is not a vault account",
//               ("n", account_obj.name)
//             );
//
//    FC_ASSERT( account_obj.license_information.valid(),
//               "Cannot submit cycles, account '${n}' does not have any licenses",
//               ("n", account_obj.name)
//          );
//
//    const auto& license_information_obj = (*account_obj.license_information)(d);
//
//    // Check if this account has a required license:
//    optional<license_information_object::license_history_record> license{license_information_obj.get_license(*op.license_id)};
//    FC_ASSERT( license.valid(),
//               "Account '${n}' does not have a license of type ${l}",
//               ("n", account_obj.name)
//               ("l", op.license_id)
//             );
//
//    // Assure we have enough funds to submit:
//    FC_ASSERT( license->total_cycles() >= op.pledged.amount,
//               "Cannot submit ${am} cycles, account '${n}' license cycle balance is ${b}",
//               ("am", op.pledged.amount)
//               ("n", account_obj.name)
//               ("b", license->amount)
//          );

//TODO
//     Assure that amount of cycles submitted would not exceed token max supply limit.
//    const auto& project_obj = op.project_id(d);
//    FC_ASSERT(d.cycles_to_asset(op.cycles_amount, frequency, project_obj.token_id) + project_obj.collected <= project_obj.token_id.max_supply * token_precision(project_obj.token_id),
//                "Cannot submit ${am} cycles with frequency (${f}), "
//                "because it would exceed token's max supply limit ${max_limit}",
//                ("am", op.amount)
//                ("f", frequency)
//                ("max_limit", project_obj.token_id.max_supply * token_precision(project_obj.token_id))
//              );

    // project exists ?

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  object_id_type das33_pledge_asset_evaluator::do_apply(const das33_pledge_asset_operation& op)
  { try {

    auto& d = db();
    const auto& account_obj = op.account_id(d);
    //const auto& license_information_obj = (*account_obj.license_information)(d);

    // Spend cycles, decrease balance and supply:
//    d.reserve_cycles(op.account_id, op.cycles_amount);
//    d.modify( license_information_obj, [&](license_information_object& lio){
//              lio.subtract_cycles(op.license_id, op.cycles_amount);
//            });

    // Create the holder object and return its ID:
    return d.create<das33_pledge_holder_object>([&](das33_pledge_holder_object& cpho){
        cpho.account_id = op.account_id;
        cpho.pledged = op.pledged;
        cpho.expected = asset{};
        cpho.license_id = op.license_id;
        cpho.project_id = op.project_id;
        cpho.timestamp = d.head_block_time();
    }).id;

  } FC_CAPTURE_AND_RETHROW((op)) }

} }  // namespace graphene::chain
