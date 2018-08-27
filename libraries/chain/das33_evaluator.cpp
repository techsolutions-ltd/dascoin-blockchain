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
  share_type users_total_pledges_in_round(account_id_type user_id, das33_project_id_type project_id, share_type round, const database& d)
  {
    share_type sum = 0;
    const auto& idx = d.get_index_type<das33_pledge_holder_index>().indices().get<by_user>().equal_range(user_id);
    for( auto it = idx.first; it != idx.second; ++it )
    {
      if (it->project_id == project_id && it->phase_number == round)
        sum += (it->base_expected.amount + it->bonus_expected.amount);
    }
    return sum;
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

  share_type precision_modifier(asset_object a, asset_object b)
  {
    share_type result = 1;
    if (a.precision > b.precision)
    {
      result = std::pow(10, a.precision - b.precision);
    }
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
      FC_ASSERT(op.token != d.get_core_asset().id
             && op.token != d.get_web_asset_id()
             && op.token != d.get_dascoin_asset_id()
             && op.token != d.get_cycle_asset_id(), "Can not create project with system assets");

      // Check that token is not used by another project
      const auto& it = std::find_if(idx.begin(), idx.end(),
            [op](const das33_project_object& m) -> bool { return m.token_id == op.token; });
      FC_ASSERT(it == idx.end(), "Token with id ${1} is already used by another project", ("1", op.token));

      // Check that discounts exist
      FC_ASSERT(op.discounts.size() > 0, "Discounts must be provided. Only assets in discounts can be pledged.");

      // Check that discounts are grater then 0
      for (auto itr = op.discounts.begin(); itr != op.discounts.end(); itr++)
      {
        FC_ASSERT(itr->second > 0, "Discount can not be zero or negative");
      }
      return {};

    } FC_CAPTURE_AND_RETHROW((op))
  }

  object_id_type das33_project_create_evaluator::do_apply( const operation_type& op )
  {
    try {
      auto& d = db();

      const asset_object token = op.token(d);
      const asset max_supply {token.options.max_supply * std::pow(10, token.precision), token.id};
      const asset to_collect {op.goal_amount_eur, d.get_web_asset_id()};
      const price token_price = to_collect / max_supply;

      return d.create<das33_project_object>([&](das33_project_object& dpo){
             dpo.name = op.name;
             dpo.owner = op.owner;
             dpo.token_id = op.token;
             dpo.goal_amount_eur = op.goal_amount_eur;
             dpo.discounts = op.discounts;
             dpo.min_pledge = op.min_pledge;
             dpo.max_pledge = op.max_pledge;
             dpo.token_price = token_price;
             dpo.collected_amount_eur = 0;
             dpo.tokens_sold = 0;
             dpo.status = das33_project_status::inactive;
             dpo.phase_number = 0;
             dpo.phase_limit = max_supply.amount;
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
      if (op.discounts.valid())
      {
        for (auto itr = (*op.discounts).begin(); itr != (*op.discounts).end(); ++itr)
        {
          FC_ASSERT(itr->second > 0, "Bonus can not be zero or negative");
        }
      }

      // Check phase number
      if (op.phase_number.valid())
      {
        FC_ASSERT(*op.phase_number > project_to_update->phase_number, "Phase number can not be decreased");
      }

      // Check phase limit
      if (op.phase_limit.valid())
      {
        share_type new_limit = *op.phase_limit;
        asset_object token = project_to_update->token_id(d);
        // If previous limit is not max supply
        if (project_to_update->phase_limit != token.options.max_supply * pow(10, token.precision) )
        {
          // New limit must be larger
          FC_ASSERT(new_limit > project_to_update->phase_limit, "New phase limit must be more then the previous one");
        }
        FC_ASSERT(new_limit <= token.options.max_supply * pow(10, token.precision), "New limit can not be more then max supply of token");
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
        // token_id: we can't alter
        if (op.goal_amount) dpo.goal_amount_eur = *op.goal_amount;
        if (op.discounts) dpo.discounts = *op.discounts;
        if (op.min_pledge) dpo.min_pledge = *op.min_pledge;
        if (op.max_pledge) dpo.max_pledge = *op.max_pledge;
        if (op.token_price) dpo.token_price = *op.token_price;
        // collected_amount_eur: we can't alter
        // tokens_sold: we can't alter
        if (op.status) dpo.status = static_cast<das33_project_status>(*op.status);
        if (op.phase_number) dpo.phase_number = *op.phase_number;
        if (op.phase_limit) dpo.phase_limit = *op.phase_limit;
        if (op.phase_end) dpo.phase_end = *op.phase_end;
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
    FC_ASSERT( op.pledged.asset_id != project_obj.token_id, "Cannot pledge project tokens" );

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
    FC_ASSERT(project_obj.tokens_sold < token_obj.options.max_supply * pow(10, token_obj.precision), "All tokens for project are sold");
    FC_ASSERT(project_obj.tokens_sold < project_obj.phase_limit, "All tokens in this phase are sold");

    // Calculate expected amount
    share_type precision = precision_modifier(op.pledged.asset_id(d), d.get_web_asset_id()(d));
    total.asset_id = project_obj.token_id;
    price_at_evaluation = get_price_in_web_eur(op.pledged.asset_id, d);
    base = asset{op.pledged.amount * precision, op.pledged.asset_id} * price_at_evaluation * project_obj.token_price;
    base.amount = base.amount / precision;

    // Assure that pledge amount is above minimum
    FC_ASSERT(base.amount >= project_obj.min_pledge, "Can not pledge: must buy at least ${min} tokens", ("min", project_obj.min_pledge));

    // Calculate expected amount with discounts
    auto discount_iterator = project_obj.discounts.find(op.pledged.asset_id);
    FC_ASSERT( discount_iterator != project_obj.discounts.end(), "This asset can not be used in this project phase" );
    discount = discount_iterator->second;
    total.amount = base.amount * BONUS_PRECISION / discount;

    // Assure that pledge amount is above zero
    FC_ASSERT(total.amount > 0,
              "Cannot pledge because expected amount of ${tok} is ${ex}",
              ("tok", token_obj.symbol)
              ("ex", d.to_pretty_string(total))
    );

    bool total_reduced = false;
    // Decrease amount if it passes tokens max supply
    if (project_obj.tokens_sold + total.amount > token_obj.options.max_supply * pow(10, token_obj.precision))
    {
        total.amount = token_obj.options.max_supply * pow(10, token_obj.precision) - project_obj.tokens_sold;
        total_reduced = true;
    }

    // Decrease amount if it passes current phase limit
    if (project_obj.tokens_sold + total.amount > project_obj.phase_limit)
    {
        total.amount = project_obj.phase_limit - project_obj.tokens_sold;
        total_reduced = true;
    }

    // Assure that pledge amount is below maximum for current user
    auto previous_pledges = users_total_pledges_in_round(op.account_id, op.project_id, project_obj.phase_number, d);
    FC_ASSERT( previous_pledges + total.amount <= project_obj.max_pledge,
              "Can not buy more then ${max} tokens per round but amount to receive with bonus is ${this} and you already pledged for ${previous}.",
              ("max", project_obj.max_pledge)
              ("this", total.amount)
              ("previous", previous_pledges));

    if (total_reduced)
    {
      base = {total.amount * discount / BONUS_PRECISION, total.asset_id};
      bonus = total - base;
      precision = precision_modifier(base.asset_id(d), d.get_web_asset_id()(d));
      to_take = asset{base.amount * precision, base.asset_id} * project_obj.token_price * price_at_evaluation;
      to_take.amount = to_take.amount / precision;
    }
    else
    {
      bonus = total - base;
      to_take = op.pledged;
    }

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  object_id_type das33_pledge_asset_evaluator::do_apply(const das33_pledge_asset_operation& op)
  { try {

    auto& d = db();
    const auto& project_obj = op.project_id(d);

    // Adjust the balance and spent amount:
    const auto& balance_obj = d.get_balance_object(op.account_id, op.pledged.asset_id);
    d.modify(balance_obj, [&](account_balance_object& from){
      from.balance -= to_take.amount;
      from.spent += to_take.amount;
    });

    // Update project
    d.modify(project_obj, [&](das33_project_object& p){
        p.tokens_sold += total.amount;
        p.collected_amount_eur += (to_take * price_at_evaluation).amount;
    });

    // Create the holder object and return its ID:
    return d.create<das33_pledge_holder_object>([&](das33_pledge_holder_object& cpho){
      cpho.account_id = op.account_id;
      cpho.pledged = to_take;
      cpho.pledge_remaining = to_take;
      cpho.base_remaining = base;
      cpho.base_expected = base;
      cpho.bonus_remaining = bonus;
      cpho.bonus_expected = bonus;
      cpho.phase_number = project_obj.phase_number;
      cpho.project_id = op.project_id;
      cpho.timestamp = d.head_block_time();
    }).id;

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result das33_distribute_project_pledges_evaluator::do_evaluate(const das33_distribute_project_pledges_operation& op)
  { try {

     auto& d = db();

     auto& pro_index = d.get_index_type<das33_project_index>().indices().get<by_id>();
     auto pro_itr = pro_index.find(op.project);
     account_id_type pro_owner;
     FC_ASSERT(pro_itr != pro_index.end(), "Missing project object with this project_id!");

     _pro_owner = pro_itr->owner;

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result das33_distribute_project_pledges_evaluator::do_apply(const das33_distribute_project_pledges_operation& op)
  { try {

    auto& d = db();

    std::vector<object_id_type> pledges_to_remove;
    const auto& index = d.get_index_type<das33_pledge_holder_index>().indices().get<by_project>().equal_range(op.project);
    auto itr = index.first;

    while(itr != index.second)
    {
       if(op.phase_number.valid() && itr->phase_number != *op.phase_number) {
           ++itr;
           continue;
       }

       const das33_pledge_holder_object& pho = *itr;

       // calc amount of token and asset that will be exchanged
       share_type base = std::round(static_cast<double>(pho.base_expected.amount.value) * op.base_to_pledger.value / BONUS_PRECISION / 100);
                  base = (base < pho.base_remaining.amount) ? base : pho.base_remaining.amount;
       share_type bonus = std::round(static_cast<double>(pho.bonus_expected.amount.value) * op.bonus_to_pledger.value / BONUS_PRECISION / 100);
                  bonus = (bonus < pho.bonus_remaining.amount) ? bonus : pho.bonus_remaining.amount;
       share_type pledge = std::round(static_cast<double>(pho.pledged.amount.value) * op.to_escrow.value / BONUS_PRECISION / 100);
                  pledge = (pledge < pho.pledge_remaining.amount) ? pledge : pho.pledge_remaining.amount;

       // make virtual op for history traking
       das33_pledge_result_operation pledge_result;
          pledge_result.funders_account = pho.account_id;
          pledge_result.account_to_fund = _pro_owner;
          pledge_result.completed = true;
          pledge_result.pledged = pledge;
          pledge_result.received = base + bonus;
          pledge_result.project_id = op.project;
          pledge_result.timestamp = d.head_block_time();
       d.push_applied_operation(pledge_result);

       // give to project owner pledged amount
       auto& balance_obj = d.get_balance_object(_pro_owner, pho.pledged.asset_id);
       d.modify(balance_obj, [&](account_balance_object& balance_obj){
          balance_obj.balance += pledge;
       });

       // issue balance object if it does not exists
       if(!d.check_if_balance_object_exists(pho.account_id,pho.base_expected.asset_id))
       {
          d.create<account_balance_object>([&pho](account_balance_object& abo){
             abo.owner = pho.account_id;
             abo.asset_type = pho.base_expected.asset_id;
             abo.balance = 0;
             abo.reserved = 0;
          });
       }

       // issue token asset
       auto& balance1_obj = d.get_balance_object(pho.account_id, pho.base_expected.asset_id);
       d.issue_asset(balance1_obj, base + bonus, 0);

       // update pledge holder object
       d.modify(pho, [&](das33_pledge_holder_object& p){
          p.pledge_remaining.amount -= pledge;
          p.base_remaining.amount -= base;
          p.bonus_remaining.amount -= bonus;
       });

       // if everything is distributed remove object
       if(pho.pledge_remaining.amount + pho.base_remaining.amount + pho.bonus_remaining.amount <= 0)
       {
          pledges_to_remove.push_back(pho.id);
       }
       itr++;
    }

    auto& index1 = d.get_index_type<das33_pledge_holder_index>().indices().get<by_id>();
    for(object_id_type id : pledges_to_remove)
    {
       auto itr = index1.find(id);
       if(itr != index1.end())
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

        das33_pledge_result_operation pledge_result;
           pledge_result.funders_account = pho.account_id;
           pledge_result.account_to_fund = _pro_owner;
           pledge_result.completed = false;
           pledge_result.pledged = pho.pledged;
           pledge_result.received = pho.pledged;
           pledge_result.project_id = op.project;
           pledge_result.timestamp = d.head_block_time();
        d.push_applied_operation(pledge_result);

        auto& balance_obj = d.get_balance_object(pho.account_id, pho.pledged.asset_id);
        d.modify(balance_obj, [&](account_balance_object& balance_obj){
           balance_obj.balance += pho.pledged.amount;
        });

        d.remove(*itr);
     }

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result das33_distribute_pledge_evaluator::do_evaluate(const das33_distribute_pledge_operation& op)
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

  void_result das33_distribute_pledge_evaluator::do_apply(const das33_distribute_pledge_operation& op)
  { try {

     auto& d = db();
     const das33_pledge_holder_object& pho = *_pledge_holder_ptr;

     // calc amount of token and asset that will be exchanged
     share_type base = std::round(static_cast<double>(pho.base_expected.amount.value) * op.base_to_pledger.value / BONUS_PRECISION / 100);
                base = (base < pho.base_remaining.amount) ? base : pho.base_remaining.amount;
     share_type bonus = std::round(static_cast<double>(pho.bonus_expected.amount.value) * op.bonus_to_pledger.value / BONUS_PRECISION / 100);
                bonus = (bonus < pho.bonus_remaining.amount) ? bonus : pho.bonus_remaining.amount;
     share_type pledge = std::round(static_cast<double>(pho.pledged.amount.value) * op.to_escrow.value / BONUS_PRECISION / 100);
                pledge = (pledge < pho.pledge_remaining.amount) ? pledge : pho.pledge_remaining.amount;

     // make virtual op for history traking
     das33_pledge_result_operation pledge_result;
        pledge_result.funders_account = pho.account_id;
        pledge_result.account_to_fund = _pro_owner;
        pledge_result.completed = true;
        pledge_result.pledged = pledge;
        pledge_result.received = base + bonus;
        pledge_result.project_id = pho.project_id;
        pledge_result.timestamp = d.head_block_time();
     d.push_applied_operation(pledge_result);

     // give to project owner pledged amount
     auto& balance_obj = d.get_balance_object(_pro_owner, pho.pledged.asset_id);
     d.modify(balance_obj, [&](account_balance_object& balance_obj){
        balance_obj.balance += pledge;
     });

     // issue balance object if it does not exists
     if(!d.check_if_balance_object_exists(pho.account_id,pho.base_expected.asset_id))
     {
        d.create<account_balance_object>([&pho](account_balance_object& abo){
           abo.owner = pho.account_id;
           abo.asset_type = pho.base_expected.asset_id;
           abo.balance = 0;
           abo.reserved = 0;
        });
     }

     // issue token asset
     auto& balance1_obj = d.get_balance_object(pho.account_id, pho.base_expected.asset_id);
     d.issue_asset(balance1_obj, base + bonus, 0);

     // update pledge holder object
     d.modify(pho, [&](das33_pledge_holder_object& p){
        p.pledge_remaining.amount -= pledge;
        p.base_remaining.amount -= base;
        p.bonus_remaining.amount -= bonus;
     });

     // if everything is distributed remove object
     if(pho.pledge_remaining.amount + pho.base_remaining.amount + pho.bonus_remaining.amount <= 0)
     {
        d.remove(pho);
     }

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
     const das33_pledge_holder_object& pho = *_pledge_holder_ptr;
     FC_ASSERT(pho.base_expected.amount == pho.base_remaining.amount
           && pho.bonus_expected.amount == pho.bonus_remaining.amount
           && pho.pledged.amount == pho.pledge_remaining.amount,
           "Project already accepted, can't be rejected!");

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result das33_pledge_reject_evaluator::do_apply(const das33_pledge_reject_operation& op)
  { try {

     auto& d = db();

     // make virtual op for history traking
     const das33_pledge_holder_object& pho = *_pledge_holder_ptr;

     // make virtual op for history traking
     das33_pledge_result_operation pledge_result;
        pledge_result.funders_account = pho.account_id;
        pledge_result.account_to_fund = _pro_owner;
        pledge_result.completed = false;
        pledge_result.pledged = pho.pledged;
        pledge_result.received = pho.pledged;
        pledge_result.project_id = pho.project_id;
        pledge_result.timestamp = d.head_block_time();
     d.push_applied_operation(pledge_result);

     // give to project owner pledged amount
     auto& balance_obj = d.get_balance_object(pho.account_id, pho.pledged.asset_id);
     d.modify(balance_obj, [&](account_balance_object& balance_obj){
        balance_obj.balance += pho.pledged.amount;
     });

     d.remove(pho);

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }


} }  // namespace graphene::chain
