#!/usr/bin/python3
#coding=utf-8

import xlsxwriter
from json import JSONDecoder, JSONEncoder
import common

if common.debug:
    import timeit

def ljust(data, length):
    if None == data:
        return "null".ljust(length)
    elif isinstance(data, str):
        data = data.replace("\n", "\\n")
        return "\"{}\"{}".format(data, " " * (length - 2 - common.display.len(data)))
    elif isinstance(data, list):
        result = "["
        for v in data:
            result += "{}, ".format(ljust(v, 0))
        result = result + "]" if "[" == result else result[:-2] + "]"
        return result.ljust(length)
    elif isinstance(data, dict):
        result = "{"
        for k, v in data.items():
            result += "\"{}\": {}, ".format(k, ljust(v, 0))
        result = result + "}" if "{" == result else result[:-2] + "}"
        return result.ljust(length)
    return str(data).ljust(length)

def json_output(file, values):
    def json_output(file, values, join0="", tabs="", tab="\t"):
        join1 = "\n"
        if isinstance(values, list):
            tabs = tabs + tab
            for value in values:
                if isinstance(value, list):
                    file.write(join1 + tabs + "[")
                    json_output(file, value, join1, tabs)
                    file.write("\n" + tabs + "]")
                elif isinstance(value, dict):
                    file.write(join1 + tabs + "{")
                    json_output(file, value, join1, tabs)
                    file.write("\n" + tabs + "}")
                else:
                    file.write(join1 + tabs + ljust(value, 0))
                join1 = ",\n"
        elif isinstance(values, dict):
            for key, value in values.items():
                file.write(join1 + tabs + tab + "\"{}\": ".format(key))
                if isinstance(value, list):
                    file.write("[")
                    json_output(file, value, "\n", tabs + tab)
                    file.write("\n" + tabs + tab + "]")
                elif isinstance(value, dict):
                    file.write("{")
                    json_output(file, value, "\n", tabs + tab)
                    file.write("\n" + tabs + tab + "}")
                else:
                    file.write(ljust(value, 0))
                join1 = ",\n"
        else:
            file.write(join0 + tabs + ljust(values, 0))
    if isinstance(values, list):
        file.write("[")
        json_output(file, values)
        file.write("\n]")
    elif isinstance(values, dict):
        file.write("{")
        json_output(file, values)
        file.write("\n}")
    else:
        json_output(file, values)

