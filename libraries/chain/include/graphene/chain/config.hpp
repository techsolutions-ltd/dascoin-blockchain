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

#define GRAPHENE_SYMBOL "CORE"
#define GRAPHENE_ADDRESS_PREFIX "GPH"

#define GRAPHENE_MIN_ACCOUNT_NAME_LENGTH 3
#define GRAPHENE_MAX_ACCOUNT_NAME_LENGTH 63

#define GRAPHENE_MIN_ASSET_SYMBOL_LENGTH 3
#define GRAPHENE_MAX_ASSET_SYMBOL_LENGTH 16

#define GRAPHENE_MAX_SHARE_SUPPLY int64_t(1000000000000000ll)
#define GRAPHENE_MAX_PAY_RATE 10000 /* 100% */
#define GRAPHENE_MAX_SIG_CHECK_DEPTH 2
/**
 * Don't allow the committee_members to publish a limit that would
 * make the network unable to operate.
 */
#define GRAPHENE_MIN_TRANSACTION_SIZE_LIMIT 1024
#define GRAPHENE_MIN_BLOCK_INTERVAL   1 /* seconds */
#define GRAPHENE_MAX_BLOCK_INTERVAL  30 /* seconds */

#define GRAPHENE_DEFAULT_BLOCK_INTERVAL  6 /* seconds */
#define GRAPHENE_DEFAULT_MAX_TRANSACTION_SIZE 2048
#define GRAPHENE_DEFAULT_MAX_BLOCK_SIZE  (static_cast<uint64_t>(GRAPHENE_DEFAULT_MAX_TRANSACTION_SIZE*GRAPHENE_DEFAULT_BLOCK_INTERVAL*200000ull))
#define GRAPHENE_DEFAULT_MAX_TIME_UNTIL_EXPIRATION (60*5) // seconds, aka: 5 minutes
#define GRAPHENE_DEFAULT_MAINTENANCE_INTERVAL  (24*60*60) // seconds, aka: 1 day
#define GRAPHENE_DEFAULT_MAINTENANCE_SKIP_SLOTS 1  // number of slots to skip for maintenance interval

#define GRAPHENE_MIN_UNDO_HISTORY 10
#define GRAPHENE_MAX_UNDO_HISTORY 10000

#define GRAPHENE_MIN_BLOCK_SIZE_LIMIT (GRAPHENE_MIN_TRANSACTION_SIZE_LIMIT*5) // 5 transactions per block
#define GRAPHENE_MIN_TRANSACTION_EXPIRATION_LIMIT (GRAPHENE_MAX_BLOCK_INTERVAL * 5) // 5 transactions per block
#define GRAPHENE_BLOCKCHAIN_PRECISION                           uint64_t( 100000 )

#define GRAPHENE_BLOCKCHAIN_PRECISION_DIGITS                    5
#define GRAPHENE_DEFAULT_TRANSFER_FEE                           (1*GRAPHENE_BLOCKCHAIN_PRECISION)
#define GRAPHENE_MAX_INSTANCE_ID                                (uint64_t(-1)>>16)
/** percentage fields are fixed point with a denominator of 10,000 */
#define GRAPHENE_100_PERCENT                                    10000
#define GRAPHENE_1_PERCENT                                      (GRAPHENE_100_PERCENT/100)
/** NOTE: making this a power of 2 (say 2^15) would greatly accelerate fee calcs */
#define GRAPHENE_MAX_MARKET_FEE_PERCENT                         GRAPHENE_100_PERCENT
#define GRAPHENE_DEFAULT_FORCE_SETTLEMENT_DELAY                 (60*60*24) ///< 1 day
#define GRAPHENE_DEFAULT_FORCE_SETTLEMENT_OFFSET                0 ///< 1%
#define GRAPHENE_DEFAULT_FORCE_SETTLEMENT_MAX_VOLUME            (20* GRAPHENE_1_PERCENT) ///< 20%
#define GRAPHENE_DEFAULT_PRICE_FEED_LIFETIME                    (60*60*24) ///< 1 day
#define GRAPHENE_MAX_FEED_PRODUCERS                             200
#define GRAPHENE_DEFAULT_MAX_AUTHORITY_MEMBERSHIP               10
#define GRAPHENE_DEFAULT_MAX_ASSET_WHITELIST_AUTHORITIES        10
#define GRAPHENE_DEFAULT_MAX_ASSET_FEED_PUBLISHERS              10

