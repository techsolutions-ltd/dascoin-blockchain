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
#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/generic_index.hpp>
#include <graphene/db/object.hpp>
#include <boost/multi_index/composite_key.hpp>

namespace graphene { namespace chain {

#define BONUS_PRECISION 100

  ///////////////////////////////
  // OBJECTS:                  //
  ///////////////////////////////

  class das33_project_object : public graphene::db::abstract_object<das33_project_object>
  {
  public:
    static const uint8_t space_id = implementation_ids;
    static const uint8_t type_id  = impl_das33_project_object_type;

    string                         name;
    account_id_type                owner;
    asset_id_type                  token_id;
    share_type                     goal_amount_eur;
    map<asset_id_type, share_type> discounts;
    share_type                     min_pledge;
    share_type                     max_pledge;
    price                          token_price;
    share_type                     collected_amount_eur = 0;
    share_type                     tokens_sold = 0;
    das33_project_status           status;
    share_type                     phase_number;
    share_type                     phase_limit;
    time_point_sec                 phase_end;

    das33_project_object() = default;
    explicit das33_project_object(string name, account_id_type owner, asset_id_type token, share_type goal_amount_eur,
                                  map<asset_id_type, share_type>& discounts, share_type min_pledge, share_type max_pledge, price price)
             : name(name),
               owner(owner),
               token_id(token),
               goal_amount_eur(goal_amount_eur),
               discounts(discounts),
               min_pledge(min_pledge),
               max_pledge(max_pledge),
               token_price(price),
               collected_amount_eur(0),
               tokens_sold(0),
               status(das33_project_status::inactive),
               phase_limit(0),
               phase_number (0) {}
  };

  class das33_pledge_holder_object : public abstract_object<das33_pledge_holder_object>
  {
  public:
    static const uint8_t space_id = implementation_ids;
    static const uint8_t type_id  = impl_das33_pledge_holder_object_type;

    account_id_type                account_id;
    das33_project_id_type          project_id;
    asset                          pledged;
    asset                          pledge_remaining;
    asset                          base_expected;
    asset                          base_remaining;
    asset                          bonus_expected;
    asset                          bonus_remaining;
    share_type                     phase_number;
    time_point_sec                 timestamp;

    extensions_type extensions;

    das33_pledge_holder_object() = default;

    explicit das33_pledge_holder_object(account_id_type account_id,
                                        das33_project_id_type project_id,
                                        asset pledged,
                                        asset pledge_remaining,
                                        asset base_expected,
                                        asset base_remaining,
                                        asset bonus_expected,
                                        asset bonus_remaining,
                                        share_type phase_number,
                                        time_point_sec timestamp)
            : account_id(account_id),
              project_id(project_id),
              pledged(pledged),
              pledge_remaining(pledge_remaining),
              base_expected(base_expected),
              base_remaining(base_remaining),
              bonus_expected(bonus_expected),
              bonus_remaining(bonus_remaining),
              phase_number(phase_number),
              timestamp(timestamp) {}
  };

  ///////////////////////////////
  // MULTI INDEX CONTAINERS:   //
  ///////////////////////////////

  using boost::multi_index::multi_index_container;
  using boost::multi_index::indexed_by;
  using boost::multi_index::ordered_unique;
  using boost::multi_index::tag;
  using boost::multi_index::member;
  using boost::multi_index::composite_key;

  struct by_user;
  struct by_project;

  using das33_pledge_holder_multi_index_type = multi_index_container<
    das33_pledge_holder_object,
    indexed_by<
      ordered_unique<
        tag<by_id>,
        member< object, object_id_type, &object::id >
      >,
      ordered_unique<
        tag<by_user>,
        composite_key<
          das33_pledge_holder_object,
          member< das33_pledge_holder_object, account_id_type, &das33_pledge_holder_object::account_id >,
          member< das33_pledge_holder_object, das33_project_id_type, &das33_pledge_holder_object::project_id >,
          member< object, object_id_type, &object::id >
        >
      >,
      ordered_unique<
        tag<by_project>,
        composite_key<
          das33_pledge_holder_object,
          member< das33_pledge_holder_object, das33_project_id_type, &das33_pledge_holder_object::project_id >,
          member< object, object_id_type, &object::id >
        >
      >
    >
  >;

  using das33_pledge_holder_index = generic_index<das33_pledge_holder_object, das33_pledge_holder_multi_index_type>;

  struct by_project_name;
  typedef multi_index_container<
      das33_project_object,
      indexed_by<
        ordered_unique<
          tag<by_id>,
          member<object, object_id_type, &object::id>
        >,
        ordered_unique<
          tag<by_project_name>,
          member<das33_project_object, string, &das33_project_object::name>
        >
     >
  > das33_project_multi_index_type;

  typedef generic_index<das33_project_object, das33_project_multi_index_type> das33_project_index;

} }  // namespace graphene::chain

///////////////////////////////
// REFLECTIONS:              //
///////////////////////////////

FC_REFLECT_DERIVED( graphene::chain::das33_pledge_holder_object, (graphene::db::object),
                    (account_id)
                    (project_id)
                    (pledged)
                    (pledge_remaining)
                    (base_expected)
                    (base_remaining)
                    (bonus_expected)
                    (bonus_remaining)
                    (phase_number)
                    (timestamp)
                  )

FC_REFLECT_DERIVED( graphene::chain::das33_project_object, (graphene::db::object),
                    (name)
                    (owner)
                    (token_id)
                    (goal_amount_eur)
                    (discounts)
                    (min_pledge)
                    (max_pledge)
                    (token_price)
                    (collected_amount_eur)
                    (tokens_sold)
                    (status)
                    (phase_number)
                    (phase_limit)
                    (phase_end)
                  )
