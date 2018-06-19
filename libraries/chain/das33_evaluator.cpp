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
#include <graphene/chain/balance_checker.hpp>

namespace graphene { namespace chain {

  // Helper methods:
  void prices_check(const vector<price>& prices, asset_id_type token)
  {
    // check that all prices are for project token and that no price for asset is used multiple times:
    std::map<asset_id_type, int> asset_count;
    for(auto item : prices)
    {
      FC_ASSERT(item.base.asset_id == token || item.quote.asset_id == token, "All prices must be for project token");
      if (asset_count.find(item.base.asset_id) != asset_count.end())
        asset_count[item.base.asset_id] += 1;
      else
    	asset_count[item.base.asset_id] = 1;
      if (asset_count.find(item.quote.asset_id) != asset_count.end())
        asset_count[item.quote.asset_id] += 1;
      else
        asset_count[item.quote.asset_id] = 1;
    }
    std::map<asset_id_type, int>::iterator itr = asset_count.begin();
    while(itr != asset_count.end())
    {
      if (itr->first != token)
        FC_ASSERT(itr->second == 1, "Each asset can appear only once in ratios");
      itr++;
    }
  }

  // method implementations:

  void_result das33_project_create_evaluator::do_evaluate( const operation_type& op )
  {
    try {
      const auto& d = db();
      const auto& gpo = d.get_global_properties();

      const auto& authority_obj = op.authority(d);
      d.perform_chain_authority_check("root authority", gpo.authorities.root_administrator, authority_obj);

      const auto& idx = d.get_index_type<das33_project_index>().indices().get<by_project_name>();
      FC_ASSERT(idx.find(op.name) == idx.end(), "Das33 project called ${1} already exists.", ("1", op.name));

      // Chcek that token isn't one of the system assets
      FC_ASSERT(op.token != d.get_core_asset().id && op.token != d.get_web_asset_id()
		&& op.token != d.get_dascoin_asset_id() && op.token != d.get_cycle_asset_id(), "Can not create project with system assets");

      // Check that token is not used by another project
      auto itr = idx.begin();
      while (itr != idx.end())
      {
	FC_ASSERT(itr->token_id != op.token, "Token with id ${1} is already used by anouther project", ("1", op.token));
	itr++;
      }

      prices_check(op.ratios, op.token);

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
	     dpo.token_prices = op.ratios;
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

      const auto& idx = d.get_index_type<das33_project_index>().indices().get<by_id>();
      auto project_iterator = idx.find(op.project_id);
      FC_ASSERT(project_iterator != idx.end(), "Das33 project with id ${1} does not exist.", ("1", op.project_id));
      project_to_update = &(*project_iterator);

      if (op.ratios.size() > 0)
        prices_check(op.ratios, project_to_update->token_id);

      return {};
    } FC_CAPTURE_AND_RETHROW((op))
  }

