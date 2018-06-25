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

  ///////////////////////////////
  // OBJECTS:                  //
  ///////////////////////////////

  class das33_project_object : public graphene::db::abstract_object<das33_project_object>
  {
public:
    static const uint8_t space_id = implementation_ids;
    static const uint8_t type_id  = impl_das33_project_object_type;

    string name;
    account_id_type owner;
    asset_id_type token_id;
    optional<share_type> min_to_collect;
    share_type collected;
    vector<price> token_prices;
    das33_project_status status;

    das33_project_object() = default;
    explicit das33_project_object(string name, account_id_type owner, asset_id_type token, share_type min_to_collect, vector<price> ratios)
             : name(name),
               owner(owner),
               token_id(token),
               min_to_collect(min_to_collect),
               collected(0),
               token_prices(ratios),
               status(das33_project_status::inactive) {}
  };

  class das33_pledge_holder_object : public abstract_object<das33_pledge_holder_object>
  {
  public:
    static const uint8_t space_id = implementation_ids;
    static const uint8_t type_id  = impl_das33_pledge_holder_object_type;

    account_id_type                account_id;
    asset                          pledged;
    asset                          expected;
    optional<license_type_id_type> license_id;
    das33_project_id_type          project_id;
    time_point_sec                 timestamp;

    extensions_type extensions;

    das33_pledge_holder_object() = default;

    explicit das33_pledge_holder_object(account_id_type account_id,
                                        asset pledged,
                                        asset expected,
                                        optional<license_type_id_type> license_id,
                                        das33_project_id_type project_id,
                                        time_point_sec timestamp)
            : account_id(account_id),
              pledged(pledged),
              expected(expected),
              license_id(license_id),
              project_id(project_id),
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
                    (pledged)
                    (expected)
                    (license_id)
                    (project_id)
                    (timestamp)
                  )

FC_REFLECT_DERIVED( graphene::chain::das33_project_object, (graphene::db::object),
                    (name)
                    (owner)
                    (token_id)
                    (min_to_collect)
                    (collected)
                    (token_prices)
                    (status)
                  )
