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

#include "database_fixture.hpp"
#include <graphene/chain/daspay_object.hpp>


using namespace graphene::chain::test;

namespace graphene { namespace chain {

  vector<payment_service_provider_object> database_fixture::get_payment_service_providers() const
  {
    const auto& idx = db.get_index_type<payment_service_provider_index>().indices().get<by_payment_service_provider>();
    auto itr = idx.begin();
    vector<payment_service_provider_object> result;

    while( itr != idx.end() )
      result.emplace_back(*itr++);

    return result;
  }

  void database_fixture::set_daspay_clearing_enabled(bool state)
  { try {
    db.modify(get_global_properties(), [state](global_property_object& gpo) {
      gpo.daspay_parameters.clearing_enabled = state;
    });

  } FC_LOG_AND_RETHROW() };

} }  // namespace graphene::chain
