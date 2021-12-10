#! /usr/bin/python
import os
from lexer import Lexer
from parser_ import myParser
from tokens import TokenType,DerivedType
from ProfileReader import gprofreading
import sys

def preprocess_call(temp):
    signs=['(',')']
    if temp[0:4] == 'call':
        mode = 'start'
    else:
        mode = 'middle'
    pos = 0
    LPcount = 0
    RPcount = 0
    while(pos<len(temp)):
        if mode == 'start':
            if temp[pos] not in signs or temp[pos] == ')':
                pos = pos + 1
            elif LPcount == 0 and temp[pos] == '(':
                LPcount = LPcount + 1
                pos = pos + 1
            elif temp[pos] == '(' and LPcount > 0:
                LPcount = LPcount + 1
                temp_pos =  pos
                pos = pos + 1
                while ((LPcount-RPcount) > 1):
                    if temp[pos] == '(':
                        LPcount = LPcount + 1
                    if temp[pos] == ')':
                        RPcount = RPcount + 1
                    pos = pos + 1
                temp = temp[0:temp_pos]+temp[pos:]
                pos = temp_pos+1
                
        else:
            if temp[pos] not in signs or temp[pos] == ')':
                pos = pos + 1
            elif  temp[pos] == '(':
                LPcount = LPcount + 1
                temp_pos =  pos
                while ((LPcount-RPcount) > 0):
                    if temp[pos] == '(':
                        LPcount = LPcount + 1
                    if temp[pos] == ')':
                        RPcount = RPcount + 1
                    pos = pos + 1
                temp = temp[0:temp_pos]+temp[pos:]
                pos  = temp_pos+1
    return temp

def readstatement(temps,funclist):
    # input: 'temps' should be a string
    # output: variables list and judge if there is needs to wait the following lines
    keyword = 'call'
    assigment = 0
    call_flag = 0
    call_function = 0
    if temps[0:4] == keyword:
 #       print('preprocess is called')
        temps = preprocess_call(temps)

    lexer = Lexer(temps)
    tokens = lexer.generate_tokens()
    #myparser = myParser(tokens)
    variable = []
    #variable = myparser.parse_tokens(variable)
    Temp_tokens = iter(tokens)

    current_token = advance(Temp_tokens)

    while current_token != None:
        if current_token.type == TokenType.ASSIG:
            assigment = 1
        if current_token.type == TokenType.IDEN :
            if current_token.value in funclist.keys() and assigment == 1:
                variable.append(str(current_token.value))
                variable_temp = readfunContent(Temp_tokens,variable)
                call_function = 1
                variable.append(variable_temp)
                return variable, assigment, call_flag,call_function
            else:
                variable.append(str(current_token.value))

        # logic for hold flag should be implement in detail
        if current_token.value == keyword:
            call_flag = 1
        current_token = advance(Temp_tokens)
    return variable, assigment, call_flag,call_function


def readfunContent(tokens_inFun,variable):
    current_token = advance(tokens_inFun)
    current_token = advance(tokens_inFun)
    while (current_token != None):
        if (current_token.type == TokenType.IDEN):
            variable.append(str(current_token.value))
            current_token = advance(tokens_inFun)
        elif (current_token.type == TokenType.LPAREN):
                # start reading inputs for the subroutine
            skip_indicies(tokens_inFun)
            current_token = advance(tokens_inFun)
        else:
            current_token = advance(tokens_inFun)
    return variable

def  skip_indicies(tokens_inParent):
    LPnum = 1
    while (LPnum > 0):
        try:
            current_token = next(tokens_inParent)
        except StopIteration:
            current_token = None
        if current_token.type == TokenType.RPAREN:
            LPnum = LPnum - 1
        elif current_token.type == TokenType.LPAREN:
            LPnum = LPnum + 1
        elif current_token.type == TokenType.IDEN:
            current_token.update_Derive(DerivedType.INDICES)

def advance(token):
    try:
        current_token = next(token)
    except StopIteration:
        current_token = None
    return current_token