/**
 *  These ratios are fixed point numbers with a denominator of GRAPHENE_COLLATERAL_RATIO_DENOM, the
 *  minimum maitenance collateral is therefore 1.001x and the default
 *  maintenance ratio is 1.75x
 */
///@{
#define GRAPHENE_COLLATERAL_RATIO_DENOM                 1000
#define GRAPHENE_MIN_COLLATERAL_RATIO                   1001  ///< lower than this could result in divide by 0
#define GRAPHENE_MAX_COLLATERAL_RATIO                   32000 ///< higher than this is unnecessary and may exceed int16 storage
#define GRAPHENE_DEFAULT_MAINTENANCE_COLLATERAL_RATIO   1750 ///< Call when collateral only pays off 175% the debt
#define GRAPHENE_DEFAULT_MAX_SHORT_SQUEEZE_RATIO        1500 ///< Stop calling when collateral only pays off 150% of the debt
///@}
#define GRAPHENE_DEFAULT_MARGIN_PERIOD_SEC              (30*60*60*24)

#define GRAPHENE_DEFAULT_MIN_WITNESS_COUNT                    (1)
#define GRAPHENE_DEFAULT_MIN_COMMITTEE_MEMBER_COUNT           (1)
#define GRAPHENE_DEFAULT_MAX_WITNESSES                        (1001) // SHOULD BE ODD
#define GRAPHENE_DEFAULT_MAX_COMMITTEE                        (1001) // SHOULD BE ODD
#define GRAPHENE_DEFAULT_MAX_PROPOSAL_LIFETIME_SEC            (60*60*24*7*4) // Four weeks
#define GRAPHENE_DEFAULT_COMMITTEE_PROPOSAL_REVIEW_PERIOD_SEC (60*60*24*7*2) // Two weeks
#define GRAPHENE_DEFAULT_NETWORK_PERCENT_OF_FEE               (20*GRAPHENE_1_PERCENT)
#define GRAPHENE_DEFAULT_LIFETIME_REFERRER_PERCENT_OF_FEE     (30*GRAPHENE_1_PERCENT)
#define GRAPHENE_DEFAULT_MAX_BULK_DISCOUNT_PERCENT            (50*GRAPHENE_1_PERCENT)
#define GRAPHENE_DEFAULT_BULK_DISCOUNT_THRESHOLD_MIN          ( GRAPHENE_BLOCKCHAIN_PRECISION*int64_t(1000) )
#define GRAPHENE_DEFAULT_BULK_DISCOUNT_THRESHOLD_MAX          ( GRAPHENE_DEFAULT_BULK_DISCOUNT_THRESHOLD_MIN*int64_t(100) )
#define GRAPHENE_DEFAULT_CASHBACK_VESTING_PERIOD_SEC          (60*60*24*365) ///< 1 year
#define GRAPHENE_DEFAULT_CASHBACK_VESTING_THRESHOLD           (GRAPHENE_BLOCKCHAIN_PRECISION*int64_t(100))
#define GRAPHENE_DEFAULT_BURN_PERCENT_OF_FEE                  (20*GRAPHENE_1_PERCENT)
#define GRAPHENE_WITNESS_PAY_PERCENT_PRECISION                (1000000000)
#define GRAPHENE_DEFAULT_MAX_ASSERT_OPCODE                    1
#define GRAPHENE_DEFAULT_FEE_LIQUIDATION_THRESHOLD            GRAPHENE_BLOCKCHAIN_PRECISION * 100;
#define GRAPHENE_DEFAULT_ACCOUNTS_PER_FEE_SCALE               1000
#define GRAPHENE_DEFAULT_ACCOUNT_FEE_SCALE_BITSHIFTS          4
#define GRAPHENE_DEFAULT_MAX_BUYBACK_MARKETS                  4

