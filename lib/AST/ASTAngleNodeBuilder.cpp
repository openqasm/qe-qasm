/* -*- coding: utf-8 -*-
 *
 * Copyright 2022 IBM RESEARCH. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * =============================================================================
 */

#include <qasm/AST/ASTTypeEnums.h>
#include <qasm/AST/ASTAngleNodeBuilder.h>
#include <qasm/AST/ASTAngleContextControl.h>
#include <qasm/AST/ASTTypeSystemBuilder.h>
#include <qasm/AST/ASTSymbolTable.h>
#include <qasm/AST/ASTBuilder.h>
#include <qasm/AST/ASTMangler.h>

#include <string>
#include <map>
#include <cassert>

namespace QASM {

std::map<std::string, ASTAngleType> ASTAngleNode::ATM = {
  { "alpha", ASTAngleTypeAlpha },
  { "beta", ASTAngleTypeBeta },
  { "gamma", ASTAngleTypeGamma },
  { "delta", ASTAngleTypeDelta },
  { "epsilon", ASTAngleTypeEpsilon },
  { "zeta", ASTAngleTypeZeta },
  { "eta", ASTAngleTypeEta },
  { "theta", ASTAngleTypeTheta },
  { "iota", ASTAngleTypeIota },
  { "kappa", ASTAngleTypeKappa },
  { "lambda", ASTAngleTypeLambda },
  { "mu", ASTAngleTypeMu },
  { "nu", ASTAngleTypeNu },
  { "xi", ASTAngleTypeXi },
  { "omicron", ASTAngleTypeOmicron },
  { "pi", ASTAngleTypePi },
  { "rho", ASTAngleTypeRho },
  { "sigma", ASTAngleTypeSigma },
  { "tau", ASTAngleTypeTau },
  { "upsilon", ASTAngleTypeUpsilon },
  { "phi", ASTAngleTypePhi },
  { "chi", ASTAngleTypeChi },
  { "psi", ASTAngleTypePsi },
  { "omega", ASTAngleTypeOmega },
  { "euler", ASTAngleTypeEuler },
  { "euler_number", ASTAngleTypeEuler },
  { u8"α", ASTAngleTypeAlpha },
  { u8"β", ASTAngleTypeBeta },
  { u8"γ", ASTAngleTypeGamma },
  { u8"δ", ASTAngleTypeDelta },
  { u8"ε", ASTAngleTypeEuler },
  { u8"ζ", ASTAngleTypeZeta },
  { u8"η", ASTAngleTypeEta },
  { u8"θ", ASTAngleTypeTheta },
  { u8"ι", ASTAngleTypeIota },
  { u8"κ", ASTAngleTypeKappa },
  { u8"λ", ASTAngleTypeLambda },
  { u8"μ", ASTAngleTypeMu },
  { u8"ν", ASTAngleTypeNu },
  { u8"ξ", ASTAngleTypeXi },
  { u8"ο", ASTAngleTypeOmicron },
  { u8"π", ASTAngleTypePi },
  { u8"ρ", ASTAngleTypeRho },
  { u8"σ", ASTAngleTypeSigma },
  { u8"τ", ASTAngleTypeTau },
  { u8"υ", ASTAngleTypeUpsilon },
  { u8"φ", ASTAngleTypePhi },
  { u8"χ", ASTAngleTypeChi },
  { u8"ψ", ASTAngleTypePsi },
  { u8"ω", ASTAngleTypeOmega },
  { u8"Α", ASTAngleTypeAlpha },
  { u8"Β", ASTAngleTypeBeta },
  { u8"Γ", ASTAngleTypeGamma },
  { u8"Δ", ASTAngleTypeDelta },
  { u8"Ε", ASTAngleTypeEpsilon },
  { u8"Ζ", ASTAngleTypeZeta },
  { u8"Η", ASTAngleTypeEta },
  { u8"Θ", ASTAngleTypeTheta },
  { u8"Ι", ASTAngleTypeIota },
  { u8"Κ", ASTAngleTypeKappa },
  { u8"Λ", ASTAngleTypeLambda },
  { u8"Μ", ASTAngleTypeMu },
  { u8"Ν", ASTAngleTypeNu },
  { u8"Ξ", ASTAngleTypeXi },
  { u8"Ο", ASTAngleTypeOmicron },
  { u8"Π", ASTAngleTypePi },
  { u8"Ρ", ASTAngleTypeRho },
  { u8"Σ", ASTAngleTypeSigma },
  { u8"Τ", ASTAngleTypeTau },
  { u8"Υ", ASTAngleTypeUpsilon },
  { u8"Φ", ASTAngleTypePhi },
  { u8"Χ", ASTAngleTypeChi },
  { u8"Ψ", ASTAngleTypePsi },
  { u8"Ω", ASTAngleTypeOmega },
};

ASTAngleNodeList    ASTAngleNodeBuilder::AL;
ASTAngleNodeMap     ASTAngleNodeBuilder::AM;
ASTAngleNodeBuilder ASTAngleNodeBuilder::B;
ASTAngleNodeMap     ASTAngleNodeMap::AM;

ASTAngleContextControl ASTAngleContextControl::ACC;
bool ASTAngleContextControl::CCS = false;

void
ASTAngleNode::EraseFromLocalSymbolTable() {
  const std::string& AN = this->GetName();

  // Reserved or globla scope angles cannot be deleted from
  // the Symbol Table.
  if (ASTTypeSystemBuilder::Instance().IsReservedAngle(AN) ||
      GetIdentifier()->IsGlobalScope())
    return;

  for (unsigned J = 0; J < 3; ++J) {
    const ASTIdentifierRefNode* AIdR = IIR[J];

    if (AIdR)
      ASTSymbolTable::Instance().EraseLocalSymbol(AIdR->GetName(), ASTIntNode::IntBits,
                                                  ASTTypeAngle);
  }

  ASTSymbolTable::Instance().EraseLocalSymbol(GetIdentifier(), GetBits(),
                                              ASTTypeAngle);
}

} // namespace QASM

