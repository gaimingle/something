#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os.path
import getpass
import hashlib
import glob
from Crypto.Cipher import AES

from padding import padding

CONFIG_DIR = ".pwdstar"
DATA_DIR = os.path.join(CONFIG_DIR, "data")
HASH_PWD_FILE = os.path.join(CONFIG_DIR, "hash_pwd")

key = ""

def initPwd():
    print "检测到第一次登陆，请设置登陆密码"
    pwd = getpass.getpass()
    hashPwd = hashlib.sha1(pwd).hexdigest()

    hashPwdFile = open(HASH_PWD_FILE, "w")
    hashPwdFile.write(hashPwd)

    global key
    key = padding(pwd, isKey = True)

def checkPwd():
    pwd = getpass.getpass()
    hashPwd = hashlib.sha1(pwd).hexdigest()

    hashPwdFile = open(HASH_PWD_FILE, "r")
    if hashPwd != hashPwdFile.read():
        print "密码错误"
        exit(1)

    global key
    key = padding(pwd, isKey = True)

def init():
    if not os.path.exists(CONFIG_DIR):
        os.mkdir(CONFIG_DIR)
        print "创建目录%s" % (CONFIG_DIR,)

    if not os.path.exists(DATA_DIR):
        os.mkdir(DATA_DIR)
        print "创建目录%s" % (DATA_DIR,)

    if os.path.exists(HASH_PWD_FILE):
        checkPwd()
    else:
        initPwd()

def listOne(filename):
    global key
    AESObj = AES.new(key)

    dataFile = open(filename, "r")
    plaintext = AESObj.decrypt(dataFile.read())

    accountLen = ord(plaintext[0])
    account = plaintext[1:accountLen+1]

    pwdPart = plaintext[accountLen+1:]
    pwdLen = ord(pwdPart[0])
    pwd = pwdPart[1:pwdLen+1]

    print "%s\t%s\t%s" % (os.path.split(filename)[1], account, pwd)

def list():
    for filename in glob.glob(os.path.join(DATA_DIR, "*")):
        listOne(filename)

def add():
    account = raw_input("请输入账号: ")
    newPwd = raw_input("请输入密码: ")
    comment = raw_input("请输入备注: ")

    plaintext = chr(len(account)) + account + chr(len(newPwd)) + newPwd
    plaintext = padding(plaintext)

    global key
    AESObj = AES.new(key)
    ciphertext = AESObj.encrypt(plaintext)

    dataFile = open(os.path.join(DATA_DIR, comment), "w")
    dataFile.write(ciphertext)

if __name__ == "__main__":
    init()

    print "欢迎使用Pwdstar"
    while True:
        print """
        1. 列出所有账号/密码对
        2. 添加一对账号/密码对"""

        option = raw_input("请选择需要的操作: ")

        if option == "1":
            list()
        elif option == "2":
            add()
        else:
            print "无效选项"
