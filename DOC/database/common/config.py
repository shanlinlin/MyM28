#!/usr/bin/python3
#coding=utf-8

class config(dict):
    """docstring for config"""
    def __init__(self, db, default={}):
        dict.__init__(self, dict_merge(db, default))

    def __call__(self, *keys):
        db = self
        for key in keys:
            db = db[key]
        return config(db)

def dict_merge(config, default):
    if isinstance(config, dict):
        new = default
        for key, value in config.items():
            if isinstance(value, dict):
                if key in new:
                    new[key] = dict_merge(value, new[key])
                else:
                    new[key] = value
            else:
                new[key] = value
        return new
    else:
        return config
