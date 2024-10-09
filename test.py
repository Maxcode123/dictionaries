import unittest

from unittest_extensions import TestCase, args

from dictionaries import SSDict


class TestSSDictSubscript(TestCase):
    def subject(self, key):
        return self.dict()[key]

    def dict(self) -> SSDict:
        return self._dict

    def setUp(self):
        self._dict = SSDict()

    @args({"key": 2})
    def test_unset_item(self):
        self.assertResultRaises(KeyError)

    @args({"key": 1})
    def test_set_item(self):
        self.dict().__setitem__(1, 11)
        self.assertResult(11)

    def test_unset_unhashable_item(self): ...


class TestSSDictAssSubscript(TestCase):
    pass


class TestSSDictLength(TestCase):
    pass


if __name__ == "__main__":
    unittest.main()


# d = SSDict()

# d.__setitem__(2, 20)

# d[2] = 10

# print("\nSet item successfully!\n")

# print(f"\nd[2] = {d.__getitem__(2)}\n")

# print(f"dictionary length: {d.__len__()}")

# print("\n\n--------------------------------------")
# print("Program ran successfully!")
# print("--------------------------------------\n\n")
