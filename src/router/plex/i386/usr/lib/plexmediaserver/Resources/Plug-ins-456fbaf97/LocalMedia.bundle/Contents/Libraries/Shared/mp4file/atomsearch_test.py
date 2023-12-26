'''
Created on Dec 26, 2009

@author: napier
'''
from atomsearch import find_path, findall_path
import unittest


class FakeAtom(object):
    def __init__(self, name, type):
        self.name = name
        self.type = type
        self.children = []

    def get_atoms(self):
        return self.children

class Test(unittest.TestCase):
    def setUp(self):
        self.mp4 = FakeAtom('root', 'root')
        self.mp4.children = [FakeAtom('child1', 'child1'),
                             FakeAtom('child2', 'child2')]
        child1 = self.mp4.children[0]
        child1.children = [FakeAtom('grandchild', 'gc'),
                           FakeAtom('granchild', 'gc')]

    def testFindSelf(self):
        root = find_path(self.mp4, '.')
        self.assertEquals(root.type, 'root')

    def testFindStar(self):
        gc1 = find_path(self.mp4, '*/gc')
        self.assertNotEquals(None, gc1)

    def testFindGc1(self):
        gc1 = find_path(self.mp4, './child1/gc')
        self.assertNotEqual(None, gc1)
        gc2 = find_path(self.mp4, './child1/grandchild')
        self.assertNotEqual(None, gc2)
        gc3 = find_path(self.mp4, 'child1/grandchild')
        self.assertNotEqual(None, gc3)
        self.assertEquals(gc1.type, gc2.type)
        self.assertEquals(gc2.type, gc3.type)

    def testFindall(self):
        res = findall_path(self.mp4, './child1/gc')
        self.assertEquals(2, len(res))
        all = findall_path(self.mp4, './/gc')
        self.assertEquals(2, len(all))

if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()