#!/usr/python3
#coding=utf-8

class iter_key:
    """docstring for iter_key"""
    def __init__(self, start, info={}):
        self.key    = start
        self.__info = info

    def __iter__(self):
        return self

    def __next__(self):
        if self.key in self.__info:
            return self.__info[self.key]
        raise StopIteration
