#!/usr/bin/python3
#coding=utf-8

import os
import sys
import db_data
import db_gui

if __name__ == "__main__":
    if os.path.isdir(sys.argv[0]):
        abspath = os.path.abspath(sys.argv[0])
    elif os.path.isfile(sys.argv[0]):
        abspath = os.path.abspath(os.path.dirname(sys.argv[0]))
    data = db_data.db_data(os.path.splitext(abspath)[0]+os.sep+os.path.basename(abspath))
    argv = sys.argv[1:]
    if len(argv) == 0:
        db_gui.start(data)
    else:
        dos = {}
        for v in argv:
            if v == "head":
                dos[v] = data.generate_head
                print("head")
            elif v == "xlsx":
                dos[v] = data.generate_xlsx
                print("xlsx")
            elif v == "format":
                dos[v] = data.format_config
                print("format")
            else:
                print("help infomation.")
                print("format : format config file.")
                print("head   : generate 28 \".h\" file.")
                print("xlsx   : generate 28 \".xlsx\" file.")
        for do in dos:
            dos[do]()