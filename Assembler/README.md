Authors:
    Yuval Ronen 314806100
    Yotam Ben Eliyahu 311437941

Project method:
    We chose the approch of making a structure of Line info that contains parameters for all the data we need for each line,
     with this method after the first pass we have every thing that we need for making all the files and tables.

Emportent logic:
    1.We added two files that helped us keep track of the mechine proccess.
        ".afp" - after first pass - gives all the data we need of the LineInfo structure orginazied in a table.
        ".asp" - this is the machine code writen in binary 15 bits.
    2.entry/extern statment - we didnt seperate the table we used the ".afp" table for all needs,
    the lines that the values of opcode and labels ar -1/null are only statments lines.
        a.is entry label - the lines that contain a LabelName and contain isEntry=1 are in the entry label list.
        b.is extern label - the lines that contain a opcode that is a label (method 1) that is marked is extern is included at the extern lable list.

Syntax attention:
    1. When we copied a input file manualy, there where ghosted nodes that couldent been seen and give us a wrong output.
    2. When coping an input the char ' " ' didnt copy right if that hapens - rewrite the ' " ' chars and it will work.
