#!/usr/python3
#coding=utf-8

# 字宽度，可用于对齐，中文占两个字符位置
def word_len(uchar):
    for u in [
            ("\u4E00", "\u9FA5"),   # 基本汉字
            ("\u9FA6", "\u9FEF"),   # 基本汉字补充
            ("\u3400", "\u4DB5"),   # 扩展A
            ("\u20000", "\u2A6D6"), # 扩展B
            ("\u2A700", "\u2B734"), # 扩展C
            ("\u2B740", "\u2B81D"), # 扩展D
            ("\u2B820", "\u2CEA1"), # 扩展E
            ("\u2CEB0", "\u2EBE0"), # 扩展F
            ("\u2F00", "\u2FD5"),   # 康熙部首
            ("\u2E80", "\u2EF3"),   # 部首扩展
            ("\uF900", "\uFAD9"),   # 兼容汉字
            ("\u2F800", "\u2FA1D"), # 兼容扩展
            ("\uE815", "\uE86F"),   # PUA(GBK)部件
            ("\uE400", "\uE5E8"),   # 部件扩展
            ("\uE600", "\uE6CF"),   # PUA增补
            ("\u31C0", "\u31E3"),   # 汉字笔画
            ("\u2FF0", "\u2FFB"),   # 汉字结构
            ("\u3105", "\u312F"),   # 汉语注音
            ("\u31A0", "\u31BA"),   # 注音扩展
            ("\u3007", "\u3007"),   # 〇
            ("\uFF00", "\uFFEF"),   # 全角和半角，半形及全形字符
            ("\u2E80", "\u2EFF"),   # CJK部首补充
            ("\u3000", "\u303F"),   # CJK标点符号
            ("\u31C0", "\u31EF"),   # CJK笔划
        ]:
        if uchar >= u[0] and uchar<=u[1]:
            return 2
    return 1

# 计算文本显示宽度
def len(string):
    length = 0
    for uchar in string:
        length += word_len(uchar)
    return length
