#!/usr/bin/env python3
import sys
structures = []

class Structure():
    structureName = ''
    variables = {}

    def __init__(self, name):
        self.structureName = name
        self.variables ={}

    def add(self, variableName, count, size):
        size_name = variableName + '_size'
        self.variables[variableName] = count
        self.variables[size_name] = size


def countVariablesInStructure(splitted):

    if len(splitted) > 1:
        structureName = splitted[len(splitted)-2]
        #variable is in a structure
        for structure in structures:
            if (structure.structureName == structureName):
                #just update
                structure.add(splitted[len(splitted) - 1], entry[4], entry[3])

        else:
            #add new
            newStructure = Structure(splitted[len(splitted) - 2])
            newStructure.add(splitted[len(splitted) - 1], entry[4], entry[3])
            structures.append(newStructure)
        
        del splitted[-1]
        countVariablesInStructure(splitted)
        
    #variable is not in a structure
    return False

if len(sys.argv)<2:
    sys.exit()
file1 = open(sys.argv[1], 'r', encoding="ISO-8859-1")
Lines = file1.readlines()

functions = {}
function_enter=False

variable_list=[]
counter=0

#we are searching for function names
for line in Lines:
    counter+=1
    list=line.split()
    if list and list[0][0]=='-':
        if function_enter:
            functions[function_name]=variable_list
        function_name = ' '.join(list)
        variable_list=[]
        function_enter=True
    elif function_enter:
        if len(line.split())!=0:
            variable_list.append(line.split('\t'))
    elif list and list[0][0]=='=' and counter>6:
        break
    

for function in functions:
    for entry in functions[function]:
        for i in range(len(entry)):
            entry[i]=entry[i].strip()

for function in functions:
    print(function)
    for entry in functions[function]:
        #print(entry[0] + "  " +entry[4])
        splitted = entry[0].split('.')
        result = countVariablesInStructure(splitted)
    
for structure in structures:
    print(structure.structureName)
    print (structure.variables)
