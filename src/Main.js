/**
 * @license
 * Copyright 2020 Roberto Luiz Souza Monteiro,
 *                Renata Souza Barreto,
 *                Hernane Borges de Barros Pereira.
 *
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * EBNF main class.
 * @class
 */
function Main() {
    init();

    /**
     * Creates the attributes of the class.
     */
    function init() {
        // Class attributes goes here.
        compiledCode = {
            'xml': '',
            'asm': ''
        }
    }

    /**
     * Compiles the EBNF code passed as a command line argument.
     */
    this.run = function() {
        // Supports only the Node.js interpreter.
        if (typeof process !== 'undefined') {
            var command = 'node';
            var argv = process.argv.slice();
            
            var fs = require('fs');
            var readTextFile = fs.readFileSync;

            function read(input) {
                if (/^{.*}$/.test(input)) {
                    return input.substring(1, input.length - 1);
                } else {
                    var content = readTextFile(input, 'utf-8');
                    return content.length > 0 && content.charCodeAt(0) == 0xFEFF ? content.substring(1) : content;
                }
            }

            var buffer = require("buffer");
            var Buffer = buffer.Buffer;

            compiledCode.xml = '';

            function getXml(data) {
                compiledCode.xml += data;
            }

            var inputFile;
            var outputFile;
            var indentCode = false;
            var indentationLength = 4;
            var outputFileType = 'json';
            var outputContents = '';
            
            if (argv.length > 2) {
                var i = 2;
                while (i < argv.length) {
                    if ((argv[i] == '-h') || (argv[i] == '--help')) {
                        console.log('EBNF Command Line Interface (CLI)');
                        console.log('Usage: ebnfcc [options] [script.maia] [--] [arguments]');
                        console.log('Options:');
                        console.log('-h     --help               Displays this help message.');
                        console.log('-o     <script.js>          Output file name.');
                        console.log('       --indent             Indent the output code.');
                        console.log('       --spaces             Number of spaces in the indentation.');
                        console.log('-x                          Just compile to XML, don\'t compile to the target language.');
                        console.log('       --                   End of compiler options.\n');
                    } else if (argv[i] == '-o') {
                        i++;
                        outputFile = argv[i];
                    } else if (argv[i] == '--indent') {
                        indentCode = true;
                    } else if (argv[i] == '--spaces') {
                        i++;
                        indentationLength = core.toNumber(argv[i]);
                    } else if (argv[i] == '-x') {
                        justCompile = true;
                        outputFileType = 'xml';
                    } else {
                        inputFile = argv[i];
                        break;
                    }
                    i++;
                }
                if (typeof inputFile != 'undefined') {
                    var code = read(String(inputFile));
                    var s = new REx.XmlSerializer(getXml, false);
                    var ebnfScriptParser = new REx(code, s);
                    try {
                        ebnfScriptParser.parse_Grammar();
                    } catch (pe) {
                        if (!(pe instanceof ebnfScriptParser.ParseException)) {
                            throw pe;
                        } else {
                            var parserError = ebnfScriptParser.getErrorMessage(pe);
                            console.log(parserError);
                            throw parserError;
                        }
                    }
                    var parser = new DOMParser();
                    var xml = parser.parseFromString(compiledCode.xml, 'text/xml');
                    var compiler = new EBNFCompiler();
                    compiledCode = compiler.compile(xml, indentCode, indentationLength);
                    console.log(compiledCode.asm);
                    
                    if (typeof outputFile == 'undefined') {
                        var fileName = inputFile.split('.').shift();
                        if (outputFileType == 'json') {
                            outputFile = fileName + '.json';
                            if (indentCode) {
                                outputContents = JSON.stringify(compiledCode.asm, null, indentationLength);
                            } else {
                                outputContents = JSON.stringify(compiledCode.asm);
                            }
                        } else if (outputFileType == 'asm') {
                            outputFile = fileName + '.asm';
                            if (indentCode) {
                                outputContents = JSON.stringify(compiledCode.asm, null, indentationLength);
                            } else {
                                outputContents = JSON.stringify(compiledCode.asm);
                            }
                        } else if (outputFileType == 'xml') {
                            outputFile = fileName + '.xml';
                            outputContents = compiledCode.xml;
                        } else {
                            outputFile = fileName + '.asm';
                            outputContents = compiledCode.asm;
                        }
                    } else {
                        outputContents = compiledCode.js;
                    }
                    fs.writeFileSync(outputFile, outputContents);
                } else {
                    console.log('EBNF Command Line Interface (CLI)');
                    console.log('Usage: ebnfcc [options] [script.maia] [--] [arguments]');
                }
            } else {
                console.log('EBNF Command Line Interface (CLI)');
                console.log('Usage: ebnfcc [options] [script.maia] [--] [arguments]');
            }
        }
    }
}

main = new Main();

/*
 * Run EBNF code if this script has been invoked
 * from the command line.
 */
if (typeof process !== 'undefined') {
    // Emulate DOM.
    const jsdom = require("jsdom");
    const {JSDOM} = jsdom;
    var doc = new JSDOM();
    var DOMParser = doc.window.DOMParser;

    main.run();
}