from FunctionClass import functionType,readOneLine
from read_variable import readstatement
def eatOneLine(currentContext):
    pos = 0
    [strLine,pos] = readOneLine(currentContext,pos)
    currentContext = currentContext[pos:]
    strLine = strLine.strip()
    return strLine,currentContext


def searchUQ(sourcename,UQ,parents_edit,children_edit,listFunction,funclist):
    RootFunction = "heat"
    keyword = ['write', 'print', 'do', 'integer', 'if', 'real']
    funcnamelist= [i for i in funclist.keys()]
    funcnamelist_lowercase = [i.lower() for i in funcnamelist]
    RootPos = funcnamelist.index(RootFunction)
    funcIndex= funcnamelist.index(sourcename)
    currentContext = listFunction[funcIndex].text
    currentFunc    = listFunction[funcIndex].name
    listFunction[funcIndex].UQ.append(UQ)
    CallStackText = []
    CallStackFunc = []
    CallStackLinePos = []
    stop_sign        = 0
    currentPos       = 0
    while(not stop_sign):
        while(currentContext != ""):
            bindStr = ''
            [strLine,currentContext] = eatOneLine(currentContext)
            if(strLine.endswith("&")):
                while(strLine.endswith("&")):
                    bindStr = bindStr + strLine
                    [strLine, currentContext] = eatOneLine(currentContext)
                bindStr = bindStr + strLine
            else:
                bindStr = strLine
            [variable, assig, call_flag, call_function] = readstatement(bindStr, funclist)
            currentPos += 1 #increasing linepos

            if (call_flag == 1 or call_function == 1):
                #                print(variable)
                FuncUQpos = []
                pos = 0
                num_UQ = 0
                for i in variable:
                    if i in listFunction[funcIndex].UQ and variable[1] in funcnamelist:
                        num_UQ += 1
                        FuncUQpos.append(pos - 2)
                    pos = pos + 1

                if(FuncUQpos):
                    if(currentPos not in  listFunction[curFUnc_ind].UQ_line_pos):
                        listFunction[curFUnc_ind].UQ_line_pos.append(currentPos)
                        listFunction[curFUnc_ind].UQ_content += bindStr + "\n"
                    funcIndex = funcnamelist.index(variable[1])
                    if(listFunction[funcIndex].UQtoken != 1):
                        CallStackText.append(currentContext)
                        CallStackFunc.append(currentFunc)
                        CallStackLinePos.append(currentPos)
                        currentContext = listFunction[funcnamelist.index(variable[1])].text
                        currentFunc    = listFunction[funcnamelist.index(variable[1])].name
                        currentPos     = 0
                    # it is a little bit long.
                    # However, the meaning is simple:
                    # adding corresponding UQ input to the UQ list from the Functionlist
                FuncUQinput = [listFunction[funcIndex].input[i] for i in FuncUQpos]
                listFunction[funcIndex].UQ = list(set(listFunction[funcIndex].UQ).union(set(FuncUQinput)))
                variable.clear()
                    # update the name in subtoutine definition
            if (assig == 1 and call_function != 1):
                for i in variable:
                    if i in listFunction[funcIndex].UQ and i not in keyword and variable[0] not in keyword:
                        if variable[0] not in listFunction[funcIndex].UQ:
                            listFunction[funcIndex].UQ.append(variable[0])
                            listFunction[funcIndex].UQ_content += bindStr + "\n"
                variable.clear()

            if (len(variable)>0 and variable[0] == "end"):
                if (currentFunc == "heat" and len(CallStackText)==0):
                    stop_sign = 1
                elif(len(CallStackText)>0):
                    children_func = currentFunc
                    currentContext = CallStackText.pop()
                    currentFunc    = CallStackFunc.pop()
                    currentPos     = CallStackLinePos.pop()
                    # the function comes to an end, update the input
                    updateUQ = listFunction[funcIndex].updateUQ_in_input()
                    if(updateUQ):
                        #return the whole function content to the current stack because the
                        #UQ variables are updated
                        curFUnc_ind = funcnamelist.index(currentFunc)
                        currentContext = listFunction[curFUnc_ind].text
                        children_func_ind = funcnamelist.index(children_func)
                        UQ_Temp = listFunction[children_func_ind].UQ_in_input()
                        # update the UQ in function
                        listFunction[curFUnc_ind].UQ = list(set(listFunction[curFUnc_ind].UQ).union(set(UQ_Temp)))
                        currentPos = 0
                else:
                    children_func            = currentFunc
                    currentFunc_lowercase    = parents_edit.get(currentFunc.lower())
                    updateUQ = listFunction[funcIndex].updateUQ_in_input()

                    if (currentFunc_lowercase == ["MAIN_"]):
                        currentFunc_lowercase = "heat"
                    funcIndex      = funcnamelist_lowercase.index(currentFunc_lowercase)
                    currentContext = listFunction[funcIndex].text
                    currentFunc    = funcnamelist[funcIndex]
                    currentPos     = 0
                    if(updateUQ):
                        #return the whole function content to the current stack because the
                        #UQ variables are updated
                        curFUnc_ind    = funcnamelist.index(currentFunc)
                        currentContext = listFunction[curFUnc_ind].text
                        children_func_ind = funcnamelist.index(children_func)
                        UQ_Temp = listFunction[children_func_ind].UQ_in_input()
                        # update the UQ in function
                        listFunction[curFUnc_ind].UQ = list(set(listFunction[curFUnc_ind].UQ).union(set(UQ_Temp)))
