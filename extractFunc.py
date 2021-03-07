#!/usr/bin/python3

#filename : extractFunc.py

import os
import sys

def extractFunc(filename):
    fin = open(filename)
    lines = fin.readlines()
    start = 0
    end = 0
    lineNum = -1
    cleanFuncLines = []
    deleteLineNames = ['global','extern','SECTION']
    for line in lines:
        flag = True;
        for item in deleteLineNames:
            if line.startswith(item):
                flag = False
                
        if flag:
            cleanFuncLines.append(line)
    fout = open(f'{filename[:-4]}.func', 'w')
    fout.writelines(cleanFuncLines)
    print(filename)

if __name__ == '__main__':
    extractFunc(sys.argv[1])