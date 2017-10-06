# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [0.2.2] - 2017-10-06
### Added
 - Query to get limit orders grouped by price
### Updated
 - get_24_volume replaced with get_24_hi_low_volume

## [0.2.1] - 2017-09-29
### Updated
 - Hotfix for reward amount to match Dascoin precision

## [0.2.0] - 2017-09-29
### Added
 - EUR limits on license types
 - Tracking latest and daily dascoin price
 - Transfer limits on balances based on dascoin price
 - New operation: issue free cycles
 - Get vault info and get vaults info agregate call
 - Issued asset record object
 - New operation: update eur limits
 - Frequency history object
 - Get limit orders for accounts api call

### Updated
 - Asset create issue operation unique_id and comment fields
 - Fallback dascoin price in genesis.json
 - Block timestamp in operation history
 - Major changes: patched in Bitshares subscription infrastructure
 - Now tracking number of owner/active authority changes on account objects

## [0.1.4] - 2017-08-31
### Added
 - New operation: update global frequency
 - Pagination in get reward queue
 - Limit orders can be made with reserved balance
 - Limit orders from reserve can credit a tethered vault account
 - License types track EUR transfer limits
 - Last DSC:WEBEUR price is tracked in dynamic global properties
 - Initial/default DSC:WEBEUR can be set as a constant

### Updated
 - Fixed not producing blocks on upgrade event
 - Fix get_ticker crash when no trades are present
 - Fix transfers of only reserve balances
 - Daily limits on transfer vault to wallet use last dascoin price
 - WEBEUR is issued directly (no pending requests)
 - Canceled limit orders can return balance to reserve
 - Fixed constraints on transfers
 - General code cleanup, removing unused features

## [0.1.3] - 2017-06-02
### Added
- Change public keys operation
- Cli wallet can sign transactions with a set of keys in WIF format

### Updated
- Fixed object_database being created in wrong folder on initial run

## [0.1.2] - 2017-04-13
### Added
- Historic sum on queue
- Time on queue estimation on the blockchain (for queue ETA purposes)

## [0.1.1] - 2017-03-30
### Updated
- Hotfix: license names with underscores
- Fixed broken test harness

## [0.1.0] - 2017-03-30
### Added
- Production config
- Unpack script for deployment

## [0.0.7] - 2017-03-28
### Updated
- Fixed comment field in reward_queue_object not being reflected

## [0.0.6] - 2017-03-26
### Updated
- Changed dascoin_reward_amount to share_type
  - This fixes an overflow issue with reasonably large reward amounts
  - Updated field with the same name in update_queue_parameters_operation
- Changed hyphens in license names to underscores (eg. "pro_charter")
- Fixed some operations not impacting account history

## [0.0.5] - 2017-03-24
### Added
- Submit reserved cycle balances to queue now has a string "comment" field

### Updated
- Api calls for blances and queue with position now return structured objects
  - Each response has the "account_id" field
  - If the account is the response includes a "result" field
  - For more information on response layout see access_layer.hpp

## [0.0.4] - 2017-03-22
### Updated:
- Production values for licenses
- Database api fixed to handle get_queue_submissions_with_pos_for_accounts

## [0.0.3] - 2017-03-14
### Added:
- New api calls: 
  - get_free_cycle_balance
  - get_all_cycle_balances
  - get_dascoin_balance
  - get_free_cycle_balances_for_accounts
  - get_all_cycle_balances_for_accounts
  - get_dascoin_balances_for_accounts
  - get_queue_submissions_with_pos
  - get_queue_submissions_with_pos_for_accounts
- Vice-president licenseses for regular, chartered, pro types
- Null license for debug purposes 

### Updated:
- Fixed segfault bug in minting
- Additional minting information in operations


## [0.0.2] - 2017-03-06
- Bonus percentage of cycles when issuing a license
- Removed requests for issuing licenses
- Removed issuing cycles to balance
- Reserve Cycles are now issued to queue
- Added operation for changing queue parameters
- Reordered operations
- Fix preventing frequency lock being 0 when issuing a chartered or promo license
- Created scripts folder in project root

## [0.0.1] - 2017-02-14
### Added:
- Initial version of the blockchain