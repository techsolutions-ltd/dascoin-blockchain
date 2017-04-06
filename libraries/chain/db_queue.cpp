/**
 * DASCOIN!
 */

#include <graphene/chain/database.hpp>

#include <graphene/chain/queue_objects.hpp>

#include <fc/optional.hpp>

namespace graphene {
namespace chain {

object_id_type database::push_queue_submission(const string& origin, optional<license_type_id_type> license,
                                               account_id_type account, share_type amount, share_type frequency,
                                               const string& comment)
{
    return create<reward_queue_object>([&](reward_queue_object& rqo) {
               rqo.origin = origin;
               rqo.license = license;
               rqo.account = account;
               rqo.amount = amount;
               rqo.frequency = frequency;
               rqo.time = head_block_time();
               rqo.comment = comment;
           }).id;
}


}  // namespace chain
}  // namespace graphene