def readfun(filename):
    # input: filename of the fortran code
    # output: return a list with contain the name of functions and their inputs

    rf = open(filename, "r",encoding='latin-1')
    operator = ['(', ',', '&', ')']
    functionlist = {}
    filelist = {filename: []}
    s = " "
    buffer = ""
    hold_flag = 0

    while (s):
        s = rf.readline()
        temp_s = s.lstrip()
        # detect defining the subroutine
        if hold_flag == 0 and temp_s != '':

            if temp_s[0:10] == "subroutine":
                temp_s = temp_s[11:]
                while (temp_s):
                    if temp_s[0] not in operator and temp_s[0] != '\n':  # read indentifier
                        buffer = buffer + temp_s[0]
                        temp_s = temp_s[1:]
                    elif temp_s[0] == "(":  # ( is sign for function name
                        functionlist[buffer] = []
                        funcname = buffer
                        filelist[filename].append(funcname)  # append the function name to the filename
                        buffer = ""
                        temp_s = temp_s[1:]
                    elif temp_s[0] == ',':  # , is sign for variables
                        functionlist[funcname].append(buffer.strip())
                        buffer = ""
                        temp_s = temp_s[1:]
                    elif temp_s[0] == '&':  # the statement is broken into serveral lines
                        hold_flag = 1
                        temp_s = []
                    elif temp_s[0] == '\n' and hold_flag != 1:
                        funcname = buffer.strip()
                        functionlist[buffer] = []
                        filelist[filename].append(funcname)
                        temp_s = []
                        buffer = ''
                    else:  # other case: the function ends
                        functionlist[funcname].append(buffer.strip())
                        temp_s = []
                        buffer = ''

            if temp_s[0:8] == "function":
                temp_s = temp_s[9:]
                while (temp_s):
                    if temp_s[0] not in operator and temp_s[0] != '\n':  # read indentifier
                        buffer = buffer + temp_s[0]
                        temp_s = temp_s[1:]
                    elif temp_s[0] == "(":  # ( is sign for function name
                        functionlist[buffer] = []
                        funcname = buffer
                        filelist[filename].append(funcname)  # append the function name to the filename
                        buffer = ""
                        temp_s = temp_s[1:]
                    elif temp_s[0] == ',':  # , is sign for variables
                        functionlist[funcname].append(buffer.strip())
                        buffer = ""
                        temp_s = temp_s[1:]
                    elif temp_s[0] == '&':  # the statement is broken into serveral lines
                        hold_flag = 1
                        temp_s = []
                    else:  # other case: the function ends
                        functionlist[funcname].append(buffer.strip())
                        temp_s = []
                        buffer = ''
            if temp_s[0:7] == "program":
                temp_s = temp_s[8:]
                while (temp_s):
                    if temp_s[0] not in operator and temp_s[0] != '\n':  # read indentifier
                        buffer = buffer + temp_s[0]
                        temp_s = temp_s[1:]
                    elif temp_s[0] == "\n":  # with enter the program name ends
                        functionlist[buffer] = []
                        temp_s = []
                        buffer = ''
        if hold_flag == 1:  # reading the contain of other lines
            while (temp_s):
                if temp_s[0] not in operator:  # read indentifier
                    buffer = buffer + temp_s[0]
                    temp_s = temp_s[1:]
                elif temp_s[0] == ',':  # , is sign for variables
                    functionlist[funcname].append(buffer.strip())
                    buffer = ""
                    temp_s = temp_s[1:]
                elif temp_s[0] == '&':  # the statement is broken into serveral lines
                    hold_flag = 1
                    temp_s = []
                elif temp_s[0] == ')':  # other case: the function ends
                    hold_flag = 0
                    functionlist[funcname].append(buffer.strip())
                    buffer = ''
                    temp_s = []
    return functionlist, filelist


def get_key(inputdict, name):
    output = []
    print(inputdict.items())
    for key, value in inputdict.items():
        for ivalue in value:
            if ivalue in name:
                output.append(key)
    return output

