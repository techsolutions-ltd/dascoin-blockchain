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
    struct fee_parameters_type {};
    asset fee;

    account_id_type              authority;
    string                       name;
    account_id_type              owner;
    asset_id_type                token;
    vector<price>                ratios;
    optional<share_type>         min_to_collect;
    extensions_type              extensions;

    das33_project_create_operation() = default;

    explicit das33_project_create_operation(const account_id_type& authority,
					    const string& name,
					    const account_id_type& owner,
					    const asset_id_type& token,
					    const vector<price>& ratios,
					    share_type min_to_collect)
	      : authority(authority)
              , name(name)
	      , owner(owner)
	      , token(token)
	      , ratios(ratios)
	      , min_to_collect(min_to_collect) {}

    account_id_type fee_payer() const { return authority; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  struct das33_project_update_operation : public base_operation
  {
    struct fee_parameters_type {};
    asset fee;

    account_id_type                        authority;
    das33_project_id_type                  project_id;
    optional<string>                       name;
    optional<account_id_type>              owner;
    vector<price>                          ratios;
    optional<share_type>                   min_to_collect;
    optional<uint8_t>                      status;
    extensions_type                        extensions;

    das33_project_update_operation() = default;

    account_id_type fee_payer() const { return authority; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  struct das33_project_delete_operation : public base_operation
    {
      struct fee_parameters_type {};
      asset fee;

      account_id_type       authority;
      das33_project_id_type project_id;
      extensions_type       extensions;

      das33_project_delete_operation() = default;

      explicit das33_project_delete_operation(const account_id_type& authority, const das33_project_id_type& project_id)
  	      : authority(authority)
  	      , project_id(project_id) {}

      account_id_type fee_payer() const
      {
	return authority;
      }
      void validate() const;
      share_type calculate_fee(const fee_parameters_type&) const { return 0; }
    };

    struct das33_pledge_asset_operation : public base_operation
    {
      struct fee_parameters_type {};
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
      share_type calculate_fee(const fee_parameters_type&) const { return 0; }
    };

} }  // namespace graphene::chain

////////////////////////////////
// REFLECTIONS:               //
////////////////////////////////

FC_REFLECT( graphene::chain::das33_pledge_asset_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::das33_pledge_asset_operation,
            (fee)
            (account_id)
            (pledged)
            (license_id)
            (project_id)
            (extensions)
          )

FC_REFLECT( graphene::chain::das33_project_create_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::das33_project_create_operation,
	    (fee)
	    (authority)
	    (name)
	    (owner)
	    (token)
	    (ratios)
	    (min_to_collect)
	    (extensions)
	  )

FC_REFLECT( graphene::chain::das33_project_update_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::das33_project_update_operation,
	    (fee)
	    (authority)
	    (project_id)
	    (name)
	    (owner)
	    (ratios)
	    (min_to_collect)
	    (status)
	    (extensions)
	  )

FC_REFLECT( graphene::chain::das33_project_delete_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::das33_project_delete_operation,
	    (authority)
	    (project_id)
	    (extensions)
	  )
