# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

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