def readcall(s):
    LPconut=0   #number of left parentheses
    RPcount=0   #number of right parentheses
    signs = [',','(',')','&']
    end_flag = 1
    Buffer = []
    Variable = []
    mode = 'start'
    #scan the whole line to disdiguish which type it is
    for i in s:
        if i == '(':
            LPcount = LPcount+1
        if i == ')':
            RPcount = RPcount+1   
        if i == '&':
            end_flag = 0
    if (LPcount - RPcount) < 1:
        mode = 'middle'
    if mode == 'start':
        while(s):
            if s[0] not in signs:
                Buffer = Buffer + s[0]
                s = s[1:]
            elif LPcount == 0 and s[0] == '(':
                Variable = Variable.append(Buffer.strip())
                s = s[1:]
                Buffer.clear()
                LPcount = LPcount + 1
            elif s[0] == ',':
                Variable = Variable.append(Buffer.strip())
                s = s[1:]
                Buffer.clear()
            elif s[0] == '(' and LPcount > 0:
                Variable = Variable.append(Buffer.strip())
                s = s[1:]
                LPcount = LPcount + 1
                Buffer.clear()
                while ((LPcount-RPcount) > 1):
                    if s[0] == '(':
                        LPcount = LPcount + 1
                    if s[0] == ')':
                        RPcount = RPcount + 1
                    s = s[1:]
    else:
        while(s):
            if s[0] not in signs:
                Buffer = Buffer + s[0]
                s = s[1:]
            elif LPcount == 0 and s[0] == '(':
                Variable = Variable.append(Buffer.strip())
                s = s[1:]
                Buffer.clear()
                LPcount = LPcount + 1
            elif s[0] == ',':
                Variable = Variable.append(Buffer.strip())
                s = s[1:]
                Buffer.clear()
            elif s[0] == '(' and LPcount > -1:
                Variable = Variable.append(Buffer.strip())
                s = s[1:]
                LPcount = LPcount + 1
                Buffer.clear()
                while ((LPcount-RPcount) > 0):
                    if s[0] == '(':
                        LPcount = LPcount + 1
                    if s[0] == ')':
                        RPcount = RPcount + 1
                    s = s[1:]
    return Variable,end_flag

def finduq_in_singlefile(filepath, UQFun, UQ, funclist):
    rf = open(filepath, "r",encoding='latin-1')
    keyword = ['write', 'print','do','integer','if','real']
    s = rf.readline()
    variable = []
    count = 1
    call_function = 0
    end_flag = 0
    call_flag = 0
    assig = 0
    call_buffer = 0
    start_line = 0
    use_start_line = 0
    bindstr = ''
    while (s):
        temp_s = s.strip()
#        if temp_s[0:3] == 'call':
#            [variabe_temp,end_flag]=readcall(temp_s)
        if temp_s != '' and temp_s[0] != '!':
            #if end with &, read the rest using first branch
            if(temp_s.endswith("&")):
                bindstr = bindstr + temp_s
                s = rf.readline()
                temp_s = s.strip()
                start_line = count
                count = count + 1
                while(temp_s.endswith("&")):
                    bindstr = bindstr + temp_s
                    count = count + 1
                    s = rf.readline()
                    temp_s = s.strip()
                bindstr = bindstr + temp_s
                [variabe_temp, assig, call_flag,call_function] = readstatement(bindstr,funclist)
                variable = variabe_temp
                bindstr_output = bindstr
                bindstr=''
                use_start_line = 1

            else:
                [variabe_temp, assig, call_flag, call_function] = readstatement(temp_s,funclist)
                variable = variabe_temp

            # if it is calling a subroutine
            if (call_flag == 1 or call_function == 1):
                #                print(variable)
                pos = 0
                has_UQ = 0
                num_UQ = 0
                for i in variable:
                    if i in UQ:
                        has_UQ = 1
                        num_UQ += 1
                        if variable[1] not in UQFun.mydict.keys():
                            print(variable[1])
                            UQFun.update_dict(variable[1],pos-1)

                        if variable[1] in UQFun.mydict.keys() and (pos-1) not in UQFun.mydict.get(variable[1]):
                            UQFun.update_exist_dict(variable[1],pos-1)

                        if variable[1] =='MPI_Gather':
                            print('MPI_Sendrecv in update is: ',pos)
                            print('MPI_Sendrecv in update is: ',variable)

                    pos = pos + 1
                if has_UQ:
                    if (not use_start_line):
                        print('the ', count, 'th line contains', num_UQ , 'UQ variable by calling fun in', filepath)
                        print(s)
                    else:
                        print('the ', start_line, 'th line contains', num_UQ , 'UQ variable by calling fun in', filepath)
                        print(bindstr_output)
                variable.clear()
                variabe_temp.clear()
                    # update the name in subtoutine definition
            if (assig == 1 and call_function != 1):
                for i in variable:
                    if i in UQ and i not in keyword and variable[0] not in keyword:
                        if variable[0] not in UQ:
                            UQ.append(variable[0])
                            if (not use_start_line):
                                print('the ', count, 'th line containsUQ variable through assigment in', filepath)
                                print(s)
                            else:
                                print('the ', start_line, 'th line contains UQ variable through assigment in', filepath)
                                print(bindstr_output)
                variable.clear()
                variabe_temp.clear()
        count = count + 1
        start_line = 0
        use_start_line = 0
        s = rf.readline()

        #recheck if the input variables contains UQ
                
    return UQFun, UQ


