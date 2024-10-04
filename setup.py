from setuptools import Extension, setup

setup(ext_modules=[Extension("dictionaries", ["dictionaries.c"])])
