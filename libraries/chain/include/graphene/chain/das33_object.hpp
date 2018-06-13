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
  using namespace boost::multi_index;

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
    price cycles_to_token_ratio;
    das33_project_status status;

    das33_project_object() = default;
    explicit das33_project_object(string name, account_id_type owner, asset_id_type token, share_type min_to_collect, price ratio)
             : name(name),
	       owner(owner),
	       token_id(token),
	       min_to_collect(min_to_collect),
	       collected(0),
	       cycles_to_token_ratio(ratio),
	       status(das33_project_status::inactive)
    {}
  };

  class das33_cycles_pledge_holder_object : public abstract_object<das33_cycles_pledge_holder_object>
  {
  public:
    static const uint8_t space_id = implementation_ids;
    static const uint8_t type_id  = impl_das33_cycles_pledge_holder_object_type;

    account_id_type       vault_id;
    license_type_id_type  license_id;
    share_type            cycles_amount;
    share_type            token_amount;
    das33_project_id_type project_id;
    time_point_sec        timestamp;

    extensions_type extensions;

    das33_cycles_pledge_holder_object() = default;

    explicit das33_cycles_pledge_holder_object(account_id_type vault_id,
                                               license_type_id_type license_id,
                                               share_type cycles_amount,
                                               share_type token_amount,
                                               das33_project_id_type project_id,
                                               time_point_sec timestamp)
            : vault_id(vault_id),
              license_id(license_id),
              cycles_amount(cycles_amount),
              token_amount(token_amount),
              project_id(project_id),
              timestamp(timestamp) {}
  };

  ///////////////////////////////
  // MULTI INDEX CONTAINERS:   //
  ///////////////////////////////

  struct by_vault;
  struct by_project;

  using das33_cycles_pledge_holder_multi_index_type = boost::multi_index::multi_index_container<
    das33_cycles_pledge_holder_object,
    indexed_by<
      ordered_unique<
        tag<by_id>,
        member< object, object_id_type, &object::id >
      >,
      ordered_unique<
        tag<by_vault>,
        composite_key<
          das33_cycles_pledge_holder_object,
          member< das33_cycles_pledge_holder_object, account_id_type, &das33_cycles_pledge_holder_object::vault_id >,
          member< das33_cycles_pledge_holder_object, das33_project_id_type, &das33_cycles_pledge_holder_object::project_id >,
          member< object, object_id_type, &object::id >
        >
      >,
      ordered_unique<
        tag<by_project>,
        composite_key<
          das33_cycles_pledge_holder_object,
          member< das33_cycles_pledge_holder_object, das33_project_id_type, &das33_cycles_pledge_holder_object::project_id >,
          member< das33_cycles_pledge_holder_object, account_id_type, &das33_cycles_pledge_holder_object::vault_id >,
          member< object, object_id_type, &object::id >
        >
      >
    >
  >;

  using das33_cycles_pledge_holder_index = generic_index<das33_cycles_pledge_holder_object, das33_cycles_pledge_holder_multi_index_type>;

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

FC_REFLECT_DERIVED( graphene::chain::das33_cycles_pledge_holder_object, (graphene::db::object),
                    (vault_id)
                    (license_id)
                    (cycles_amount)
                    (token_amount)
                    (timestamp)
                    (project_id)
)

FC_REFLECT_DERIVED( graphene::chain::das33_project_object, (graphene::db::object),
                    (name)
                    (owner)
		    (token_id)
		    (min_to_collect)
		    (collected)
		    (cycles_to_token_ratio)
		    (status)
                  )
