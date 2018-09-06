# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [0.4.5] - 2018-09-06
### Added
 - Hardfork: use correct Dascoin price in clearing contract
 - New query: get amount of project tokens received for asset
 - New query: get amount of asset needed for project token
 - Reserved first 20 places in asset_id space for system assets

### Updated
 - Cli Wallet: performance of get_account_history_by_operation query is
   vastly improved
 - Allow wireout of BTC asset

## [0.4.4] - 2018-08-31
### Added
 - Hardfork: key rollback removed
 - BTC asset created
 - New operation: update external BTC price
 - New operation: use external BTC price or price from internal exchange
 - New operation: distribute das33 project pledges
 - New operation: reject das33 project
 - New operation: distribute single das33 pledge
 - New operation: reject single das33 pledge
 - New query: get tethered accounts balances
 - New query: get amount of assets pledged to a project
 - Support for fees in other assets
 - Cli Wallet: new method get account history by operation
 - Cli Wallet: new method update external btc price

### Updated
 - Flags of DAS and WEBEUR assets are fixed
 - Issuer of DAS asset is now webasset-issuer
 - Transfer operation now obeys asset restrictions
 - Restriction that last DAS cannot be spent is removed
 - Changed internal das33 mechanics

## [0.4.3] - 2018-08-03
### Added
 - New query: get limit orders collection grouped by price

### Updated
 - Updated upgrade behavior for utility licenses
 - Fixed bug regarding websocket server not accepting connections after
   firewall rules are changed.

## [0.4.2] - 2018-07-11
### Added
 - New operation: update global parameters
 - New query: get delayed operation for account
 - Hardfork: new algorithm to calculate price in debit and credit operations

### Updated
 - Allow 0 amount in debit operation
 - Allow purchasing cycles from all accoind kinds
 - Cherry picked fixes for BitShares issue #411

## [0.4.1] - 2018-06-27
### Added

### Updated
 - Changed utility licences number of upgrades
 - Changed number of cycles for Vice President Utility Licence

## [0.4.0] - 2018-06-27
### Added
 - New operation: set daspay transaction ratio
 - New operation: register daspay authority
 - New operation: unregister daspay authority
 - New operation: reserve asset on account
 - New operation: unreserve asset on account
 - New operation: create payment service provider
 - New operation: update payment service provider
 - New operation: delete payment service provider
 - New operation: daspay debit account
 - New operation: daspay credit account
 - New operation: update daspay clearing parameters
 - New operation: update delayed operations resolver parameters
 - New operation: das33 pledge asset
 - New operation: das33 project create
 - New operation: das33 project update
 - New operation: das33 project delete
 - New operation: set chain authority
 - Clearing smart contract
 - Delayed operations resolver smart contract
 - New license kind: utility
 - New authorities: daspay_administrator and das33_administrator
 - New query: get total cycles
 - New query: get daspay authority for account
 - New query: get payment service providers
 - New query: get das33 projects
 - New query: get das33 pledges
 - New query: get das33 pledges by account
 - New query: get das33 pledges by project
 - Cli Wallet: new method set daspay transaction ratio
 - Cli Wallet: new methods to create, update and delete payment service provide
 - Cli Wallet: new methods to register and unregister daspay authority
 - Cli Wallet: new methods to reserve and unreserve asset on account
 - Cli Wallet: new method daspay debit
 - Cli Wallet: new method daspay credit
 - Cli Wallet: new method update clearing parameters
 - Cli Wallet: new method update delayed operations resolver parameters
 - Cli Wallet: new method get daspay authority for account
 - Cli Wallet: new method das33 pledge asset
 - Cli Wallet: new method get das33 pledges
 - Cli Wallet: new method get das33 pledges by account
 - Cli Wallet: new method get das33 pledges by project
 - Cli Wallet: new methods to create, update and delete project
 - Cli Wallet: new method get das33 projects

### Updated
 - License upgrade system updated to handle utility licences

## [0.3.2] - 2018-05-31
### Added
 - New operations: roll back public key, set enable rollback
 - New operation: submit cycles from pool account
 - Hardfork: removed operations which were erroneously sent to live net

## [0.3.1] - 2018-04-24
### Updated
 - Hardfork: fixed timestamp

## [0.3.0] - 2018-04-24
### Added
 - New fee implementation
 - Hardfork: users cannot spend the last dascoin but only on cycle purchase
 - New operation: purchase cycles
 - New operation: wire out with fee
 - New operation: transfer cycles from license to wallet
 - New query: calculate cycle price
 - Add 100 cycles to all wallets to be used as fees
 - New account type: custodian
 - Cli Wallet: new method get account history by operation
 - Cli Wallet: new method to read and sign memo key
 - Cli Wallet: new method transfer vault to wallet
 - Cli Wallet: new method tether accounts
 - Cli Wallet: net method create account

### Updated
 - Allow transfers between wallets

## [0.2.9] - 2018-03-30
### Added
 - New operation: remove vault limits
 - Hardfork: update euro limit operation is deprecated. No limit on vault to
   wallet transfer

## [0.2.8] - 2018-01-14
### Added
 - Upgrades are now done internally, on chain
 - Support for multiple master nodes is reworked to better suit our needs
 - New operation: issue cycles to a license
 - New operation: update license

### Updated
 - Merged changes from BitShares related to block database. Indexing should
   be more robust now
 - Charter licenses didn't have proper multipliers vector set. This is now
   fixed.

## [0.2.7] - 2017-12-21
### Added
 - Query to get trade history by sequence

### Updated
 - Fill order now keeps track of match price (should prevent 0 price seen on
   the chart, this was cherry picked from bitshares PR-455)
 - get_24_hi_low_volume will now use all trades made in the same block,
   not only the first 100

## [0.2.6] - 2017-11-22
### Added
 - Added 'history-trade-ticks-size' (max tracked trade ticks) to market-history-plugin config

### Updated
 - Renamed 'history-per-size' to 'history-per-bucket-size'

## [0.2.5] - 2017-11-16
### Added
 - Get license types grouped by kind to database api
 - Hardfork: update queue parameters now uses the correct number decimal
 - Get blocks to database api

## [0.2.4] - 2017-11-02
### Added
 - Wire out result virtual operation added to account history
 - New license kind: locked licenses keep frequency lock but have manual submit
 - New operation: submit cycles to queue by license
 - New constraint: vaults cannot have mixed license kinds

### Updated
 - Tracking wire out complete and reject operations in account history
 - Updated operation: submit cycles to queue uses comment to submit from license
 - Reflected base_amount on license history object

## [0.2.3] - 2017-10-11
### Updated
 - Query to get limit orders grouped by price, now grouped by price with precision of two decimal places

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
