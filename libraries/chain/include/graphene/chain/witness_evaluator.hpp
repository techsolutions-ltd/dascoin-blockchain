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
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/witness_object.hpp>

namespace graphene { namespace chain {

   class witness_create_evaluator : public evaluator<witness_create_evaluator>
   {
      public:
         typedef witness_create_operation operation_type;

         void_result do_evaluate( const witness_create_operation& o );
         object_id_type do_apply( const witness_create_operation& o );
   };

   class witness_update_evaluator : public evaluator<witness_update_evaluator>
   {
      public:
         typedef witness_update_operation operation_type;

         void_result do_evaluate( const witness_update_operation& o );
         void_result do_apply( const witness_update_operation& o );
   };

   class remove_root_authority_evaluator : public evaluator<remove_root_authority_evaluator>
   {
      public:
         typedef remove_root_authority_operation operation_type;

         void_result do_evaluate( const remove_root_authority_operation& o );
         void_result do_apply( const remove_root_authority_operation& o );
   };

   class create_witness_evaluator : public evaluator<create_witness_evaluator>
   {
      public:
         typedef create_witness_operation operation_type;

         void_result do_evaluate( const create_witness_operation& o );
         object_id_type do_apply( const create_witness_operation& o );
   };

   class update_witness_evaluator : public evaluator<update_witness_evaluator>
   {
      public:
         typedef update_witness_operation operation_type;

         void_result do_evaluate( const update_witness_operation& o );
         void_result do_apply( const update_witness_operation& o );
   };

   class remove_witness_evaluator : public evaluator<remove_witness_evaluator>
   {
      public:
         typedef remove_witness_operation operation_type;

         void_result do_evaluate( const remove_witness_operation& o );
         void_result do_apply( const remove_witness_operation& o );
   };

   class activate_witness_evaluator : public evaluator<activate_witness_evaluator>
   {
      public:
         typedef activate_witness_operation operation_type;

         void_result do_evaluate( const activate_witness_operation& o );
         void_result do_apply( const activate_witness_operation& o );
   };

   class deactivate_witness_evaluator : public evaluator<deactivate_witness_evaluator>
   {
      public:
         typedef deactivate_witness_operation operation_type;

         void_result do_evaluate( const deactivate_witness_operation& o );
         void_result do_apply( const deactivate_witness_operation& o );
   };

   class witness_delegate_data_evaluator
   {
         database& db;
      public:
         typedef void result_type;

         witness_delegate_data_evaluator(database& db) : db(db){}
         void operator ()(const update_witness_delegate_data& o);
         void operator ()(const remove_witness_delegate_data& o);
         void operator ()(const activate_witness_delegate_data& o);
         void operator ()(const deactivate_witness_delegate_data& o);
   };

} } // graphene::chain