class db_struct:
    """docstring for db_struct"""
    def __init__(self, database, info_tab):
        self.__database = database
        self.__info_tab = {k: v for v, k in enumerate(info_tab)}

    def __get_range(self, count):
        def data_convert(data):
            if isinstance(data, str):
                return int(data[2:], 16)
            return data
        if count:
            if len(count) > 2:
                c = count[0]
                i = count[1]
                s = count[2]
            elif len(count) > 1:
                c = count[0]
                i = count[1]
                s = 1
            else:
                return (None, None, None)
        else:
            return (None, None, None)
        return (data_convert(c), data_convert(i), data_convert(s))

    def get_info(self, info, infos=None):
        if not infos:
            infos = []
        for data in self.__database[info]:
            if isinstance(data, dict):
                info = data["data"]
                if info not in infos:
                    infos = self.get_info(info, infos)
                    if info not in infos:
                        infos += [info]
        return infos

    def get_once(self, info, key, keys=None):
        if not keys:
            keys = []
        for data in self.__database[info]:
            if isinstance(data, list):
                t = data[self.__info_tab[key]]
                if t not in keys:
                    keys += [t]
            elif isinstance(data, dict):
                count = self.__get_range(data["count"])[0]
                if count:
                    for i in range(count):
                        keys = self.get_once(data["data"], key, keys)
                else:
                    keys = self.get_once(data["data"], key, keys)
            else:
                raise BaseException("info={}".format(info))
        return keys

    def get_struct(self, info, fold=False, **argd):
        if "base" not in argd:
            argd["base"] = []
        if "names" not in argd:
            argd["names"] = []
        if "types" not in argd:
            argd["types"] = []
        if "description" not in argd:
            argd["description"] = {}
        structs = []
        for i, data in enumerate(self.__database[info]):
            if isinstance(data, list):
                struct = {}
                for key in self.__info_tab:
                    struct[key] = data[self.__info_tab[key]]
                struct["id"] = argd["base"] + [int(data[self.__info_tab["id"]][2:], 16) if isinstance(data[self.__info_tab["id"]], str) else data[self.__info_tab["id"]]]
                struct["name"] = argd["names"] + [data[self.__info_tab["name"]]]
                struct["type"] = argd["types"] + [data[self.__info_tab["type"]]]
                for desc in argd["description"]:
                    struct["description." + desc] = argd["description"][desc] + [data[self.__info_tab["description." + desc]]]
                structs += [struct]
            elif isinstance(data, dict):
                arg = {}
                id_base = int(data["base"][2:], 16) if isinstance(data["base"], str) else data["base"]
                count, id_inc, start = self.__get_range(data["count"])
                if count:
                    for i in range(count):
                        arg["base"] = argd["base"] + [id_base]
                        arg["names"] = argd["names"] + ["{}{}".format(data["name"] if data["name"] else "_", start+i)]
                        arg["types"] = argd["types"] + [data["data"]]
                        arg["description"] = {}
                        for desc in data["description"]:
                            if desc in argd["description"]:
                                arg["description"][desc] = argd["description"][desc] + ["{}{}".format(data["description"][desc], start+i)]
                            else:
                                arg["description"][desc] = ["{}{}".format(data["description"][desc], start+i)]
                        if fold:
                            structs += [{"id": arg["base"], "name": arg["names"], "description": arg["description"], "data": data["data"]}]
                        else:
                            structs += self.get_struct(data["data"], fold, **arg)
                        id_base += id_inc
                else:
                    arg["base"] = argd["base"] + [id_base]
                    arg["names"] = argd["names"] + [data["name"]] if data["name"] else argd["names"]
                    arg["types"] = argd["types"] + [data["data"]]
                    arg["description"] = {}
                    for desc in data["description"]:
                        if desc in argd["description"]:
                            arg["description"][desc] = argd["description"][desc] + [data["description"][desc]]
                        else:
                            arg["description"][desc] = [data["description"][desc]]
                    if fold:
                        structs += [{"id": arg["base"], "name": arg["names"], "description": arg["description"], "data": data["data"]}]
                    else:
                        structs += self.get_struct(data["data"], fold, **arg)
            else:
                raise BaseException("datas={}".format(datas))
        return structs

    def get_groups(self, info):
        def __get_groups(info, name="", data_name=""):
            groups = {}
            for data in self.__database[info]:
                if isinstance(data, list):
                    groups[name] = data_name[:-1]
                elif isinstance(data, dict):
                    count, id_inc, start = self.__get_range(data["count"])
                    if count:
                        if data["name"]:
                            n0 = data["name"]
                        else:
                            n0 = "_"
                        groups[n0] = {}
                        for i in range(count):
                            n = "{}{}".format(n0, start+i)
                            dn = "{}{}.".format(data_name, n)
                            groups[n0][n] = __get_groups(data["data"], n, dn)
                    else:
                        if data["name"]:
                            n = data["name"]
                            dn = "{}{}.".format(data_name, n)
                            groups[data["name"]] = __get_groups(data["data"], n, dn)
                        else:
                            for key, value in __get_groups(data["data"], name, data_name).items():
                                groups[key] = value
            return groups
        def __convert_groups(groups):
            result = []
            for key, value in groups.items():
                if isinstance(value, str):
                    result += [value]
                else:
                    if key in value and len(value.keys()) == 1:
                        result += [value[key]]
                    else:
                        result += [{key: __convert_groups(value)}]
            return result
        return __convert_groups(__get_groups(info))

    def get_database(self, info, check=False):
        database = []
        for data in self.get_struct(info):
            data["id"] = sum(data["id"])
            data["group"] = ".".join(data["name"][:-1])
            data["name"] = "_".join(data["name"])
            data["type"] = data["type"][-1]
            data["description.english"] = "".join(data["description.english"])
            data["description.简体中文"] = "".join(data["description.简体中文"])
            data["description.繁体中文"] = "".join(data["description.繁体中文"])
            database += [data]
        return database

    def check_data(self, info, check=False):
        database = []
        for data in self.get_struct(info):
            data["id"] = sum(data["id"])
            data["group"] = ".".join(data["name"][:-1])
            data["name"] = "_".join(data["name"])
            data["type"] = data["type"][-1]
            data["description.english"] = "".join(data["description.english"])
            data["description.简体中文"] = "".join(data["description.简体中文"])
            data["description.繁体中文"] = "".join(data["description.繁体中文"])
            database += [data]
        print("{} data total={}".format(info, len(database)))
        if check:
            debug_time = timeit.default_timer()
            for i, v in enumerate(database):
                if None != v["default_value"] and None != v["min_value"]:
                    if v["default_value"] < v["min_value"]:
                        print("\"min\" too large: ( {} ).".format(key))
                if None != v["default_value"] and None != v["max_value"]:
                    if v["default_value"] > v["max_value"]:
                        print("\"max\" too small: ( {} ).".format(key))
                for j in range(i+1, len(database)):
                    if (database[i]["id"]) >= database[j]["id"]:
                      print("name: ( {} , {} ), id error: ( 0x{:08X} , 0x{:08X} ).".format(database[i]["name"], database[j]["name"], database[i]["id"], database[j]["id"]))
                    if database[i]["name"] == database[j]["name"]:
                      print("same of name: ( {} ).".format(v["name"]))
                    if database[i]["description.english"] == database[j]["description.english"]:
                      print("name: ( {} , {} ), same of \"description.english\": ( {} ).".format(database[i]["name"], database[j]["name"], v["description.english"]))
                    if database[i]["description.简体中文"] == database[j]["description.简体中文"]:
                      print("name: ( {} , {} ), same of \"description.简体中文\": ( {} ).".format(database[i]["name"], database[j]["name"], v["description.简体中文"]))
                    if database[i]["description.繁体中文"] == database[j]["description.繁体中文"]:
                      print("name: ( {} , {} ), same of \"description.繁体中文\": ( {} ).".format(database[i]["name"], database[j]["name"], v["description.繁体中文"]))
            print("check {} data finish: {:.3f}s.".format(info, timeit.default_timer() - debug_time))

    def format_info(self, info):
        max_len_list = [0 for x in self.__info_tab]
        max_len_dict = {x: 0 for x in ["base", "name", "count", "data"]}
        max_len_desc = {}
        for data in self.__database[info]:
            if isinstance(data, dict):
                for key in max_len_dict:
                    length = common.display.len(ljust(data[key], 0))
                    if max_len_dict[key] < length:
                        max_len_dict[key] = length
                for key, value in data["description"].items():
                    length = common.display.len(ljust(value, 0))
                    if key in max_len_desc:
                        if max_len_desc[key] < length:
                            max_len_desc[key] = length
                    else:
                        max_len_desc[key] = length
            elif isinstance(data, list):
                for index, value in enumerate(max_len_list):
                    length = common.display.len(ljust(data[index], 0))
                    if max_len_list[index] < length:
                        max_len_list[index] = length
        for data in self.__database[info]:
            if isinstance(data, dict):
                result = "{"
                description = " \"description\": {"
                for key, value in data.items():
                    if "description" == key:
                        join0 = " "
                        for key, value in value.items():
                            description += join0 + "\"{}\": {} ".format(key, ljust(value, max_len_desc[key]))
                            join0 = ", "
                    elif key in max_len_dict:
                        result += " \"{}\": {} ,".format(key, ljust(value, max_len_dict[key]))
                    else:
                        result += " \"{}\": {} ,".format(key, ljust(value, 0))
                yield result + description + "} }"
            elif isinstance(data, list):
                result = "["
                join0 = " "
                for index, value in enumerate(data):
                    result += join0 + ljust(value, max_len_list[index]) + " "
                    join0 = ", "
                yield result + "]"

    def generate_xlsx(self, file_name, info):
        columns = (
            ("id", "ID"),
            ("name", "Name"),
            ("group", "Group"),
            ("reserved", "Reserved"),
            ("type", "DataType"),
            ("property", "Property"),
            ("loop", "LoopRequest"),
            ("privilege", "Privilege"),
            ("display_control", "DisplayControl"),
            ("display_value", "DisplayValue"),
            ("unit", "Unit"),
            ("precision", "Precision"),
            ("default_value", "DefaultValue"),
            ("min_value", "Min"),
            ("max_value", "Max"),
            ("description.english", "DescriptionE"),
            ("description.简体中文", "936"),
            ("description.繁体中文", "950"))

        workbook = xlsxwriter.Workbook(file_name + ".xlsx")
        worksheet = workbook.add_worksheet('database')
        database = self.get_database(info)

        # first line.
        for i, value in enumerate(columns):
            worksheet.write(0, i, value[1])
        # other line.
        for i, data in enumerate(database, 1):
            data = data.copy()
            data["reserved"] = 0
            data["loop"] = 0
            data["id"] = "0x{:08X}".format(data["id"])
            if None == data["display_control"]:
                if isinstance(data["display_value"], str):
                    data["display_control"] = "Combox"
                elif "FLOAT" == data["type"]:
                    data["display_control"] = "Spinedit-Float"
                else:
                    data["display_control"] = "Spinedit-Int"
            if data["property"] == "RO":
                data["property"] = "READONLY"
            elif data["property"] == "RW":
                data["property"] = "READWRITE"
            if None == data["min_value"]:
                if data["type"] == "FLOAT":
                    data["min_value"] = -pow(2, 32)
                elif data["type"] == "DWORD" or data["type"] == "WORD" or data["type"] == "BYTE":
                    data["min_value"] = 0
                elif data["type"] == "LONG":
                    data["min_value"] = -0x80000000
                elif data["type"] == "INT" or data["type"] == "SHORT":
                    data["min_value"] = -0x8000
                elif data["type"] == "BOOL":
                    data["min_value"] = 0
                else:
                    print("DataType( {} ) of \"Min\" no process.".format(data["type"]))
            if None == data["max_value"]:
                if data["type"] == "FLOAT":
                    data["max_value"] = pow(2, 32) - 1
                elif data["type"] == "DWORD":
                    data["max_value"] = 0xFFFFFFFF
                elif data["type"] == "WORD":
                    data["max_value"] = 0xFFFF
                elif data["type"] == "BYTE":
                    data["max_value"] = 0xFF
                elif data["type"] == "LONG":
                    data["max_value"] = 0x7FFFFFFF
                elif data["type"] == "INT" or data["type"] == "SHORT":
                    data["max_value"] = 0x7FFF
                elif data["type"] == "BOOL":
                    data["max_value"] = 1
                else:
                    print("DataType( {} ) of \"Max\" no process.".format(data["type"]))
            if data["type"] == "SHORT":
                data["type"] = "INT"
            for col, value in enumerate(columns):
                worksheet.write(i, col, data[value[0]])
        workbook.close()
        self.generate_group(file_name, info)

    def generate_group(self, file_name, info):
        with open(file_name + ".group", mode="w", encoding="utf-8") as file:
            groups = self.get_groups(info)
            json_output(file, groups)

    def generate_head(self, file_name, info):
        if common.debug:
            self.get_database(info)
        with open(file_name + ".h", mode="w", encoding="utf-8") as file:
            file.write("#ifdef DATABASE_ENUMERATION_H\n\n")

            file.write("enum {\n")
            for i, v in enumerate(self.get_struct(info, True)):
                file.write("\tDBGROUP_{}, // {}\n".format(v["name"][0], i))
            file.write("\n\tDBGROUP_SUM // {}\n".format(i+1))
            file.write("};\n\n")

            for v in self.get_info(info):
                file.write("enum {\n")
                for i, s in enumerate(self.get_struct(v)):
                    file.write("\tDBGROUPINDEX_{}_{}, // {}\n".format(v, "_".join(s["name"]), i))
                file.write("\n\tDBGROUPINDEX_{}_SUM // {}\n".format(v, i+1))
                file.write("};\n")
            file.write("\n")

            file.write("typedef enum {\n")
            for i, s in enumerate(self.get_struct(info)):
                file.write("\tDBINDEX_{}, // {}\n".format("_".join(s["name"]), i))
            file.write("\n\tDBINDEX_SUM // {}\n".format(i+1))
            file.write("} DBIndex_E;\n\n")

            file.write("typedef enum {\n")
            for i, n in enumerate(self.get_once(info, "type")):
                file.write("\tDATATYPE_{}, // {}\n".format(n, i))
            file.write("\n\tDATATYPE_SUM // {}\n".format(i+1))
            file.write("} E_DataType;\n\n")

            file.write("#endif // DATABASE_ENUMERATION_H\n")
            file.write("#ifdef DATABASE_ENUMERATION_C\n\n")

            file.write("enum {\n")
            for i, n in enumerate([x for x in self.get_once(info, "callback") if x]):
                file.write("\tDBCALLBACK_{}, // {}\n".format(n, i))
            file.write("\n\tDBCALLBACK_SUM // {}\n".format(i+1))
            file.write("};\n\n")

            file.write("#endif // DATABASE_ENUMERATION_C\n")
            file.write("#ifdef DATABASE_TYPE_DEFINITION_H\n\n")

            def __generate_block_struct(v):
                structs = self.get_struct(v, True)
                max_len = 0
                for struct in structs:
                    if "data" in struct:
                        struct["type"] = "S_DBBlock_" + struct["data"]
                    else:
                        struct["type"] = struct["type"][0]
                    if max_len < len(struct["type"]):
                        max_len = len(struct["type"])
                file.write("typedef struct {\n")
                for struct in structs:
                    file.write("\t{} {};\n".format(struct["type"].ljust(max_len), struct["name"][0]))
                file.write("{}{};\n".format("} S_DBBlock_", v))
            for v in self.get_info(info):
                __generate_block_struct(v)
            __generate_block_struct(info)
            file.write("typedef S_DBBlock_{} S_DB;\n".format(info))
            file.write("\n")

            file.write("typedef union {\n")
            _type = self.get_once(info, "type")
            max_len = 0
            for t in _type:
                if max_len < len(t):
                    max_len = len(t)
            for i, t in enumerate(_type):
                file.write("\t{} v{};\n".format(t.ljust(max_len), t))
            file.write("} DBVALUE;\n\n")

            file.write("#endif // DATABASE_TYPE_DEFINITION_H\n")
            file.write("#ifdef DATABASE_FUNCTION_DECLARATION_H\n\n")

            for c in self.get_once(info, "callback"):
                file.write("extern E_DBRetCode {}(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData);\n".format(c))
            file.write("\n")

            file.write("#endif // DATABASE_FUNCTION_DECLARATION_H\n")
            file.write("#ifdef DATABASE_DATA_DECLARATION_C\n\n")

            file.write("static E_DBRetCode (*const uc_afCallback[])(DBIndex_T tIndex, E_DBEvent eEvent, DBVALUE const* pvData) = {\n")
            for n in [x for x in self.get_once(info, "callback") if x]:
                file.write("\t{},\n".format(n))
            file.write("};\n\n")

            blocks = []
            for s in self.get_struct(info, True):
                if "data" in s:
                    if s["data"] in blocks:
                        continue
                    blocks += [s["data"]]
                    file.write("{}{}[] = {}\n".format("static DBAttrib_S const uc_asBlock_", s["data"], "{"))
                    data = []
                    for i, d in enumerate(self.get_struct(s["data"])):
                        data += [{}]
                        data[i]["id"] = sum(d["id"])
                        data[i]["offset_of"] = "{}, {}".format(s["data"], ".".join(d["name"]))
                        data[i]["property"] = d["property"]
                        data[i]["type"] = d["type"][-1]
                        data[i]["callback"] = d["callback"]
                    max_len = {x: 0 for x in ["offset_of", "type", "callback"]}
                    for d in data:
                        if max_len["offset_of"] < len(d["offset_of"]):
                            max_len["offset_of"] = len(d["offset_of"])
                        if max_len["type"] < len(d["type"]):
                            max_len["type"] = len(d["type"])
                        if max_len["callback"] < len(d["callback"]):
                            max_len["callback"] = len(d["callback"])
                    for d in data:
                        file.write("\t{ ")
                        file.write("0x{:08X} , OffsetOf(S_DBBlock_{}) , DATAACCESS_{} , DATATYPE_{}, DBCALLBACK_{}".format(
                            d["id"],
                            d["offset_of"].ljust(max_len["offset_of"]),
                            d["property"],
                            d["type"].ljust(max_len["type"]),
                            d["callback"].ljust(max_len["callback"])))
                        file.write(" },\n")
                    file.write("};\n")
                else:
                    file.write("{}{} = {}".format("static DBAttrib_S const uc_sBlock_", s["name"][0], "{ "))
                    file.write("0 , 0 , DATAACCESS_{} , DATATYPE_{}, DBCALLBACK_{}".format(
                        s["property"],
                        "".join(s["type"]),
                        s["callback"]))
                    file.write(" };\n")
            file.write("\n")

            file.write("WORD const gc_asDataType[] = {\n")
            for t in self.get_once(info, "type"):
                file.write("\tsizeof({}),\n".format(t))
            file.write("};\n\n")

            file.write("DBGroup_S const gc_asGroup[] = {\n")
            struct = self.get_struct(info, True)
            data = []
            max_len = {x: 0 for x in ["attrib", "addr_base"]}
            for i, s in enumerate(struct):
                data += [{}]
                if "data" in s:
                    data[i]["attrib"] = "uc_asBlock_" + s["data"]
                else:
                    data[i]["attrib"] = "&uc_sBlock_" + s["name"][-1]
                data[i]["addr_base"] = s["name"][-1]
                data[i]["id"] = "0x{:08X}".format(sum(s["id"]))
                if max_len["attrib"] < len(data[i]["attrib"]):
                    max_len["attrib"] = len(data[i]["attrib"])
                if max_len["addr_base"] < len(data[i]["addr_base"]):
                    max_len["addr_base"] = len(data[i]["addr_base"])
            for i, t in enumerate(struct):
                file.write("\t{}{} , &g_sDB.{} , {}{}\n".format(
                    "{ ",
                    data[i]["attrib"].ljust(max_len["attrib"]),
                    data[i]["addr_base"].ljust(max_len["addr_base"]),
                    data[i]["id"],
                    " },"))
            file.write("};\n\n")

            file.write("DBAttribInfo_S const gc_asAttribInfo[] = {\n")
            max_len = {x: 0 for x in ["group", "attrib"]}
            data = []
            for i, s in enumerate(self.get_struct(info)):
                data += [{}]
                data[i]["group"] = s["name"][0]
                if max_len["group"] < len(data[i]["group"]):
                    max_len["group"] = len(data[i]["group"])
            i = 0
            for s in self.get_struct(info, True):
                if "data" in s:
                    for s1 in self.get_struct(s["data"]):
                        data[i]["attrib"] = "DBGROUPINDEX_{}_{}".format(s["data"], "_".join(s1["name"]))
                        if max_len["attrib"] < len(data[i]["attrib"]):
                            max_len["attrib"] = len(data[i]["attrib"])
                        i += 1
                else:
                    data[i]["attrib"] = "0"
                    if max_len["attrib"] < len(data[i]["attrib"]):
                        max_len["attrib"] = len(data[i]["attrib"])
                    i += 1
            for d in data:
                file.write("\t{ ")
                file.write("DBGROUP_{} , {}".format(
                    d["group"].ljust(max_len["group"]),
                    d["attrib"].ljust(max_len["attrib"])))
                file.write(" },\n")
            file.write("};\n\n")

            file.write("static DBIndex_T const uc_asFindInitTab[][2] = {\n")
            find_tab = []
            for i, v in enumerate(self.get_struct(info)):
                _id = sum(v["id"])
                index = _id >> 24
                if index <= len(find_tab) - 1:
                    end = "DBINDEX_" + "_".join(v["name"])
                    find_tab[index][1] = end
                else:
                    while len(find_tab) < index:
                        find_tab += [["1", "0"]]
                    else:
                        start = "DBINDEX_" + "_".join(v["name"])
                        find_tab += [[start, start]]
            max_len = [0, 0]
            for v in find_tab:
                if max_len[0] < len(v[0]):
                    max_len[0] = len(v[0])
                if max_len[1] < len(v[1]):
                    max_len[1] = len(v[1])
            for i, v in enumerate(find_tab):
                file.write("{}{} , {}{}0x{:02X}\n".format(
                    "\t{ ",
                    v[0].ljust(max_len[0]),
                    v[1].ljust(max_len[1]),
                    " }, // ",
                    i))
            file.write("};\n\n")

            file.write("#endif // DATABASE_DATA_DECLARATION_C\n")
            file.write("#ifdef DATABASE_FUNCTION_PROTOTYPE_H\n\n")

            for t in self.get_once(info, "type"):
                file.write("inline void DB_Set" + t + "(DBIndex_T vIndex, " + t + " vData)\n{\n")
                file.write("\tDEBUG_ASSERT(DB_GetDataType(vIndex) == DATATYPE_" + t + ");\n")
                file.write("\t*(" + t + "*)DB_GetDataAddr(vIndex) = vData;\n}\n")
                file.write("inline " + t + " DB_Get" + t + "(DBIndex_T vIndex)\n{\n")
                file.write("\tDEBUG_ASSERT(DB_GetDataType(vIndex) == DATATYPE_" + t + ");\n")
                file.write("\treturn *(" + t + "*)DB_GetDataAddr(vIndex);\n}\n")
            file.write("\n")

            file.write("#endif // DATABASE_FUNCTION_PROTOTYPE_H\n")

