#!/bin/sh

rm -rf build/*

# Creates uncompressed versions of the library.
cat src/Shebang.js src/REx.js src/EBNFCompiler.js  src/Main.js > build/ebnfcc.js

cp build/ebnfcc.js bin/

chmod 755 bin/*

