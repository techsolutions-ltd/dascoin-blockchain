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

#include <graphene/chain/protocol/base.hpp>
#include <iostream>

namespace graphene { namespace chain {

  struct das33_project_create_operation : public base_operation
  {
    struct fee_parameters_type { uint64_t fee = 0; };
    asset fee;

    account_id_type                authority;
    string                         name;
    account_id_type                owner;
    asset_id_type                  token;
    share_type                     goal_amount_eur;
    map<asset_id_type, share_type> discounts;
    share_type                     min_pledge;
    share_type                     max_pledge;
    extensions_type                extensions;

    das33_project_create_operation() = default;

    explicit das33_project_create_operation(const account_id_type& authority,
                                            const string& name,
                                            const account_id_type& owner,
                                            const asset_id_type& token,
                                            share_type goal_amount_eur,
                                            const map<asset_id_type, share_type>& discounts,
                                            share_type min_pledge = 0,
                                            share_type max_pledge = share_type::max())
              : authority(authority)
              , name(name)
              , owner(owner)
              , token(token)
              , goal_amount_eur(goal_amount_eur)
              , discounts(discounts)
              , min_pledge(min_pledge)
              , max_pledge(max_pledge) {}

    account_id_type fee_payer() const { return authority; }
    void validate() const;
  };

  struct das33_project_update_operation : public base_operation
  {
    struct fee_parameters_type { uint64_t fee = 0; };
    asset fee;

    account_id_type                          authority;
    das33_project_id_type                    project_id;
    optional<string>                         name;
    optional<account_id_type>                owner;
    optional<share_type>                     goal_amount;
    optional<price>                          token_price;
    optional<map<asset_id_type, share_type>> discounts;
    optional<share_type>                     min_pledge;
    optional<share_type>                     max_pledge;
    optional<share_type>                     phase_number;
    optional<share_type>                     phase_limit;
    optional<time_point_sec>                 phase_end;
    optional<uint8_t>                        status;
    extensions_type                          extensions;

    das33_project_update_operation() = default;

    account_id_type fee_payer() const { return authority; }
    void validate() const;
  };

  struct das33_project_delete_operation : public base_operation
  {
    struct fee_parameters_type { uint64_t fee = 0; };
    asset fee;

    account_id_type       authority;
    das33_project_id_type project_id;
    extensions_type       extensions;

    das33_project_delete_operation() = default;

    explicit das33_project_delete_operation(const account_id_type& authority, const das33_project_id_type& project_id)
             : authority(authority)
             , project_id(project_id) {}

    account_id_type fee_payer() const { return authority; }
    void validate() const;
  };

  struct das33_pledge_asset_operation : public base_operation
  {
    struct fee_parameters_type { uint64_t fee = 0; };
    asset fee;

    account_id_type                 account_id;
    asset                           pledged;
    optional<license_type_id_type>  license_id;
    das33_project_id_type           project_id;
    extensions_type                 extensions;

    das33_pledge_asset_operation() = default;

    explicit das33_pledge_asset_operation(const account_id_type& account_id,
                                          const asset& pledged,
                                          optional<license_type_id_type> license_id,
                                          const das33_project_id_type& project_id)
            : account_id(account_id)
            , pledged(pledged)
            , license_id(license_id)
            , project_id(project_id) {}

    account_id_type fee_payer() const { return account_id; }
    void validate() const;
  };

  struct das33_distribute_project_pledges_operation : public base_operation
  {
    struct fee_parameters_type { uint64_t fee = 0; };
    asset fee;

    account_id_type        authority;
    das33_project_id_type  project;
    optional<share_type>   phase_number;
    share_type             to_escrow;
    share_type             base_to_pledger;
    share_type             bonus_to_pledger;

    extensions_type        extensions;

    das33_distribute_project_pledges_operation() = default;

    explicit das33_distribute_project_pledges_operation(const account_id_type&       authority,
                                                        const das33_project_id_type&  project,
                                                        share_type                    phase_number,
                                                        share_type                    to_escrow,
                                                        share_type                    base_to_pledger,
                                                        share_type                    bonus_to_pledger)
              : authority(authority)
              , project(project)
              , phase_number(phase_number)
              , to_escrow(to_escrow)
              , base_to_pledger(base_to_pledger)
              , bonus_to_pledger(bonus_to_pledger){}

    account_id_type fee_payer() const { return authority; }
    void validate() const;
  };

  struct das33_project_reject_operation : public base_operation
  {
    struct fee_parameters_type { uint64_t fee = 0; };
    asset fee;

    account_id_type        authority;
    das33_project_id_type  project;
    extensions_type        extensions;

    das33_project_reject_operation() = default;

    explicit das33_project_reject_operation(const account_id_type& authority,
                                           const das33_project_id_type&  project)
              : authority(authority)
              , project(project) {}

    account_id_type fee_payer() const { return authority; }
    void validate() const;
  };

