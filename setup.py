from setuptools import Extension, setup

setup(
    ext_modules=[
        Extension(
            "dictionaries",
            sources=["src/ssdict/ssdict.c", "src/ssdict/node.c"],
            include_dirs=["src/ssdict"],
        )
    ]
)
