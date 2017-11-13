/**
 * DASCOIN!
 */

#pragma once
#include <graphene/chain/protocol/types.hpp>

namespace graphene { namespace chain {

  class upgrade_type
  {
  public:
    typedef upgrade_multiplier_type array_type;

    upgrade_type() = default;
    explicit upgrade_type(const array_type& mul) : multipliers(mul) { max = static_cast<uint8_t>(mul.size()); }

    void reset(const array_type& mul) { max = static_cast<uint8_t>(mul.size()); used = 0; multipliers = mul; }
    void improve(const upgrade_type& new_upgrade);
    bool based_on(const upgrade_type& base_upgrade) const;

    share_type operator() (share_type x);
    upgrade_type& operator+= (const upgrade_type& rhs) { improve(rhs); return *this; }
    bool operator< (const upgrade_type& rhs) const { return max < rhs.max; }
    bool operator== (const upgrade_type& rhs) const { return based_on(rhs) && used == rhs.used; }

    uint8_t max = 0;
    uint8_t used = 0;
    array_type multipliers;
  };

} }  // namesapce graphene::chain

FC_REFLECT( graphene::chain::upgrade_type,
            (max)
            (used)
            (multipliers)
          )
