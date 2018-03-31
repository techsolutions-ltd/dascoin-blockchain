**work-in-progress**

This is a checklist of **software** requirements for BitShares 2.0.0.

# Alpha
- All items on roadmap before Phase 5: Alpha Testing must be complete
- All Github issues in alpha milestone must be closed
- All tests must be passing
- Graphene Core
    - High level features
        - SmartCoins
        - UIAs
        - Prediction markets
        - Vesting
        - Recurring payments
        - Proposed transactions
        - Workers
        - Delegates
        - Budget items
        - Referral system
        - Dynamic account permissions
        - Transferrable names
    - Freeze all operations
    - Freeze all objects
    - Peer review entire libraries/chain folder
        - 2 or more core devs must sign off on every line
    - Peer review entire test folder
- Graphene CLI Wallet
- Graphene Web App
  * Full onboarding experience
  * Multiple account support
  * BitShares 1 key import and account migration
  * Secure backup scheme
  * Full transfer capability
  * User-issued asset creation and maintainence
  * Full exchange capability
    - Buy, sell, short, cover
  * Gateway integration
  * Referral integration
    - Tracking scoreboards
  * Acceptance test
  * Usability/QA testing

Special care
    memos
    skip flags
    fee schedule
        data fees
        rate limiting
    global properties
    witness scheduling
    random number generator
    supply checking
    transaction malleability
    black swans
    referral system
        recursive referrals
    recursive authorities
    proposed transactions
        expiration
        invalidation
    prices
        overflows / underflows
    uia permissions
    all config constants

remove all compiler warnings gcc and clang
address all TODOs
merge all unused branches


# Beta
- All Alpha requirements
- All items on roadmap before Phase 6: Beta Testing must be complete
- All Github issues in beta milestone must be closed

# DevShares 2.0
- All Beta requirements
- All items on roadmap before Phase 7: DevShares 2.0 must be complete
- All Github issues in devshares milestone must be closed
