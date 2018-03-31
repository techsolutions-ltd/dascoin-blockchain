#### Operation
`cancel_bond_offer_operation`

#### Parent Feature
[Bond Market](bond-market)

#### Description
Retract a published open offer for a specified bond contract.

#### Justification
Allow a user to change their mind after publishing an offer for a bond contract.

#### Structure
The operation shall specify at least the following:
1. Source account
2. Bond contract offer that is to be retracted
3. Refund asset amount

#### Self-Consistency Requirements
1. Refund asset amount SHALL be positive

#### World-Consistency Requirements
1. Source account SHALL be an existing account
2. Bond contract offer that is to be retracted SHALL be an existing bond contract offer
3. Refund asset amount type SHALL be an existing asset type
4. Source account SHALL be the creator of the bond contract offer that is to be retracted
5. IF the source account was offering to borrow, THEN the refund asset amount SHALL be equal to the collateral asset amount in the bond contract offer
6. IF the source account was offering to lend, THEN the refund asset amount SHALL be equal to the principle asset amount in the bond contract offer

#### Output
1. The specified bond contract offer SHALL be unpublished
2. The source account SHALL be credited the refund asset amount

#### Asset Flow Diagram

#### Owners
- Author: Vikram (has not approved)
- Reviewer: Ben (has not approved)