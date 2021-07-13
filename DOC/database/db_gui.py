#!/usr/bin/python3
#coding=utf-8

import re
import common
import tkinter as tk
import tkinter.messagebox as messagebox

class show_database(tk.Frame):
    def __init__(self, parent, config, column, get_database):
        tk.Frame.__init__(self, parent)
        self.__config = config
        self.__database = get_database(self.__config)
        self.column = [{"data.info": x} for x in column]
        sbar_width = self.__config("common")["font"][1]
        if 0 == sbar_width:
            self.__sbar = tk.Scrollbar(self, command=self.__scrollbar_command)
        else:
            self.__sbar = tk.Scrollbar(self, command=self.__scrollbar_command, width=self.__config("common")["font"][1])
        for i, column in enumerate(self.column):
            column["data.class"] = "database.info." + column["data.info"]
            column["filter"] = ""
            self.__update_column_width(column, i)
            try:
                column["width"] = self.__config("common")[column["data.class"]+".width"]
                if common.debug:
                    print("column", i, "default width:", column["width"])
            except Exception as err:
                pass
            column_frame = tk.Entry(self)
            column_frame.pack(side="left", fill="both", expand="yes")
            column["entry"] = tk.Entry(column_frame, font=self.__config("common")["font"], width=column["width"])
            column["entry"].bind("<Return>", self.__entry_event_return(column, i))
            column["entry"].bind("<Double-Button-1>", self.__entry_event_button(column, i))
            column["entry"].pack(side="top", fill="both")
            column["listbox"] = tk.Listbox(column_frame, font=self.__config("common")["font"], width=column["width"], selectmode="single", yscrollcommand=self.__listbox_yscrollcommand)
            column["listbox"].pack(side="top", fill="both", expand="yes")
        self.__sbar.pack(side="left", fill="y", expand="no")
        self.__update_entry()
        self.__update_lbox()

    def __update_column_width(self, column, index):
        language = self.__config("language")[self.__config("common")["language"]]
        column["width"] = common.display.len(language[column["data.class"]])
        for value in self.__database:
            value = value[column["data.info"]]
            width = common.display.len(str(value))
            if column["width"] < width:
                column["width"] = width

    def __update_entry(self):
        language = self.__config("language")[self.__config("common")["language"]]
        for column in self.column:
            column["entry"].delete(0, "end")
            if column["filter"] != "":
                column["entry"].config(bg="#FFF", fg="#000")
                column["entry"].insert(0, column["filter"])
            else:
                column["entry"].config(bg="#000", fg="#FFF")
                column["entry"].insert(0, language[column["data.class"]])
    def __update_lbox(self):
        for column in self.column:
            column["listbox"].delete(0, "end")
        if self.__filter():
            total = 0
            for value in self.__database:
                if not self.__filter_value(value):
                    for column in self.column:
                       column["listbox"].insert("end", value[column["data.info"]])
                    total += 1
            return total
        else:
            for value in self.__database:
               for column in self.column:
                   column["listbox"].insert("end", value[column["data.info"]])
            return len(self.__database)

    def __filter(self):
        for column in self.column:
            if column["filter"] != "":
                return True
        return False

    def __filter_value(self, value):
        for column in self.column:
            if column["filter"] != "":
                if len(re.findall(column["filter"], str(value[column["data.info"]]))) == 0:
                    return True
        return False

    def __entry_event_return(self, column, index):
        def event(event):
            filte = column["entry"].get()
            if column["filter"] != filte:
                column["filter"] = filte
                total = self.__update_lbox()
                if common.debug:
                    print("filter=", [x["filter"] for x in self.column], ", data total=", total, sep="")
            self.__update_entry()
        return event

    def __entry_event_button(self, column, index):
        def event(event):
            self.__update_column_width(column, index)
            if common.debug:
                print("config column", index, "width:", column["width"])
            column["listbox"].config(width=column["width"])
        return event

    def __scrollbar_command(self, line, *argl):
        for column in self.column:
            column["listbox"].yview(line, *argl)

    def __listbox_yscrollcommand(self, start, end):
        self.__sbar.set(start, end)
        for column in self.column:
            column["listbox"].yview(int(column["listbox"].size() * float(start)))

class generate_file(tk.Frame):
    """docstring for generate_file"""
    def __init__(self, parent, config):
        tk.Frame.__init__(self, parent)
        self.__config = config
        #tk.Button(self, font=self.__config("common")["font"], text=self.__config("language", self.__config("common")["language"])["generate.28.head"], command=self.__get_button_command(self.__config.generate_head)).pack(side="left")
        tk.Button(self, font=self.__config("common")["font"], text=self.__config("language", self.__config("common")["language"])["generate.28.xlsx"], command=self.__get_button_command(self.__config.generate_xlsx)).pack(side="right")

    def __get_button_command(self, func):
        language = self.__config("language", self.__config("common")["language"])
        def command():
            func()
            messagebox.showinfo(title=language["tip"], message=language["finish"])
        return command

class start(tk.Tk):
    """docstring for index"""
    def __init__(self, config):
        self.__config = config
        tk.Tk.__init__(self)
        self.title(self.__config("language", self.__config("common")["language"])["preview"])
        show_database(self, config, ["id", "name", "group", "default_value", "property", "type", "description"], self.get_28_xlsx_db).pack(fill="both", expand="yes")
        generate_file(self, config).pack()
        self.mainloop()

    def get_28_xlsx_db(self, config):
        language = config("common")["language"]
        dbs = config.get_database("XLSX")
        datas = []
        for data in dbs:
            data = data.copy()
            if isinstance(data["id"], int):
                data["id"] = "0x{:08X}".format(data["id"])
            data["description"] = data["description."+language]
            datas += [data]
        return datas
