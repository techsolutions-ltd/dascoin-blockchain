
#include <graphene/chain/virtual_operation_helper.hpp>


namespace graphene { namespace chain {

bool is_virtual_operation(const operation& op)
{
   return op.which() >= operation_type_limits::virtual_index_start && op.which() <= operation_type_limits::virtual_index_end;
}

bool is_virtual_operation(const unsigned operation_num)
{
   return operation_num >= operation_type_limits::virtual_index_start && operation_num <= operation_type_limits::virtual_index_end;
}

} }
