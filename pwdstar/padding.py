#!/usr/bin/env python
# -*- coding: utf-8 -*-

def _padding(text):
    textLen = len(text)
    pLen = ((textLen + 0xF) & ~0xF) - textLen

    result = pLen / textLen
    remainder = pLen % textLen

    return ((text * (result + 1)) + text[:remainder])

def padding(text, isKey = False):
    if isKey and len(text) > 16:
        return text[:16]
    else:
        return _padding(text)