class db_data(db_struct, common.config):
    """docstring for db_data"""

    def __init__(self, file_name):
        self.file_name = file_name
        self.__update()

    def __update(self):
        self.__database_buffer = {}
        file_name = self.file_name + ".json"
        if common.debug:
            print("config file:", file_name)
            debug_time = timeit.default_timer()
        db = ""
        with open(file_name, mode="r", encoding="utf-8") as f:
            db += f.read()
        common.config.__init__(self, JSONDecoder().decode(db),
            {
                "common": {
                    "language": "简体中文",
                    "font": ["", 0],
                    "xlsx.file.name": self.file_name,
                    "head.file.name": self.file_name,
                    "database.index": ["id", "name", "property", "type", "callback", "privilege", "display_control", "display_value", "unit", "precision", "default_value", "min_value", "max_value", "description.english", "description.简体中文", "description.繁体中文"],
                },
                "language": {
                    "简体中文" : {
                        "database.content": "数据库内容",
                        "database.info.id": "ID",
                        "database.info.name": "名字",
                        "database.info.group": "分组",
                        "database.info.reserved": "保留",
                        "database.info.loop": "循环请求",
                        "database.info.callback": "回调函数",
                        "database.info.type": "数据类型",
                        "database.info.property": "资料访问限定",
                        "database.info.privilege": "资料授权等级",
                        "database.info.display_control": "显示控制",
                        "database.info.display_value": "显示值",
                        "database.info.unit": "单位",
                        "database.info.precision": "精度",
                        "database.info.default_value": "默认值",
                        "database.info.min_value": "最小值",
                        "database.info.max_value": "最大值",
                        "database.info.description": "描述",
                        "generate.28.head": "生成C语言头文件",
                        "generate.28.xlsx": "生成数据表文件",
                        "finish": "完成",
                        "submit": "提交",
                        "tip": "提示",
                        "preview": "预览",
                    },
                },
            })
        if common.debug:
            print("parse config finish: {:.3f}s.".format(timeit.default_timer() - debug_time))
        db_struct.__init__(self, self["database"], self("common")["database.index"])
        self.__database_buffer = {}

    def get_database(self, info):
        if info in self.__database_buffer:
            if common.debug:
                print("return database by buffer:", info)
            return self.__database_buffer[info]
        if common.debug:
            db_struct.check_data(self, info, common.check_data_error)
        database = db_struct.get_database(self, info)
        self.__database_buffer[info] = database
        return database

    def format_config(self):
        db = ""
        with open(self.file_name + ".json", mode="r", encoding="utf-8") as f:
            db += f.read()
        with open(self.file_name + ".json", mode="w", encoding="utf-8") as f:
            config = JSONDecoder().decode(db)
            items = config.items()
            if items:
                f.write("{")
                join0 = "\n"
                for key, value in items:
                    f.write(join0 + "\t\"{}\": {}".format(key, "{"))
                    join0 = ",\n"
                    if "database" in key:
                        join1 = "\n"
                        for k, v in value.items():
                            f.write(join1 + "\t\t\"{}\": [".format(k))
                            join1 = ",\n"
                            join2 = "\n"
                            for v in db_struct.format_info(self, k):
                                f.write(join2 + "\t\t\t{}".format(v))
                                join2 = ",\n"
                            f.write("\n\t\t]")
                    elif "language" in key:
                        join1 = "\n"
                        for key, value in value.items():
                            f.write(join1 + "\t\t\"{}\": {}".format(key, "{"))
                            join1 = ",\n"
                            join2 = "\n"
                            for k, v in value.items():
                                f.write(join2 + "\t\t\t\"{}\": {}".format(k, ljust(v, 0)))
                                join2 = ",\n"
                            f.write("\n\t\t}")
                    else:
                        if isinstance(value, dict):
                            join1 = "\n"
                            for k, v in value.items():
                                f.write(join1 + "\t\t\"{}\": {}".format(k, ljust(v, 0)))
                                join1 = ",\n"
                        else:
                            f.write(ljust(value, 0))
                    f.write("\n\t}")
                f.write("\n}\n")

    def generate_xlsx(self):
        db_struct.generate_xlsx(self, self("common")["xlsx.file.name"], "XLSX")

    def generate_head(self):
        db_struct.generate_head(self, self("common")["head.file.name"], "HEAD")
