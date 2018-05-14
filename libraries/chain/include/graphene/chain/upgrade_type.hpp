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
    bool has_remaining_upgrades() const { return used < max; }

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
