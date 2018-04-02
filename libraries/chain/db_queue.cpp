/*
 * MIT License
 *
 * Copyright (c) 2018 TechSolutions Ltd.
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

#include <graphene/chain/database.hpp>

#include <graphene/chain/queue_objects.hpp>

#include <fc/optional.hpp>

#include <cmath>

namespace graphene {
namespace chain {

object_id_type database::push_queue_submission(const string& origin, optional<license_type_id_type> license,
                                               account_id_type account, share_type amount, share_type frequency,
                                               const string& comment)
{
    share_type historic_sum = 0;
    auto dascoin_amount = cycles_to_dascoin(amount, frequency);
    const auto dgp = get_dynamic_global_properties();

    const auto& queue = get_index_type<reward_queue_index>().indices().get<by_time>();
    if (queue.size() == 0) {
        historic_sum = dgp.total_dascoin_minted + dascoin_amount;
    }
    else {
        const auto last_el_it = queue.rbegin();
        historic_sum = last_el_it->historic_sum + dascoin_amount;
    }

    // Set submission number:
    auto number = dgp.max_queue_submission_num + 1;
    modify(dgp, [](dynamic_global_property_object& dgpo){
        dgpo.max_queue_submission_num++;
    });

    return create<reward_queue_object>([&](reward_queue_object& rqo) {
               rqo.number = number;
               rqo.origin = origin;
               rqo.license = license;
               rqo.account = account;
               rqo.amount = amount;
               rqo.frequency = frequency;
               rqo.time = head_block_time();
               rqo.comment = comment;
               rqo.historic_sum = historic_sum;
           })
        .id;
}

uint32_t get_time_on_queue(share_type historic_sum, share_type total_dascoin_minted, share_type reward_amount,
                           uint32_t reward_interval)
{
    auto ra = static_cast<double>(reward_amount.value);
    auto diff = historic_sum - total_dascoin_minted;

    auto res = std::floor(diff.value / ra) * reward_interval;

    return static_cast<uint32_t>(res);
}


}  // namespace chain
}  // namespace graphene
