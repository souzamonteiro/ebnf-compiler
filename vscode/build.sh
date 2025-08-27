#!/bin/sh

npm install -g vsce
vsce package
code --install-extension ebnf-extended-syntax-1.0.0.vsix