#define GRAPHENE_MAX_WORKER_NAME_LENGTH                       63

#define GRAPHENE_MAX_URL_LENGTH                               127

// counter initialization values used to derive near and far future seeds for shuffling witnesses
// we use the fractional bits of sqrt(2) in hex
#define GRAPHENE_NEAR_SCHEDULE_CTR_IV                    ( (uint64_t( 0x6a09 ) << 0x30)    \
                                                         | (uint64_t( 0xe667 ) << 0x20)    \
                                                         | (uint64_t( 0xf3bc ) << 0x10)    \
                                                         | (uint64_t( 0xc908 )        ) )

// and the fractional bits of sqrt(3) in hex
#define GRAPHENE_FAR_SCHEDULE_CTR_IV                     ( (uint64_t( 0xbb67 ) << 0x30)    \
                                                         | (uint64_t( 0xae85 ) << 0x20)    \
                                                         | (uint64_t( 0x84ca ) << 0x10)    \
                                                         | (uint64_t( 0xa73b )        ) )

/**
 * every second, the fraction of burned core asset which cycles is
 * GRAPHENE_CORE_ASSET_CYCLE_RATE / (1 << GRAPHENE_CORE_ASSET_CYCLE_RATE_BITS)
 */
#define GRAPHENE_CORE_ASSET_CYCLE_RATE                        17
#define GRAPHENE_CORE_ASSET_CYCLE_RATE_BITS                   32

#define GRAPHENE_DEFAULT_WITNESS_PAY_PER_BLOCK            (GRAPHENE_BLOCKCHAIN_PRECISION * int64_t( 10) )
#define GRAPHENE_DEFAULT_WITNESS_PAY_VESTING_SECONDS      (60*60*24)
#define GRAPHENE_DEFAULT_WORKER_BUDGET_PER_DAY            (GRAPHENE_BLOCKCHAIN_PRECISION * int64_t(500) * 1000 )

#define GRAPHENE_DEFAULT_MINIMUM_FEEDS                       7

#define GRAPHENE_MAX_INTEREST_APR                            uint16_t( 10000 )

#define GRAPHENE_RECENTLY_MISSED_COUNT_INCREMENT             4
#define GRAPHENE_RECENTLY_MISSED_COUNT_DECREMENT             3

#define GRAPHENE_CURRENT_DB_VERSION                          "GPH2.5"

#define GRAPHENE_IRREVERSIBLE_THRESHOLD                      (70 * GRAPHENE_1_PERCENT)

/**
 *  Reserved Account IDs with special meaning
 */
///@{
/// Represents the current committee members, two-week review period
#define GRAPHENE_COMMITTEE_ACCOUNT (graphene::chain::account_id_type(0))
/// Represents the current witnesses
#define GRAPHENE_WITNESS_ACCOUNT (graphene::chain::account_id_type(1))
/// Represents the current committee members
#define GRAPHENE_RELAXED_COMMITTEE_ACCOUNT (graphene::chain::account_id_type(2))
/// Represents the canonical account with NO authority (nobody can access funds in null account)
#define GRAPHENE_NULL_ACCOUNT (graphene::chain::account_id_type(3))
/// Represents the canonical account with WILDCARD authority (anybody can access funds in temp account)
#define GRAPHENE_TEMP_ACCOUNT (graphene::chain::account_id_type(4))
/// Represents the canonical account for specifying you will vote directly (as opposed to a proxy)
#define GRAPHENE_PROXY_TO_SELF_ACCOUNT (graphene::chain::account_id_type(5))
/// Sentinel value used in the scheduler.
#define GRAPHENE_NULL_WITNESS (graphene::chain::witness_id_type(0))
///@}

