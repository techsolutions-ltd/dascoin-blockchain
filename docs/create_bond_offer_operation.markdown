#### Operation
`create_bond_offer_operation`

#### Parent Feature
[Bond Market](bond-market)

#### Description
Publish an open offer for a specified bond contract, with the source [account](Accounts) picking a specific side (borrowing or lending) of the contract. Publishing offers is required so that users can compare and contrast all available offers in the market. If a user finds an offer that they want to take, they can accept the offer and lock their account into the other side of the contract.

#### Justification
See parent feature.

#### Structure
The operation shall specify at least the following:
1. Source account
2. Side of contract the source account will take
3. Principle asset amount being loaned
4. Collateral asset amount that will secure the loan
5. Minimum time before which the loan cannot be repaid
6. Minimum time before which the collateral cannot be forfeit
7. Interest rate

#### Self-Consistency Requirements
1. Source account SHALL be either borrowing or lending
2. Principle asset amount SHALL be positive
3. Collateral asset amount SHALL be positive
4. Principle asset type SHALL be distinct from collateral asset type
5. Minimum time before which the loan cannot be repaid SHALL be positive
6. Minimum time before which the collateral cannot be forfeit SHALL be no earlier than the minimum time before which the loan cannot be repaid
7. Interest rate SHALL be an element of the interval [0, 100]%

#### World-Consistency Requirements
1. Source account SHALL be an existing account
2. Principle asset type SHALL be an existing asset type
3. Collateral asset type SHALL be an existing asset type
4. IF the source account is borrowing, THEN it SHALL have at least the collateral asset amount available
5. IF the source account is lending, THEN it SHALL have at least the principle asset amount available

#### Output
1. An open offer for the specified bond contract SHALL be published
2. IF the source account is borrowing, THEN it SHALL be debited the collateral asset amount
3. IF the source account is lending, THEN it SHALL be debited the principle asset amount

#### Asset Flow Diagram

#### Owners
- Author: Vikram (has not approved)
- Reviewer: Ben (has not approved)