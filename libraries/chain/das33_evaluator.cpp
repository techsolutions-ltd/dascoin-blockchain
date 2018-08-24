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
    for(const auto& item : prices)
    {
      FC_ASSERT(item.base.asset_id == token || item.quote.asset_id == token, "All prices must be for project token");
      asset_count[item.base.asset_id]++;
      asset_count[item.quote.asset_id]++;
    }

    const auto& it = std::find_if(asset_count.begin(), asset_count.end(),
      [token](const std::pair<asset_id_type, int>& m) -> bool { return m.first != token && m.second != 1; });
    FC_ASSERT(it == asset_count.end(), "Each asset can appear only once in ratios");
  }

  void price_check(const price& price_to_check, asset_id_type first_asset, asset_id_type second_asset)
  {
    FC_ASSERT(price_to_check.base.asset_id == first_asset || price_to_check.quote.asset_id == first_asset,
              "Price must be for ${1}", ("1", first_asset));
    FC_ASSERT(price_to_check.base.asset_id == second_asset || price_to_check.quote.asset_id == second_asset,
                  "Price must be for ${1}", ("1", second_asset));
  }

  price get_price_in_web_eur(asset_id_type original_asset_id, const database& d)
  {
    price result;
    if (original_asset_id == d.get_dascoin_asset_id())
      result = d.get_dynamic_global_properties().last_dascoin_price;

    return result;
  }

  // method implementations:

  void_result das33_project_create_evaluator::do_evaluate( const operation_type& op )
  {
    try {
      const auto& d = db();
      const auto& gpo = d.get_global_properties();

      // Check authority
      const auto& authority_obj = op.authority(d);
      d.perform_chain_authority_check("das33 authority", gpo.authorities.das33_administrator, authority_obj);

      // Check that name is unique
      const auto& idx = d.get_index_type<das33_project_index>().indices().get<by_project_name>();
      FC_ASSERT(idx.find(op.name) == idx.end(), "Das33 project called ${1} already exists.", ("1", op.name));

      // Check that owner is a wallet

      const auto& owner_obj = op.owner(d);
      FC_ASSERT( owner_obj.is_wallet(), "Owner account '${name}' is not a wallet account", ("name", owner_obj.name));

      // Check that token exists
      const auto& token_index = d.get_index_type<asset_index>().indices().get<by_id>();
      FC_ASSERT(token_index.find(op.token) != token_index.end(), "Token with id ${1} does not exist", ("1", op.token));

      // Check that token has max_supply
      FC_ASSERT(op.token(d).options.max_supply > 0, "Token must have max_supply > 0");

      // Check that token isn't one of the system assets
      FC_ASSERT(op.token != d.get_core_asset().id && op.token != d.get_web_asset_id()
                && op.token != d.get_dascoin_asset_id() && op.token != d.get_cycle_asset_id(), "Can not create project with system assets");

      // Check that token is not used by another project
      const auto& it = std::find_if(idx.begin(), idx.end(),
            [op](const das33_project_object& m) -> bool { return m.token_id == op.token; });
      FC_ASSERT(it == idx.end(), "Token with id ${1} is already used by another project", ("1", op.token));

      // Check that bonuses, if present, are grater then 0
      if (op.bonuses.size() > 0)
      {
        for (auto itr = op.bonuses.begin(); itr != op.bonuses.end(); itr++)
        {
          FC_ASSERT(itr->second > 0, "Bonus can not be zero or negative");
        }
      }
      return {};
    } FC_CAPTURE_AND_RETHROW((op))
  }

  object_id_type das33_project_create_evaluator::do_apply( const operation_type& op )
  {
    try {
      auto& d = db();

      price token_price;
      asset_object token = op.token(d);
      asset max_supply {token.options.max_supply, token.id};
      asset to_collect {op.goal_amount_eur, d.get_web_asset_id()};
      token_price = to_collect / max_supply;

      return d.create<das33_project_object>([&](das33_project_object& dpo){
             dpo.name = op.name;
             dpo.owner = op.owner;
             dpo.token_id = op.token;
             dpo.goal_amount_eur = op.goal_amount_eur;
             dpo.bonuses = op.bonuses;
             dpo.token_price = token_price;
             dpo.collected_amount_eur = 0;
             dpo.tokens_sold = 0;
             dpo.status = das33_project_status::inactive;
             dpo.phase_limit = token.options.max_supply;
             dpo.phase_end = time_point_sec::min();
           }).id;
    } FC_CAPTURE_AND_RETHROW((op))
  }

  void_result das33_project_update_evaluator::do_evaluate( const operation_type& op )
  {
    try {
      const auto& d = db();
      const auto& gpo = d.get_global_properties();

      // Check authority
      const auto& authority_obj = op.authority(d);
      d.perform_chain_authority_check("das33 authority", gpo.authorities.das33_administrator, authority_obj);

      // Get project
      const auto& idx = d.get_index_type<das33_project_index>().indices().get<by_id>();
      auto project_iterator = idx.find(op.project_id);
      FC_ASSERT(project_iterator != idx.end(), "Das33 project with id ${1} does not exist.", ("1", op.project_id));
      project_to_update = &(*project_iterator);

      // Check name
      if (op.name.valid())
      {
        const auto& name_index = d.get_index_type<das33_project_index>().indices().get<by_project_name>();
        FC_ASSERT(name_index.find(*op.name) == name_index.end(), "Das33 project called ${1} already exists.", ("1", *op.name));
      }

      // Check owner
      if (op.owner.valid())
      {
        const auto& owner_obj = (*op.owner)(d);
        FC_ASSERT( owner_obj.is_wallet(), "Owner account '${name}' is not a wallet account", ("name", owner_obj.name));
      }

      // Check price
      if (op.token_price.valid())
      {
        price_check(*op.token_price, d.get_web_asset_id(), project_to_update->token_id);
      }

      // Check bonuses
      if (op.bonuses.valid())
      {
        if ((*op.bonuses).size() > 0)
        {
          for (auto itr = (*op.bonuses).begin(); itr != (*op.bonuses).end(); ++itr)
          {
            FC_ASSERT(itr->second > 0, "Bonus can not be zero or negative");
          }
        }
      }

      // Check phase limit
      if (op.phase_limit.valid())
      {
        share_type new_limit = *op.phase_limit;
        FC_ASSERT(new_limit > project_to_update->phase_limit, "New phase limit must be more then the previous one");
        asset_object token = project_to_update->token_id(d);
        FC_ASSERT(new_limit <= token.options.max_supply, "New limit can not be more then max supply of token");
      }

      // Check status
      if (op.status.valid())
      {
        FC_ASSERT(*op.status < das33_project_status::DAS33_PROJECT_STATUS_COUNT, "Unknown status value");
      }

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
        if (op.goal_amount) dpo.goal_amount_eur = *op.goal_amount;
        if (op.token_price) dpo.token_price = *op.token_price;
        if (op.bonuses) dpo.bonuses = *op.bonuses;
        if (op.phase_limit) dpo.phase_limit = *op.phase_limit;
        if (op.phase_end) dpo.phase_end = *op.phase_end;
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
      d.perform_chain_authority_check("das33 authority", gpo.authorities.das33_administrator, authority_obj);

      const auto& idx = d.get_index_type<das33_project_index>().indices().get<by_id>();
      auto project_iterator = idx.find(op.project_id);
      FC_ASSERT(project_iterator != idx.end(), "Das33 project with id ${1} does not exist.", ("1", op.project_id));
      project_to_delete = &(*project_iterator);

      const auto& pledges_idx = d.get_index_type<das33_pledge_holder_index>().indices().get<by_project>().equal_range(op.project_id);
      //auto pledges_iterator = pledges_idx.begin();
      FC_ASSERT(pledges_idx.first == pledges_idx.second, "Project can not be deleted as it has pledges");

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

    // Assure target project exists:
    const auto& idx = d.get_index_type<das33_project_index>().indices().get<by_id>();
    FC_ASSERT( idx.find(project_obj.id) != idx.end(), "Bad project id" );

    // Check if project is active
    FC_ASSERT(project_obj.status == das33_project_status::active, "Pladge can only be made to active project");

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

    // Assure current phase hasn't ended
    if (project_obj.phase_end != time_point_sec::min())
    {
      FC_ASSERT(d.head_block_time() < project_obj.phase_end, "Can not pledge: new ICO phase hasn;t started yet");
    }

    // Assure that all tokens aren't sold
    FC_ASSERT(project_obj.tokens_sold < token_obj.options.max_supply, "All tokens for project are sold");
    FC_ASSERT(project_obj.tokens_sold < project_obj.phase_limit, "All tokens in this phase are sold");

    // Calculate expected amount
    expected.asset_id = project_obj.token_id;
    price_at_evaluation = get_price_in_web_eur(op.pledged.asset_id, d);
    expected = op.pledged * price_at_evaluation * project_obj.token_price;
    if (project_obj.bonuses.find(op.pledged.asset_id) != project_obj.bonuses.end())
    {
      share_type bonus = project_obj.bonuses.find(op.pledged.asset_id)->second;
      expected.amount = expected.amount * bonus / BONUS_PRECISION;
    }
    FC_ASSERT(expected.amount > 0,
              "Cannot pledge because expected amount of ${tok} is ${ex}",
              ("tok", token_obj.symbol)
              ("ex", d.to_pretty_string(expected))
    );

    if (project_obj.tokens_sold + expected.amount > token_obj.options.max_supply)
      expected.amount = token_obj.options.max_supply - project_obj.tokens_sold;

    if (project_obj.tokens_sold + expected.amount > project_obj.phase_limit)
      expected.amount = token_obj.options.max_supply - project_obj.phase_limit;

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  object_id_type das33_pledge_asset_evaluator::do_apply(const das33_pledge_asset_operation& op)
  { try {

    auto& d = db();
    //const auto& account_obj = op.account_id(d);
    const auto& project_obj = op.project_id(d);

    // Calculate exact amount that should be taken:
    asset amount;
    amount.asset_id = op.pledged.asset_id;

    share_type bonus = 100;
    if (project_obj.bonuses.find(op.pledged.asset_id) != project_obj.bonuses.end())
    {
      bonus = project_obj.bonuses.find(op.pledged.asset_id)->second;
    }

    asset_object pledged_asset = op.pledged.asset_id(d);

    asset single_asset = asset(std::pow(10, pledged_asset.precision), op.pledged.asset_id);
    asset web_euros = single_asset * price_at_evaluation;
    asset token_in_asset = web_euros * project_obj.token_price;
    token_in_asset.amount = token_in_asset.amount * bonus / BONUS_PRECISION;
    amount.amount = expected.amount / token_in_asset.amount;

    const auto& balance_obj = d.get_balance_object(op.account_id, op.pledged.asset_id);

    // Adjust the balance and spent amount:
    d.modify(balance_obj, [&](account_balance_object& from){
      from.balance -= amount.amount;
      from.spent += amount.amount;
    });

    // Update project
    d.modify(project_obj, [&](das33_project_object& p){
        p.tokens_sold += expected.amount;
        p.collected_amount_eur += (amount * price_at_evaluation).amount;
    });

    // Create the holder object and return its ID:
    return d.create<das33_pledge_holder_object>([&](das33_pledge_holder_object& cpho){
      cpho.account_id = op.account_id;
      cpho.pledged = amount;
      cpho.pledge_owed = amount;
      cpho.expected = expected;
      cpho.expect_owed = {expected.amount / cpho.discount, expected.asset_id};
      cpho.project_id = op.project_id;
      cpho.timestamp = d.head_block_time();
    }).id;

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result das33_project_complete_evaluator::do_evaluate(const das33_project_complete_operation& op)
  { try {

     auto& d = db();

     auto& pro_index = d.get_index_type<das33_project_index>().indices().get<by_id>();
     auto pro_itr = pro_index.find(op.project);
     account_id_type pro_owner;
     FC_ASSERT(pro_itr != pro_index.end(), "Missing project object with this project_id!");

     _pro_owner = pro_itr->owner;

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result das33_project_complete_evaluator::do_apply(const das33_project_complete_operation& op)
  { try {

    auto& d = db();

    while(true)
    {
       auto& index = d.get_index_type<das33_pledge_holder_index>().indices().get<by_project>();
       auto itr = index.lower_bound(op.project);
       if(itr == index.end())
          break;

       // make virtual op for history traking
       const das33_pledge_holder_object& pho = *itr;
       das33_pledge_result_operation new_op = das33_pledge_result_operation(
          pho.account_id, _pro_owner, true, pho.pledged, pho.expected, op.project, fc::time_point::now()
       );
       d.push_applied_operation(new_op);

       // give to project owner pledged amount
       auto& balance_obj = d.get_balance_object(_pro_owner, pho.pledged.asset_id);
       d.modify(balance_obj, [&](account_balance_object& balance_obj){
          balance_obj.balance += pho.pledged.amount;
       });

       // issue balance object if it does not exists
       if(!d.check_if_balance_object_exists(pho.account_id,pho.expected.asset_id))
       {
          d.create<account_balance_object>([&pho](account_balance_object& abo){
             abo.owner = pho.account_id;
             abo.asset_type = pho.expected.asset_id;
             abo.balance = 0;
             abo.reserved = 0;
          });
       }

       // issue token asset
       auto& balance1_obj = d.get_balance_object(pho.account_id, pho.expected.asset_id);
       d.issue_asset(balance1_obj, pho.expected.amount, 0);

       d.remove(*itr);
    }

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result das33_project_reject_evaluator::do_evaluate(const das33_project_reject_operation& op)
  { try {

     auto& d = db();

     auto& pro_index = d.get_index_type<das33_project_index>().indices().get<by_id>();
     auto pro_itr = pro_index.find(op.project);
     account_id_type pro_owner;
     FC_ASSERT(pro_itr != pro_index.end(), "Missing project object with this project_id!");

     _pro_owner = pro_itr->owner;

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result das33_project_reject_evaluator::do_apply(const das33_project_reject_operation& op)
  { try {

     auto& d = db();

     while(true)
     {
        auto& index = d.get_index_type<das33_pledge_holder_index>().indices().get<by_project>();
        auto itr = index.lower_bound(op.project);
        if(itr == index.end())
           break;

        const das33_pledge_holder_object& pho = *itr;
        das33_pledge_result_operation new_op = das33_pledge_result_operation(
              pho.account_id, _pro_owner, false, pho.pledged, pho.expected, op.project, fc::time_point::now()
        );
        d.push_applied_operation(new_op);

        auto& balance_obj = d.get_balance_object(pho.account_id, pho.pledged.asset_id);
        d.modify(balance_obj, [&](account_balance_object& balance_obj){
           balance_obj.balance += pho.pledged.amount;
        });

        d.remove(*itr);
     }

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result das33_pledge_complete_evaluator::do_evaluate(const das33_pledge_complete_operation& op)
  { try {
     auto& d = db();

     // find pledge object
     auto& index = d.get_index_type<das33_pledge_holder_index>().indices().get<by_id>();
     auto itr = index.find(op.pledge);
     FC_ASSERT(itr != index.end(),"Missing pledge object with this pledge_id.");

     auto& pro_index = d.get_index_type<das33_project_index>().indices().get<by_id>();
     auto pro_itr = pro_index.find(itr->project_id);
     account_id_type pro_owner;
     FC_ASSERT(pro_itr != pro_index.end(), "Missing project object with this project_id!");

     _pro_owner = pro_itr->owner;
     _pledge_holder_ptr = &(*itr);

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result das33_pledge_complete_evaluator::do_apply(const das33_pledge_complete_operation& op)
  { try {

     auto& d = db();

     // make virtual op for history traking
     const das33_pledge_holder_object& pho = *_pledge_holder_ptr;
     das33_pledge_result_operation new_op = das33_pledge_result_operation(
        pho.account_id, _pro_owner, true, pho.pledged, pho.expected, pho.project_id, fc::time_point::now()
     );
     d.push_applied_operation(new_op);

     // give to project owner pledged amount
     auto& balance_obj = d.get_balance_object(_pro_owner, pho.pledged.asset_id);
     d.modify(balance_obj, [&](account_balance_object& balance_obj){
        balance_obj.balance += pho.pledged.amount;
     });

     // issue balance object if it does not exists
     if(!d.check_if_balance_object_exists(pho.account_id,pho.expected.asset_id))
     {
        d.create<account_balance_object>([&pho](account_balance_object& abo){
           abo.owner = pho.account_id;
           abo.asset_type = pho.expected.asset_id;
           abo.balance = 0;
           abo.reserved = 0;
        });
     }

     // issue token asset
     auto& balance1_obj = d.get_balance_object(pho.account_id, pho.expected.asset_id);
     d.issue_asset(balance1_obj, pho.expected.amount, 0);

     d.remove(pho);

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result das33_pledge_reject_evaluator::do_evaluate(const das33_pledge_reject_operation& op)
  { try {

     auto& d = db();

     // find pledge object
     auto& index = d.get_index_type<das33_pledge_holder_index>().indices().get<by_id>();
     auto itr = index.find(op.pledge);
     FC_ASSERT(itr != index.end(),"Missing pledge object with this pledge_id.");

     auto& pro_index = d.get_index_type<das33_project_index>().indices().get<by_id>();
     auto pro_itr = pro_index.find(itr->project_id);
     account_id_type pro_owner;
     FC_ASSERT(pro_itr != pro_index.end(), "Missing project object with this project_id!");

     _pro_owner = pro_itr->owner;
     _pledge_holder_ptr = &(*itr);

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result das33_pledge_reject_evaluator::do_apply(const das33_pledge_reject_operation& op)
  { try {

     auto& d = db();

     // make virtual op for history traking
     const das33_pledge_holder_object& pho = *_pledge_holder_ptr;
     das33_pledge_result_operation new_op = das33_pledge_result_operation(
        pho.account_id, _pro_owner, false, pho.pledged, pho.expected, pho.project_id, fc::time_point::now()
     );
     d.push_applied_operation(new_op);

     // give to project owner pledged amount
     auto& balance_obj = d.get_balance_object(pho.account_id, pho.pledged.asset_id);
     d.modify(balance_obj, [&](account_balance_object& balance_obj){
        balance_obj.balance += pho.pledged.amount;
     });

     d.remove(pho);

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }


} }  // namespace graphene::chain
