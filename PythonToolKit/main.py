from lexer import Lexer
from tokens import TokenType
from parser_ import Parser
from interpreter import Interpreter


text = 'call process(w f e)'
lexer = Lexer(text)
tokens = lexer.generate_tokens()


a = iter(tokens)
current_token = next(a)
while current_token != None :
	if current_token.type == TokenType.IDEN:

		print(str(current_token.value))
	try:
		current_token = next(a)
	except StopIteration:
		current_token = None

