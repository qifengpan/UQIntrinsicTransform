from tokens import Token, TokenType,DerivedType

WHITESPACE = ' \n\t'
DIGITS = '0123456789'
IDENS='abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_%'
CONVERT = '"\''
COMPARE = '></'

class Lexer:
	def __init__(self, text):
		self.text = iter(text)
	#	self.text = text
	#	self.pos  = 0
	#	self.current_char = self.text[self.pos]
	#	self.next_char = self.text[self.pos + 1]
		self.advance()

	def advance(self):
		try:
			self.current_char = next(self.text)
		except StopIteration:
			self.current_char = None

	'''
    def advance(self):
		if self.pos < len(self.text)-1:
			self.current_char = self.text[self.pos]
			self.next_char = self.text[self.pos+1]
		elif self.pos == len(self.text)-1:
			self.current_char = self.text[self.pos]
			self.next_char = None
		else:
			self.current_char = None
			self.next_char = None
		self.pos = self.pos + 1
	'''


	def generate_tokens(self):
		while self.current_char != None:
			if self.current_char == "!":
				while(self.current_char != None):
					self.advance()
				break
			if self.current_char in WHITESPACE:
				self.advance()
			elif self.current_char in DIGITS:
				yield self.generate_number()
			elif self.current_char == '.':
				yield self.generate_tokens_start_by_point()
			elif self.current_char in IDENS:
				yield self.generate_idens()
			elif self.current_char == '+':
				self.advance()
				yield Token(TokenType.PLUS,DerivedType.NORMAL)
			elif self.current_char == '-':
				self.advance()
				yield Token(TokenType.MINUS,DerivedType.NORMAL)
			elif self.current_char == '*':
				self.advance()
				yield Token(TokenType.MULTIPLY,DerivedType.NORMAL)
			elif self.current_char == '/':
				self.advance()
				yield Token(TokenType.DIVIDE,DerivedType.NORMAL)
			elif self.current_char == '(':
				self.advance()
				yield Token(TokenType.LPAREN,DerivedType.NORMAL)
			elif self.current_char == ')':
				self.advance()
				yield Token(TokenType.RPAREN,DerivedType.NORMAL)
			elif self.current_char == '=':
				yield self.generate_tokens_start_by_equal()
			elif self.current_char == '&':
				self.advance()
				yield Token(TokenType.HOLD,DerivedType.NORMAL)
			#elif self.current_char == '%':
			#	self.advance()
			#	yield Token(TokenType.POINTER_RE,DerivedType.NORMAL)
			elif (self.current_char in CONVERT):
				self.advance()
				yield self.generate_string()
			elif(self.current_char in COMPARE):
				yield self.generate_compare()
			else:
				#print(self.current_char)
				self.advance()



	def generate_number(self):
		decimal_point_count = 0
		number_str = self.current_char
		self.advance()

		while self.current_char != None and (self.current_char == '.' or self.current_char in DIGITS):
			if self.current_char == '.':
				decimal_point_count += 1
				if decimal_point_count > 1:
					break
			
			number_str += self.current_char
			self.advance()

		if number_str.startswith('.'):
			number_str = '0' + number_str
		if number_str.endswith('.'):
			number_str += '0'

		return Token(TokenType.NUMBER,DerivedType.NORMAL ,float(number_str))

	def generate_idens(self):
		iden_str = self.current_char
		self.advance()
		while self.current_char != None and (self.current_char in IDENS or self.current_char in DIGITS):

			iden_str += self.current_char
			self.advance()
		return Token(TokenType.IDEN , DerivedType.NORMAL, iden_str)

	def generate_tokens_start_by_point(self):
		token_str = self.current_char
		self.advance()
		if(self.current_char in DIGITS ):
			self.generate_number()
		elif(self.current_char in IDENS):
			self.generate_logical_tokens()

	def generate_logical_tokens(self):
		logical_str = "."
		while self.current_char != None and self.current_char != '.':
			logical_str += self.current_char
			self.advance()
		logical_str += self.current_char
		self.advance()
		return Token(TokenType.logicalOp, DerivedType.OPERATOR,logical_str)

	def generate_tokens_start_by_equal(self):
		self.advance()
		if(self.current_char == "="):
			self.advance()
			return Token(TokenType.logicalOp,"==")
		elif(self.current_char == ">"):
			self.advance()
			return Token(TokenType.POINTER,"=>")
		elif(self.current_char in IDENS):
			return Token(TokenType.ASSIG,"=")
		else:
			return Token(TokenType.ASSIG,"=")

	def generate_string(self):
		str_cont = ''
		while (self.current_char not in CONVERT):
			str_cont += self.current_char
			self.advance()
		self.advance()
		return Token(TokenType.STRING,DerivedType.NORMAL,str_cont)

	def generate_compare(self):
		logical_str = self.current_char
		self.advance()
		if(self.current_char == "="):
			self.advance()
			return Token(TokenType.logicalOp,logical_str + "=")
		else:
			return  Token(TokenType.logicalOp,DerivedType.OPERATOR,logical_str)