/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/chain_parameters.hpp>

namespace graphene { namespace chain {

   /**
    * @brief Create a committee_member object, as a bid to hold a committee_member seat on the network.
    * @ingroup operations
    *
    * Accounts which wish to become committee_members may use this operation to create a committee_member object which stakeholders may
    * vote on to approve its position as a committee_member.
    */
   struct committee_member_create_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = 5000 * GRAPHENE_BLOCKCHAIN_PRECISION; };

      asset                                 fee;
      /// The account which owns the committee_member. This account pays the fee for this operation.
      account_id_type                       committee_member_account;
      string                                url;

      account_id_type fee_payer()const { return committee_member_account; }
      void            validate()const;
   };

   /**
    * @brief Update a committee_member object.
    * @ingroup operations
    *
    * Currently the only field which can be updated is the `url`
    * field.
    */
   struct committee_member_update_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = 20 * GRAPHENE_BLOCKCHAIN_PRECISION; };

      asset                                 fee;
      /// The committee member to update.
      committee_member_id_type              committee_member;
      /// The account which owns the committee_member. This account pays the fee for this operation.
      account_id_type                       committee_member_account;
      optional< string >                    new_url;

      account_id_type fee_payer()const { return committee_member_account; }
      void            validate()const;
   };

   /**
    * @brief Used by committee_members to update the global parameters of the blockchain.
    * @ingroup operations
    *
    * This operation allows the committee_members to update the global parameters on the blockchain. These control various
    * tunable aspects of the chain, including block and maintenance intervals, maximum data sizes, the fees charged by
    * the network, etc.
    *
    * This operation may only be used in a proposed transaction, and a proposed transaction which contains this
    * operation must have a review period specified in the current global parameters before it may be accepted.
    */
   struct committee_member_update_global_parameters_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };

      asset             fee;
      chain_parameters  new_parameters;

      account_id_type fee_payer()const { return account_id_type(); }
      void            validate()const;
   };


    /**
     * @brief Used by committee_members to update the license issuing authority on the blockchain.
     * @ingroup operations
     *
     * This operation allows the committee_members to update the license issuing authority in the global_properties
     * object on the blockhain. This authority has the power to create licenses, to issue license grant requests to
     * users. It is checked by the license authorization authority.
     *
     * This operation may only be used in a proposed transaction, and a proposed transaction which contains this
     * operation must have a review period specified in the current global parameters before it may be accepted.
     */
    struct committee_member_update_license_issuer_operation : public base_operation
    {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };  // TODO: zero the fee?

      asset fee;
      /// The account to propose:
      account_id_type license_issuer;
      /// The commitee member that is proposing the change:
      account_id_type committee_member_account;

      account_id_type fee_payer()const { return committee_member_account; }
      void            validate()const;
    };

    /**
     * @brief Used by committee_members to update the license authentication authority on the blockchain.
     * @ingroup operations
     *
     * This operation allows the committee_members to update the license authentication authority in the
     * global_properties object on the blockhain. This authority has to verify and confirm license requests that the
     * license issuing authority has issued to an user.
     *
     * This operation may only be used in a proposed transaction, and a proposed transaction which contains this
     * operation must have a review period specified in the current global parameters before it may be accepted.
     */
    struct committee_member_update_license_authenticator_operation : public base_operation
    {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };  // TODO: zero the fee?

      asset fee;
      /// The account to propose:
      account_id_type license_authenticator;
      /// The commitee member that is proposing the change:
      account_id_type committee_member_account;

      account_id_type fee_payer()const { return committee_member_account; }
      void            validate()const;
    };

    /**
     * @brief Used by commitee members to update the account registrar on the blockchain.
     * @ingroup operations
     *
     * This operation allows the committee members to update the account registrar authority in the global properties
     * object on the blockchain. This authority must sign each account registration into the blockchain.
     *
     * This operation may only be used in a proposed transaction, and a proposed transaction which contains this
     * operation must have a review period specified in the current global parameters before it may be accepted.
     */
    struct committee_member_update_account_registrar_operation : public base_operation
    {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };  // TODO: zero the fee?

      asset fee;

      account_id_type registrar;  // The account to propose.
      account_id_type committee_member_account;  // The committee member that is proposing the change.

      account_id_type fee_payer()const { return committee_member_account; }
      void validate()const;
    };

   /// TODO: committee_member_resign_operation : public base_operation

} } // graphene::chain

FC_REFLECT( graphene::chain::committee_member_create_operation::fee_parameters_type,
            (fee)
          )

FC_REFLECT( graphene::chain::committee_member_update_operation::fee_parameters_type,
            (fee)
          )

FC_REFLECT( graphene::chain::committee_member_update_global_parameters_operation::fee_parameters_type,
            (fee)
          )

FC_REFLECT( graphene::chain::committee_member_update_license_issuer_operation::fee_parameters_type,
            (fee)
          )

FC_REFLECT( graphene::chain::committee_member_update_license_authenticator_operation::fee_parameters_type,
            (fee)
          )

FC_REFLECT( graphene::chain::committee_member_update_account_registrar_operation::fee_parameters_type,
            (fee)
          )

FC_REFLECT( graphene::chain::committee_member_create_operation,
            (fee)
            (committee_member_account)
            (url)
          )

FC_REFLECT( graphene::chain::committee_member_update_operation,
            (fee)
            (committee_member)
            (committee_member_account)
            (new_url)
          )

FC_REFLECT( graphene::chain::committee_member_update_global_parameters_operation,
            (fee)
            (new_parameters)
          );

FC_REFLECT( graphene::chain::committee_member_update_license_issuer_operation,
            (fee)
            (license_issuer)
            (committee_member_account)
          );

FC_REFLECT( graphene::chain::committee_member_update_license_authenticator_operation,
            (fee)
            (license_authenticator)
            (committee_member_account)
          );

FC_REFLECT( graphene::chain::committee_member_update_account_registrar_operation,
            (fee)
            (registrar)
            (committee_member_account)
          );