class UQfunset():
    def __init__(self):
        #self.name = ''
        #self.pos = []
        self.mydict = {}

    #def addname(self, name):
    #    self.name = name

    #def addpos(self, pos):
    #    self.pos = self.pos + [pos]


    def update_dict(self,name,pos):
        local_UQ = {name: [pos]}
        if name is not '':
            self.mydict.update(local_UQ)
    def update_exist_dict(self,name,pos):
        self.mydict[name].append(pos)
        

if __name__ == "__main__":
    '''
    file = "/home/qifeng/qifeng_file/parallel-heat3d-f90-master/explicitPar.f90"
    UQ = ['k0']
    UQFun = []
    rf1 = open(file, "r")
    s = rf1.readline()
    variable_temp = []
    UQFun = UQfunset()
    path = "/home/qifeng/qifeng_file/parallel-heat3d-f90-master"
    files = os.listdir(path)
    funclist = {}
    filelist = {}
    for filename in files:
        if os.path.splitext(filename)[1] == ".f90":
            filename = path + '/' + filename
            [funlist_t, filelist_t] = readfun(filename)
            funclist.update(funlist_t)
            filelist.update(filelist_t)
    [UQFun, UQ] = finduq_in_singlefile(file, UQFun, UQ, funclist)
    print(UQ)
    print(UQFun.mydict)

    #    print(filelist)
    name = [key for key, value in UQFun.mydict.items()]
    file_path2 = get_key(filelist, name)
    rf2 = open(file_path2[0], "r")
    [UQFun, UQ] = finduq_in_singlefile(file_path2[0], UQFun, UQ, funclist)
    print(UQFun.mydict)
    print(UQ)
    '''
    path = "/home/qifeng/qifeng_file/parallel-heat3d-f90-master"
    #path = '/home/qifeng/non-Euler2D/euler2D-kfvs-Fortran2003-master'
    files = os.listdir(path)
    parents = {}
    children = {}
    times = {}
    stack = []
    tolist = []
    UQ = ['k0']
    UQFun = UQfunset()
    funclist = {}
    subrname = 'MAIN__'
    gprof_filename = path + '/' + 'profile.out'
    [parents,children] = gprofreading(gprof_filename,parents,children,times)
    print(parents.keys())
    print(children.keys())
    print(parents)
    print(children)
    func_flag_stack = {}
    stack = []
    #travel through the tree
    funclist = {}
    for filename in files:
        if os.path.splitext(filename)[1] == ".f90":
            filename = path + '/' + filename
            [funlist_t, filelist_t] = readfun(filename)
            funclist.update(funlist_t)
    #print(funclist)
    nonstop = 1
    while nonstop:
        temp_len = len(UQ)
        for filename in files:
            if os.path.splitext(filename)[1] == ".f90":
                filename = path + '/' + filename
                [UQFun, UQ] = finduq_in_singlefile(filename, UQFun, UQ, funclist)
        if len(UQ) == temp_len:
            nonstop = 0
        
    print(UQ)
    print(UQFun.mydict)


