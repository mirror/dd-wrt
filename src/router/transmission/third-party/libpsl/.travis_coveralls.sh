#!/bin/bash

make check-coverage-libicu
pip install --user cpp-coveralls

coveralls --include src/