// hack for unit test
#define GRAPHENE_FBA_STEALTH_DESIGNATED_ASSET (asset_id_type(1))

/**
 * Maximum length of comments for operations:
 */
#define DASCOIN_MAX_COMMENT_LENGTH (128)

/**
 * Maximum length for the wire out memo:
 */
#define DASCOIN_MAXIMUM_INTERNAL_MEMO_LENGTH (128)

#define DASCOIN_DEFAULT_MINIMUM_TRANSFER_LIMIT (0)
#define DASCOIN_DEFAULT_MAXIMUM_TRANSFER_LIMIT (100000)

/**
 * Core system assets (i.e. Dascoin) should have the following precision:
 */
///@{
#define DASCOIN_DEFAULT_ASSET_PRECISION_DIGITS (5)
#define DASCOIN_DEFAULT_ASSET_PRECISION (static_cast<uint64_t>(100000))

#define DASCOIN_FIAT_ASSET_PRECISION_DIGITS (2)
#define DASCOIN_FIAT_ASSET_PRECISION (static_cast<uint64_t>(100))
///@}

#define DASCOIN_WEBASSET_SYMBOL "WEBEUR"
#define DASCOIN_WEB_ASSET_INDEX (1)

#define DASCOIN_CYCLE_ASSET_SYMBOL "CYCLE"
#define DASCOIN_CYCLE_ASSET_INDEX (3)
#define DASCOIN_CYCLE_ASSET_PRECISION_DIGITS (0)
#define DASCOIN_CYCLE_ASSET_PRECISION (static_cast<uint64_t>(1))

#define DASCOIN_DEFAULT_LIMIT_INTERVAL_ELAPSE_TIME_SECONDS (86400)
#define DASCOIN_DEFAULT_WEB_ASSET_REQUEST_EXPIRATION_TIME_SECONDS (86400)
#define DASCOIN_DEFAULT_CYCLE_REQUEST_EXPIRATION_TIME_SECONDS (86400)

/**
 * Number of days between upgrade events:
 */
#define DASCOIN_DEFAULT_UPGRADE_EVENT_INTERVAL_DAYS (108)

#define DASCOIN_DEFAULT_REWARD_INTERVAL_TIME_SECONDS (10*60)
#define DASCOIN_DEFAULT_DASCOIN_REWARD_AMOUNT (2000 * DASCOIN_DEFAULT_ASSET_PRECISION)

/**
 * Dascoin asset parameters:
 */
///@{
#define DASCOIN_DASCOIN_SYMBOL "DAS"
#define DASCOIN_DASCOIN_INDEX (2)
#define DASCOIN_MAX_DASCOIN_SUPPLY int64_t(8589934592LL)    // 2^33
///}@

/**
 * Bitcoin asset parameters:
 */
///@{
#define DASCOIN_BITCOIN_SYMBOL "BTC"
#define DASCOIN_BITCOIN_INDEX (4)
#define DASCOIN_BITCOIN_PRECISION_DIGITS (8)
#define DASCOIN_BITCOIN_PRECISION (100000000LL)
#define DASCOIN_MAX_BITCOIN_SUPPLY int64_t(10000000LL)
///}@


/**
 * Frequency_type parameters:
 */
///@{
#define DASCOIN_FREQUENCY_PRECISION_DIGITS (2)
#define DASCOIN_FREQUENCY_PRECISION (static_cast<uint64_t>(100))
#define DASCOIN_INITIAL_FREQUENCY (200)
///@}

/**
 *  Minimum dascoin reward amount:
 */
#define DASCOIN_MIN_DASCOIN_REWARD_AMOUNT (0)

/**
 * Reserved object ids with special meaning:
 */
///@{
/// Represents invalid or non-existant license
#define DASCOIN_NULL_LICENSE (graphene::chain::license_type_id_type(0))
///@}

/**
 * Base cycle values for license types:
 */
