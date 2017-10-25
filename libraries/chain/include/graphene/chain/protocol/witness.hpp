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

namespace graphene { namespace chain { 

  /**
    * @brief Create a witness object, as a bid to hold a witness position on the network.
    * @ingroup operations
    *
    * Accounts which wish to become witnesses may use this operation to create a witness object which stakeholders may
    * vote on to approve its position as a witness.
    */
   struct witness_create_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = 5000 * GRAPHENE_BLOCKCHAIN_PRECISION; };

      asset             fee;
      /// The account which owns the witness. This account pays the fee for this operation.
      account_id_type   witness_account;
      string            url;
      public_key_type   block_signing_key;

      account_id_type fee_payer()const { return witness_account; }
      void            validate()const;
   };

  /**
    * @brief Update a witness object's URL and block signing key.
    * @ingroup operations
    */
   struct witness_update_operation : public base_operation
   {
      struct fee_parameters_type
      {
         share_type fee = 20 * GRAPHENE_BLOCKCHAIN_PRECISION;
      };

      asset             fee;
      /// The witness object to update.
      witness_id_type   witness;
      /// The account which owns the witness. This account pays the fee for this operation.
      account_id_type   witness_account;
      /// The new URL.
      optional< string > new_url;
      /// The new block signing key.
      optional< public_key_type > new_signing_key;

      account_id_type fee_payer()const { return witness_account; }
      void            validate()const;
   };

   struct remove_root_authority_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = 0; };

      asset             fee;
      /// Root account whose authority we will revoke. This account pays the fee for this operation.
      account_id_type   root_account;

      string            comment;

      account_id_type fee_payer()const { return root_account; }
      void            validate()const{}
   };

   struct create_witness_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = 0; };

      asset             fee;
      /// Root account authority. This account pays the fee for this operation.
      account_id_type   authority;
      /// Existing account that we want to promote into a master node candidate.
      account_id_type   witness_account;
      /// Public key that is used for signing blocks
      public_key_type   block_signing_key;

      string            url;
      string            comment;
      account_id_type fee_payer()const { return authority; }
      void            validate()const{}
   };

   struct update_witness_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = 0; };

      asset             fee;
      witness_id_type   witness;

      /// Root account authority. This account pays the fee for this operation.
      account_id_type   authority;
      /// Existing account that we want to promote into a master node candidate.
      optional < account_id_type >   witness_account;
      /// Public key that is used for signing blocks
      optional < public_key_type >   block_signing_key;
      optional < string >            url;
      optional < string >            comment;

      account_id_type fee_payer()const { return authority; }
      void            validate()const{}
   };

   struct remove_witness_operation : public base_operation
   {
      struct fee_parameters_type { uint64_t fee = 0; };

      asset                fee;
      witness_id_type      witness;
      /// Root account authority. This account pays the fee for this operation.
      account_id_type      authority;
      optional < string >  comment;

      account_id_type fee_payer()const { return authority; }
      void            validate()const{}
   };


   /// TODO: witness_resign_operation : public base_operation

} } // graphene::chain

FC_REFLECT( graphene::chain::witness_create_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::witness_create_operation, (fee)(witness_account)(url)(block_signing_key) )

FC_REFLECT( graphene::chain::witness_update_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::witness_update_operation, (fee)(witness)(witness_account)(new_url)(new_signing_key) )

FC_REFLECT( graphene::chain::remove_root_authority_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::remove_root_authority_operation, (fee)(root_account)(comment) )

FC_REFLECT( graphene::chain::create_witness_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::create_witness_operation, (fee)(authority)(witness_account)(block_signing_key)(url)(comment) )

FC_REFLECT( graphene::chain::update_witness_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::update_witness_operation, (fee)(witness)(authority)(witness_account)(block_signing_key)(url)(comment) )

FC_REFLECT( graphene::chain::remove_witness_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::remove_witness_operation, (fee)(witness)(authority)(comment) )
