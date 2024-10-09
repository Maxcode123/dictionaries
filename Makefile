PY=$(REPOS)/cpython/python
PIP=$(PY) -m pip

debug:
	gdb $(PY)

py:
	$(PY)

install-pip:
	$(PY) -m ensurepip --default-pip

install-requirements:
	$(PIP) install --upgrade -t $(REPOS)/cpython/Lib setuptools unittest-extensions

build-module:
	$(PY) setup.py install --home $(REPOS)/cpython/
	cp -r dist/ $(REPOS)/cpython/
	cp -r build/ $(REPOS)/cpython/

test:
	$(PY) -m unittest -v test.py

run:
	$(PY) example.py

clean:
	rm -rf dist/ build/ src/dictionaries.egg-info
	$(PIP) uninstall dictionaries -y
