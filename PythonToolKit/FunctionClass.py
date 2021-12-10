from lexer import Lexer
from tokens import TokenType
#from FunctionClass_test import readOneLine,advance

def readOneLine(CodeContent,pos):
    strLine = ''
    while(CodeContent[pos] != '\n'):
        strLine +=CodeContent[pos]
        pos = pos+1
    pos = pos + 1
    return strLine,pos

def advance(item):
    try:
        current_item = next(item)
    except StopIteration:
        current_item = None
    return current_item

class functionType():
    class_counter = 0
    def __init__(self,name,input):
        self.name = name
        self.text = ''
        self.UQtoken = 0
        self.variable = []
        self.input = input
        self.output = []
        self.ID = functionType.class_counter
        self.isChecked = False
        self.UQ        = []
        self.NumUQ_in_input = 0
        self.UQ_content = ""
        self.UQ_line_pos = []
        functionType.class_counter += 1
    def updatename(self,name):
        self.name = name

    def updatetext(self,text):
        self.text += text

    def updateUQtoken(self):
        self.UQtoken = 1

    def updateinput(self,input):
        self.input = input

    def updateNumUQ_in_input(self,num):
        self.NumUQ_in_input = num

    def parseVariables(self):
        pos = 0
        keyword = ["integer","real","double","dimension","intent","in","out","inout","precision"]
        while (pos < len(self.text)):
            [strLine, pos] = readOneLine(self.text, pos)
            lexer  = Lexer(strLine)
            self.tokens = lexer.generate_tokens()
            self.tokens = iter(self.tokens)
            current_token = advance(self.tokens)
            while(current_token != None):
                if(current_token.value in keyword):
                    while (current_token != None):
                        if (current_token.type == TokenType.IDEN and current_token.value not in keyword):
                            if(current_token.value not in self.variable):
                                self.variable.append(current_token.value)
                            current_token = advance(self.tokens)
                        else:
                            current_token = advance(self.tokens)
                else:
                    current_token = advance(self.tokens)

    def readVariable(self,keyword):
        current_token = advance(self.tokens)
        while(current_token != None):
            if(current_token.type == TokenType.IDEN and current_token.value not in keyword):
                self.variable.append(current_token.value)
                current_token = advance(self.tokens)
            else:
                current_token = advance(self.tokens)

    def updateUQ_in_input(self):
        NumUQcount = 0
        for variables in self.input:
            if(variables in self.UQ):
                NumUQcount += 1
        if(NumUQcount > self.NumUQ_in_input):
            output = True
        else:
            output = False
        self.updateNumUQ_in_input(NumUQcount)
        return output

    def UQ_in_input(self):
        # return the and set of UQ and input!
        output = list(set(self.UQ).intersection(set(self.input)))
        return output

class moduleType():
    Module_counter = 0
    def __init__(self,name):
        self.name = name
        self.text = ''
        self.variable = []
        self.listFunc = []
        self.ID = moduleType.Module_counter
        moduleType.Module_counter += 1

    def read_fun(self):
        while(self.text):
            Linestr  = self.eatOneLine(self.text)
            self.listFunc.append(functionType())

    def eatOneLine(self):
        pos = 0
        [strLine, pos] = readOneLine(self.text, pos)
        self.text = self.text[pos:]
        strLine = strLine.strip()
        return strLine



