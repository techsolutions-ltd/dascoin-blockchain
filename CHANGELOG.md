# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).  

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