  struct das33_distribute_pledge_operation : public base_operation
  {
    struct fee_parameters_type { uint64_t fee = 0; };
    asset fee;

    account_id_type              authority;
    das33_pledge_holder_id_type  pledge;
    share_type                   to_escrow;
    share_type                   base_to_pledger;
    share_type                   bonus_to_pledger;

    extensions_type              extensions;

    das33_distribute_pledge_operation() = default;

    explicit das33_distribute_pledge_operation(const account_id_type& authority,
                                           const das33_pledge_holder_id_type&  pledge,
                                           const share_type to_escrow,
                                           const share_type base_to_pledger,
                                           const share_type bonus_to_pledger)

              : authority(authority)
              , pledge(pledge)
              , to_escrow(to_escrow)
              , base_to_pledger(base_to_pledger)
              , bonus_to_pledger(bonus_to_pledger){}

    account_id_type fee_payer() const { return authority; }
    void validate() const;
  };

  struct das33_pledge_reject_operation : public base_operation
  {
    struct fee_parameters_type { uint64_t fee = 0; };
    asset fee;

    account_id_type              authority;
    das33_pledge_holder_id_type  pledge;
    extensions_type              extensions;

    das33_pledge_reject_operation() = default;

    explicit das33_pledge_reject_operation(const account_id_type& authority,
                                           const das33_pledge_holder_id_type&  pledge)
              : authority(authority)
              , pledge(pledge) {}

    account_id_type fee_payer() const { return authority; }
    void validate() const;
  };

  struct das33_pledge_result_operation : public base_operation
  {
    struct fee_parameters_type { uint64_t fee = 0; };
    asset fee;

    account_id_type                funders_account;
    account_id_type                account_to_fund;
    bool                           completed;
    asset                          pledged;
    asset                          received;
    das33_project_id_type          project_id;
    time_point_sec                 timestamp;
    extensions_type                extensions;

    das33_pledge_result_operation() = default;

    explicit das33_pledge_result_operation(const account_id_type& funders_account,
                                           const account_id_type& account_to_fund,
                                           const bool completed,
                                           const asset& pledged,
                                           const asset& received,
                                           const das33_project_id_type& project_id,
                                           const time_point_sec& timestamp)
              : funders_account(funders_account)
              , account_to_fund(account_to_fund)
              , completed(completed)
              , pledged(pledged)
              , received(received)
              , project_id(project_id)
              , timestamp(timestamp) {}

    account_id_type fee_payer() const { return funders_account; }
    void validate() const;
  };

} }  // namespace graphene::chain

////////////////////////////////
// REFLECTIONS:               //
////////////////////////////////

FC_REFLECT( graphene::chain::das33_project_create_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::das33_project_create_operation,
            (fee)
            (authority)
            (name)
            (owner)
            (token)
            (discounts)
            (goal_amount_eur)
            (min_pledge)
            (max_pledge)
            (extensions)
          )

FC_REFLECT( graphene::chain::das33_project_update_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::das33_project_update_operation,
            (fee)
            (authority)
            (project_id)
            (name)
            (owner)
            (goal_amount)
            (token_price)
            (discounts)
            (min_pledge)
            (max_pledge)
            (phase_number)
            (phase_limit)
            (phase_end)
            (status)
            (extensions)
          )

FC_REFLECT( graphene::chain::das33_project_delete_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::das33_project_delete_operation,
            (fee)
            (authority)
            (project_id)
            (extensions)
          )

FC_REFLECT( graphene::chain::das33_pledge_asset_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::das33_pledge_asset_operation,
            (fee)
            (account_id)
            (pledged)
            (license_id)
            (project_id)
            (extensions)
          )

FC_REFLECT( graphene::chain::das33_distribute_project_pledges_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::das33_distribute_project_pledges_operation,
            (fee)
            (authority)
            (project)
            (phase_number)
            (to_escrow)
            (base_to_pledger)
            (bonus_to_pledger)
            (extensions)
          )

FC_REFLECT( graphene::chain::das33_project_reject_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::das33_project_reject_operation,
            (fee)
            (authority)
            (project)
            (extensions)
          )

FC_REFLECT( graphene::chain::das33_distribute_pledge_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::das33_distribute_pledge_operation,
            (fee)
            (authority)
            (pledge)
            (to_escrow)
            (base_to_pledger)
            (bonus_to_pledger)
            (extensions)
          )

FC_REFLECT( graphene::chain::das33_pledge_reject_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::das33_pledge_reject_operation,
            (fee)
            (authority)
            (pledge)
            (extensions)
          )

FC_REFLECT( graphene::chain::das33_pledge_result_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::das33_pledge_result_operation,
            (fee)
            (funders_account)
            (account_to_fund)
            (completed)
            (pledged)
            (received)
            (project_id)
            (timestamp)
            (extensions)
          )