  void_result das33_project_update_evaluator::do_apply( const operation_type& op )
  {
    try {
      auto& d = db();

      d.modify<das33_project_object>(*project_to_update, [&](das33_project_object& dpo){
	if (op.name) dpo.name = *op.name;
	if (op.owner) dpo.owner = *op.owner;
	if (op.min_to_collect) dpo.min_to_collect = op.min_to_collect;
	if (op.ratios.size() > 0) dpo.token_prices = op.ratios;
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

      const auto& pledges_idx = d.get_index_type<das33_pledge_holder_index>().indices().get<by_project>().equal_range(op.project_id);
      //auto pledges_iterator = pledges_idx.begin();
      FC_ASSERT(pledges_idx.first == pledges_idx.second, "Project can not be deleted as it has no pledges");

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
    const auto& account_obj = op.account_id(d);
    const auto& project_obj = op.project_id(d);
    const auto& token_obj = project_obj.token_id(d);

    // Check if pledged asset and project token are different assets
    FC_ASSERT( op.pledged.asset_id != project_obj.token_id,
               "Cannot pledge project tokens"
    );

    // Check if project is active
    FC_ASSERT(project_obj.status == das33_project_status::active, "Pladge can only be made to active project");

    // Evaluate different things for cycles and other assets
    if( op.pledged.asset_id == d.get_cycle_asset_id() )
    {
      do_evaluate_cycles(d, op, account_obj);
    }
    else
    {
      do_evaluate_asset(d, op, account_obj);
    }

    // Assure target project exists:
    const auto& idx = d.get_index_type<das33_project_index>().indices().get<by_id>();
    FC_ASSERT( idx.find(project_obj.id) != idx.end(), "Bad project id" );

    // Calculate expected amount
    expected.asset_id = project_obj.token_id;
    for( const auto& current : project_obj.token_prices )
    {
      if( current.base.asset_id == op.pledged.asset_id )
        expected.amount = op.pledged.amount.value * current.quote.amount.value / current.base.amount.value;
      else if( current.quote.asset_id == op.pledged.asset_id)
        expected.amount = op.pledged.amount.value * current.base.amount.value / current.quote.amount.value;
    }
    FC_ASSERT(expected.amount > 0,
              "Cannot pledge because expected amount of ${tok} is ${ex}",
              ("tok", token_obj.symbol)
              ("ex", d.to_pretty_string(expected))
    );

    // Assure that amount pledged would not exceed token max supply limit.
    FC_ASSERT(project_obj.collected + expected.amount <= token_obj.options.max_supply,
              "Cannot pledge ${am} because it would exceed token's max supply limit ${max_limit}",
              ("am", d.to_pretty_string(op.pledged))
              ("max_limit", d.to_pretty_string(expected))
    );

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  object_id_type das33_pledge_asset_evaluator::do_apply(const das33_pledge_asset_operation& op)
  { try {

    auto& d = db();
    const auto& account_obj = op.account_id(d);

    // Apply different things for cycles and other assets
    if( op.pledged.asset_id == d.get_cycle_asset_id() )
    {
      const auto& license_information_obj = (*account_obj.license_information)(d);
      do_apply_cycles(d, op, license_information_obj);
    }
    else
    {
      const auto& balance_obj = d.get_balance_object(op.account_id, op.pledged.asset_id);
      do_apply_asset(d, op, balance_obj);
    }

    // Update project
    const auto& project_obj = op.project_id(d);
    d.modify(project_obj, [&](das33_project_object& p){
        p.collected -= expected.amount;
    });

    // Create the holder object and return its ID:
    return d.create<das33_pledge_holder_object>([&](das33_pledge_holder_object& cpho){
      cpho.account_id = op.account_id;
      cpho.pledged = op.pledged;
      cpho.expected = expected;
      cpho.license_id = op.license_id;
      cpho.project_id = op.project_id;
      cpho.timestamp = d.head_block_time();
    }).id;

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result das33_pledge_asset_evaluator::do_evaluate_cycles(const database &d, const das33_pledge_asset_operation &op, const account_object &account_obj) const {

    // Only vault accounts are allowed to submit cycles:
    FC_ASSERT( account_obj.is_vault(),
               "Account '${n}' is not a vault account",
               ("n", account_obj.name)
    );

    // Check if this account has a license:
    FC_ASSERT( account_obj.license_information.valid(),
               "Cannot pledge cycles, account '${n}' does not have any licenses",
               ("n", account_obj.name)
    );

    const auto& license_information_obj = (*account_obj.license_information)(d);

    // Check if this account has a required license:
    const auto& license_iterator = std::find_if(license_information_obj.history.begin(), license_information_obj.history.end(),
                                                [&op](const license_information_object::license_history_record& history_record) {
                                                    return history_record.license == op.license_id;
                                                });
    FC_ASSERT ( license_iterator != license_information_obj.history.end(),
                "License ${l} is not issued to account ${a}",
                ("l", op.license_id)
                ("a", op.account_id)
    );

    // Assure we have enough cycles to submit:
    FC_ASSERT ( (*license_iterator).amount >= op.pledged.amount,
                "Trying to pledge ${t} cycles from license ${l} of vault ${v}, while ${r} remaining",
                ("t", op.pledged.amount)
                ("l", op.license_id)
                ("v", op.account_id)
                ("r", (*license_iterator).amount)
    );

    return {};
  }

  void_result das33_pledge_asset_evaluator::do_evaluate_asset(const database &d, const das33_pledge_asset_operation &op, const account_object &account_obj) const {

    // Assure we have enough balance to pledge:
    const auto& balance_obj = d.get_balance_object(op.account_id, op.pledged.asset_id);
    FC_ASSERT( balance_obj.get_balance() >= op.pledged,
               "Not enough balance on user account ${a}, left ${l}, needed ${n}",
               ("a", op.account_id)
               ("l", d.to_pretty_string(balance_obj.get_balance()))
               ("n", d.to_pretty_string(op.pledged))
    );

    // Assure we dont spend last DASC:
    const auto& asset_obj = op.pledged.asset_id(d);
    balance_checker::check_remaining_balance(d, account_obj, asset_obj, op.pledged.amount);

    return {};
  }

  void_result das33_pledge_asset_evaluator::do_apply_cycles(database &d, const das33_pledge_asset_operation &op, const license_information_object &license_obj) const {

    // Spend cycles:
    d.modify(license_obj, [&](license_information_object& lio) {
      lio.subtract_cycles(*op.license_id, op.pledged.amount);
    });

    return {};
  }


  void_result das33_pledge_asset_evaluator::do_apply_asset(database &d, const das33_pledge_asset_operation &op, const account_balance_object &balance_obj) const {

    // Adjust the balance and spent amount:
    d.modify(balance_obj, [&](account_balance_object& from){
      from.balance -= op.pledged.amount;
      from.spent += op.pledged.amount;
    });

    // Decrease asset supply:
    const auto& asset_obj = op.pledged.asset_id(d);
    d.modify(asset_obj.dynamic_asset_data_id(d), [&](asset_dynamic_data_object& data){
      data.current_supply -= op.pledged.amount;
    });

    return {};
  }


} }  // namespace graphene::chain
