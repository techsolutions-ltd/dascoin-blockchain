#### Operation
`accept_bond_offer_operation`

#### Parent Feature
[Bond Market](bond-market)

#### Description
Fill or partially fill a published open offer for a specified bond contract.

#### Justification
See parent feature.

#### Structure
The operation shall specify at least the following:
1. Source account
2. Bond contract offer that is to be filled
3. Payment asset amount

#### Self-Consistency Requirements
1. Payment asset amount SHALL be positive

#### World-Consistency Requirements
1. Source account SHALL be an existing account
2. Bond contract offer that is to be filled SHALL be an existing bond contract offer
3. Payment asset amount type SHALL be an existing asset type
4. Source account SHALL have at least the payment asset amount available
5. IF the bond contract offer was an offer to borrow, THEN the payment asset amount SHALL be no more than its principle asset amount
6. IF the bond contract offer was an offer to lend, THEN the payment asset amount SHALL be no more than its collateral asset amount

#### Output
1. An active bond contract SHALL be initialized as specified by the payment asset amount provided by the source account in accordance with the rules of the bond contract offer being filled
2. IF the bond contract offer was an offer to borrow AND the payment asset amount was equal to the full principle asset amount, THEN the bond contract offer shall be closed
3. IF the bond contract offer was an offer to lend AND the payment asset amount was equal to the full collateral asset amount, THEN the bond contract offer shall be closed
4. The source account SHALL be debited the payment asset amount

#### Asset Flow Diagram

#### Owners
- Author: Vikram (has not approved)
- Reviewer: Ben (has not approved)