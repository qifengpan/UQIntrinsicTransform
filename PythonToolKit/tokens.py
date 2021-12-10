from enum import Enum
from dataclasses import dataclass

class TokenType(Enum):
	NUMBER    = 0
	PLUS      = 1
	MINUS     = 2
	MULTIPLY  = 3
	DIVIDE    = 4
	LPAREN    = 5
	RPAREN    = 6
	IDEN      = 7
	ASSIG     = 8
	HOLD      = 9
	POINTER   = 10
	#POINTER_RE= 11
	logicalOp = 11
	STRING    = 12

class DerivedType(Enum):
	FUNCTION   = 0
	SUBROUTINE = 1
	VARIABLE   = 2
	OPERATOR   = 3
	INDICES    = 4
	NORMAL     = 5


@dataclass
class Token:
	type: TokenType
	Derive: DerivedType
	value: any = None

	def __repr__(self):
		return self.type.name + (f":{self.value}" if self.value != None else "")

	def update_Derive(self,inputDerive):
		self.Derive = inputDerive