///@{
#define DASCOIN_BASE_STANDARD_CYCLES (1100)
#define DASCOIN_BASE_MANAGER_CYCLES (5500)
#define DASCOIN_BASE_PRO_CYCLES (24000)
#define DASCOIN_BASE_EXECUTIVE_CYCLES (65000)
#define DASCOIN_BASE_EXECUTIVE_CYCLES_NEW_VALUE (72000)
#define DASCOIN_BASE_VICE_PRESIDENT_CYCLES (225000)
#define DASCOIN_BASE_VICE_PRESIDENT_CYCLES_NEW_VALUE (180000)
#define DASCOIN_BASE_PRESIDENT_CYCLES (325000)
///@}

/**
 * Eur limits for transferring vault to wallet:
 */
///@{
#define DASCOIN_DEFAULT_EUR_LIMIT_ADVOCATE (static_cast<share_type>(113))
#define DASCOIN_DEFAULT_EUR_LIMIT_STANDARD (5 * DASCOIN_FIAT_ASSET_PRECISION)
#define DASCOIN_DEFAULT_EUR_LIMIT_MANAGER (10 * DASCOIN_FIAT_ASSET_PRECISION)
#define DASCOIN_DEFAULT_EUR_LIMIT_PRO (25 * DASCOIN_FIAT_ASSET_PRECISION)
#define DASCOIN_DEFAULT_EUR_LIMIT_EXECUTIVE (50 * DASCOIN_FIAT_ASSET_PRECISION)
#define DASCOIN_DEFAULT_EUR_LIMIT_VICE_PRESIDENT (100 * DASCOIN_FIAT_ASSET_PRECISION)
#define DASCOIN_DEFAULT_EUR_LIMIT_PRESIDENT (static_cast<share_type>(187.50 * DASCOIN_FIAT_ASSET_PRECISION))
///@}

/**
 * Dascoin default starting price components:
 */
///@{
#define DASCOIN_DEFAULT_STARTING_PRICE_BASE_AMOUNT (1 * DASCOIN_DEFAULT_ASSET_PRECISION)
#define DASCOIN_DEFAULT_STARTING_PRICE_QUOTE_AMOUNT (100 * DASCOIN_FIAT_ASSET_PRECISION)
///@}

/**
 * Max license name length
 */
///@{
#define DASCOIN_MAX_LICENSE_NAME_LEN (64)
///@}

/**
 * Default starting amount of cycle asset for each account
 */
///@{
#define DASCOIN_DEFAULT_STARTING_CYCLE_ASSET_AMOUNT (100)
///@}


/**
 * DasPay parameters:
 */
///@{
#define DASPAY_DEFAULT_CLEARING_ENABLED  (false) ///< by default off
#define DASPAY_DEFAULT_CLEARING_INTERVAL_TIME_SECONDS (300) ///< in seconds
#define DASPAY_DEFAULT_CLEARING_COLLATERAL_DASC (0 * DASCOIN_DEFAULT_ASSET_PRECISION) ///< by default set to 0
#define DASPAY_DEFAULT_CLEARING_COLLATERAL_WEBEUR (0 * DASCOIN_FIAT_ASSET_PRECISION) ///< by default set to 0
///@}

/**
 * Delayed operations resolver parameters:
 */
///@{
#define DASCOIN_DEFAULT_DELAYED_OPERATIONS_RESOLVER_ENABLED  (false) ///< by default off
#define DASCOIN_DEFAULT_DELAYED_OPERATIONS_RESOLVER_INTERVAL_TIME_SECONDS (30)  ///< in seconds
///@}

#define ORDER_BOOK_QUERY_PRECISION (static_cast<uint64_t>(1000000))
#define ORDER_BOOK_GROUP_QUERY_PRECISION_DIFF (static_cast<uint64_t>(10000))

/**
 * Das33 parameters:
 */
///@{
#define DAS33_DEFAULT_USE_EXTERNAL_BTC_PRICE  (true)
///@}
