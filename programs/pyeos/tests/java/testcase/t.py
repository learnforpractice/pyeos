import os
import json
import math

import rodb
import debug
import eosapi
import wallet
import initeos
from common import prepare
from eosapi import N
import unittest

def init(func):
    def func_wrapper(*args, **kwargs):
        prepare('testcase', 'Testcase.java', 'Testcase.abi', __file__, 12)
        func(*args, **kwargs)
    return func_wrapper

@init
def testexit():
    eosapi.push_action('testcase', 'testexit', '', {'testcase':'active'})

@init
def testfileaccess():
    eosapi.push_action('testcase', 'testfile', '', {'testcase':'active'})

@init
def testmemory():
    eosapi.push_action('testcase', 'testmemory', '', {'testcase':'active'})

@init
def testcall():
    eosapi.push_action('testcase', 'testcall', '', {'testcase':'active'})

@init
def testtimeout():
    eosapi.push_action('testcase', 'testtimeout', '', {'testcase':'active'})

class LUATestCase(unittest.TestCase):
    def setUp(self):
        pass

    @unittest.expectedFailure
    def testexit(self):
        testexit()

    @unittest.expectedFailure
    def testfileaccess(self):
        testfileaccess()

    @unittest.expectedFailure
    def testmemory(self):
        testmemory()

    @unittest.expectedFailure
    def testcall(self):
        testcall()

    @unittest.expectedFailure
    def testtimeout(self):
        testtimeout()

    def tearDown(self):
        pass

def ut():
    unittest.main(module=ut.__module__, exit=False)
