from tokens import TokenType,DerivedType,TokenType
from nodes import *

class Parser:
	def __init__(self, tokens):
		self.tokens = iter(tokens)
		self.advance()

	def raise_error(self):
		raise Exception("Invalid syntax")
	
	def advance(self):
		try:
			self.current_token = next(self.tokens)
		except StopIteration:
			self.current_token = None

	def parse(self):
		if self.current_token == None:
			return None

		result = self.expr()

		if self.current_token != None:
			self.raise_error()

		return result

	def expr(self):
		result = self.term()

		while self.current_token != None and self.current_token.type in (TokenType.PLUS, TokenType.MINUS):
			if self.current_token.type == TokenType.PLUS:
				self.advance()
				result = AddNode(result, self.term())
			elif self.current_token.type == TokenType.MINUS:
				self.advance()
				result = SubtractNode(result, self.term())

		return result

	def term(self):
		result = self.factor()

		while self.current_token != None and self.current_token.type in (TokenType.MULTIPLY, TokenType.DIVIDE):
			if self.current_token.type == TokenType.MULTIPLY:
				self.advance()
				result = MultiplyNode(result, self.factor())
			elif self.current_token.type == TokenType.DIVIDE:
				self.advance()
				result = DivideNode(result, self.factor())
				
		return result

	def factor(self):
		token = self.current_token

		if token.type == TokenType.LPAREN:
			self.advance()
			result = self.expr()

			if self.current_token.type != TokenType.RPAREN:
				self.raise_error()
			
			self.advance()
			return result

		elif token.type == TokenType.NUMBER:
			self.advance()
			return NumberNode(token.value)

		elif token.type == TokenType.PLUS:
			self.advance()
			return PlusNode(self.factor())
		
		elif token.type == TokenType.MINUS:
			self.advance()
			return MinusNode(self.factor())
		
		self.raise_error()


class myParser:
	def __init__(self, tokens):
		self.tokens = iter(tokens)
		#self.advance()
	def advance(self):
		try:
			self.current_token = next(self.tokens)
		except StopIteration:
			self.current_token = None

	def parse_tokens(self,variable):
		self.advance()
		while(self.current_token !=None):
			if self.current_token.type == TokenType.ASSIG:
				assigment = 1
			if(self.current_token.type == TokenType.IDEN):
				variable.append(str(self.current_token.value))
			if(self.current_token.value == "call"):
				self.current_token.update_Derive(DerivedType.SUBROUTINE)
				self.advance()
				self.current_token.update_Derive(DerivedType.SUBROUTINE)
				self.advance()
				self.advance()
				while(self.current_token != None and self.current_token.type != TokenType.RPAREN):
					if self.current_token.type == TokenType.IDEN:
						self.current_token.update_Derive(DerivedType.VARIABLE)
						self.advance()
					if (self.current_token.type == TokenType.LPAREN):
						#start reading inputs for the subroutine
						self.update_indicies()
						self.advance()
					else:
						self.advance()
				return variable
			else:
				return variable

	def update_indicies(self):
		LPnum = 1
		while (LPnum > 0):
			self.advance()
			if self.current_token.type == TokenType.RPAREN:
				LPnum = LPnum - 1
			if self.current_token.type == TokenType.LPAREN:
				LPnum = LPnum + 1
			if self.current_token.type == TokenType.IDEN:
				self.current_token.update_Derive(DerivedType.INDICES)