import unittest

from unittest_extensions import TestCase, args

from dictionaries import SSDict


class TestSSDict(TestCase):
    def dict(self) -> SSDict:
        return self._dict

    def setUp(self):
        self._dict = SSDict()


class TestSSDictSubscript(TestSSDict):
    def subject(self, key):
        return self.dict()[key]

    @args(2)
    def test_unset_item(self):
        self.assertResultRaises(KeyError)

    @args(1)
    def test_set_item(self):
        self.dict().__setitem__(1, 11)
        self.assertResult(11)

    @args({1})
    def test_unset_unhashable_item(self):
        self.assertResultRaises(TypeError)


class TestSSDictAssSubscript(TestSSDict):
    def subject(self, key, value):
        self.dict()[key] = value
        return self.dict()

    @args({1}, 1)
    def test_unhashable_key(self):
        self.assertResultRaises(TypeError)

    @args(12, [1, 2, 3])
    def test_new_key(self):
        self.assertEqual(self.result().__getitem__(12), [1, 2, 3])

    @args("a", "b")
    def test_existent_key(self):
        self.dict().__setitem__("a", "aa")
        self.assertEqual(self.result().__getitem__("a"), "b")


class TestSSDictLength(TestSSDict):
    def subject(self):
        return len(self.dict())

    def add_item(self, key=1, value=1):
        self.dict().__setitem__(key, value)

    def test_empty_dict(self):
        self.assertResult(0)

    def test_one_item(self):
        self.add_item()
        self.assertResult(1)

    def test_add_same_item(self):
        self.add_item()
        self.add_item()
        self.assertResult(1)

    def test_add_different_items(self):
        self.add_item()
        self.add_item(2)
        self.assertResult(2)


class TestSSDictContains(TestSSDict):
    def subject(self, key):
        return key in self.dict()

    @args("a")
    def test_key_not_in_dict(self):
        self.assertResultFalse()

    @args("a")
    def test_key_in_dict(self):
        self.dict().__setitem__("a", 1)
        self.assertResultTrue()

    @args({1})
    def test_unhashable_item(self):
        self.assertResultRaises(TypeError)


if __name__ == "__main__":
    unittest.main()
