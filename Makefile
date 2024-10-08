PY=$(REPOS)/cpython/python
PIP=$(PY) -m pip

debug:
	gdb $(PY)

py:
	$(PY)

install-pip:
	$(PY) -m ensurepip --default-pip

install-requirements:
	$(PIP) install -t $(REPOS)/Lib setuptools

build-module:
	$(PY) setup.py install --home $(REPOS)/cpython/
	cp -r dist/ $(REPOS)/cpython/
	cp -r build/ $(REPOS)/cpython/

test:
	$(PY) test.py

clean:
	rm -rf dist/ build/
	$(PIP) uninstall dictionaries -y
