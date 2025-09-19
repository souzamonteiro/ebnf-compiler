// This file was generated on Thu Sep 18, 2025 15:32 (UTC-03) by REx v6.1 which is Copyright (c) 1979-2025 by Gunther Rademacher <grd@gmx.net>
// REx command line: ISO_EBNF_EXT.ebnf -tree -main -javascript

function ISO_EBNF_EXT(string, parsingEventHandler)
{
  init(string, parsingEventHandler);

  var thisParser = this;

  this.ParseException = function(b, e, s, o, x)
  {
    var begin = b;
    var end = e;
    var state = s;
    var offending = o;
    var expected = x;

    this.getBegin = function() {return begin;};
    this.getEnd = function() {return end;};
    this.getState = function() {return state;};
    this.getExpected = function() {return expected;};
    this.getOffending = function() {return offending;};
    this.isAmbiguousInput = function() {return false;};

    this.getMessage = function()
    {
      return offending < 0
           ? "lexical analysis failed"
           : "syntax error";
    };
  };

  function init(source, parsingEventHandler)
  {
    eventHandler = parsingEventHandler;
    input = source;
    size = source.length;
    reset(0, 0, 0);
  }

  this.getInput = function()
  {
    return input;
  };

  this.getTokenOffset = function()
  {
    return b0;
  };

  this.getTokenEnd = function()
  {
    return e0;
  };

  function reset(l, b, e)
  {
            b0 = b; e0 = b;
    l1 = l; b1 = b; e1 = e;
    l2 = 0; b2 = 0; e2 = 0;
    end = e;
    eventHandler.reset(input);
  }

  this.reset = function(l, b, e)
  {
    reset(l, b, e);
  };

  this.getOffendingToken = function(e)
  {
    var o = e.getOffending();
    return o >= 0 ? ISO_EBNF_EXT.TOKEN[o] : null;
  };

  this.getExpectedTokenSet = function(e)
  {
    var expected;
    if (e.getExpected() < 0)
    {
      expected = ISO_EBNF_EXT.getTokenSet(- e.getState());
    }
    else
    {
      expected = [ISO_EBNF_EXT.TOKEN[e.getExpected()]];
    }
    return expected;
  };

  this.getErrorMessage = function(e)
  {
    var message = e.getMessage();
    var found = this.getOffendingToken(e);
    var tokenSet = this.getExpectedTokenSet(e);
    var size = e.getEnd() - e.getBegin();
    message += (found == null ? "" : ", found " + found)
            + "\nwhile expecting "
            + (tokenSet.length == 1 ? tokenSet[0] : ("[" + tokenSet.join(", ") + "]"))
            + "\n"
            + (size == 0 || found != null ? "" : "after successfully scanning " + size + " characters beginning ");
    var prefix = input.substring(0, e.getBegin());
    var lines = prefix.split("\n");
    var line = lines.length;
    var column = lines[line - 1].length + 1;
    return message
         + "at line " + line + ", column " + column + ":\n..."
         + input.substring(e.getBegin(), Math.min(input.length, e.getBegin() + 64))
         + "...";
  };

  this.parse_grammar = function()
  {
    eventHandler.startNonterminal("grammar", e0);
    lookahead1W(6);                 // metaa_identifier | whitespace^token | EOF | '%TOKENS%' | '(*'
    whitespace();
    parse_syntax_definition();
    if (l1 == 8)                    // '%TOKENS%'
    {
      whitespace();
      parse_lexical_definition();
    }
    consume(7);                     // EOF
    eventHandler.endNonterminal("grammar", e0);
  };

  function parse_syntax_definition()
  {
    eventHandler.startNonterminal("syntax_definition", e0);
    for (;;)
    {
      lookahead1W(6);               // metaa_identifier | whitespace^token | EOF | '%TOKENS%' | '(*'
      if (l1 != 1                   // metaa_identifier
       && l1 != 10)                 // '(*'
      {
        break;
      }
      switch (l1)
      {
      case 1:                       // metaa_identifier
        whitespace();
        parse_syntax_rule();
        break;
      default:
        whitespace();
        parse_comment();
      }
    }
    eventHandler.endNonterminal("syntax_definition", e0);
  }

  function parse_syntax_rule()
  {
    eventHandler.startNonterminal("syntax_rule", e0);
    consume(1);                     // metaa_identifier
    lookahead1W(3);                 // whitespace^token | '='
    consume(18);                    // '='
    lookahead1W(8);                 // metaa_identifier | terminal_string | integer | hexadecimal_number |
                                    // whitespace^token | '(' | '[' | '{'
    whitespace();
    parse_definition_list();
    consume(17);                    // ';'
    eventHandler.endNonterminal("syntax_rule", e0);
  }

  function parse_definition_list()
  {
    eventHandler.startNonterminal("definition_list", e0);
    parse_single_definition();
    for (;;)
    {
      if (l1 != 22)                 // '|'
      {
        break;
      }
      consume(22);                  // '|'
      lookahead1W(8);               // metaa_identifier | terminal_string | integer | hexadecimal_number |
                                    // whitespace^token | '(' | '[' | '{'
      whitespace();
      parse_single_definition();
    }
    eventHandler.endNonterminal("definition_list", e0);
  }

  function parse_single_definition()
  {
    eventHandler.startNonterminal("single_definition", e0);
    parse_term();
    for (;;)
    {
      if (l1 != 14)                 // ','
      {
        break;
      }
      consume(14);                  // ','
      lookahead1W(8);               // metaa_identifier | terminal_string | integer | hexadecimal_number |
                                    // whitespace^token | '(' | '[' | '{'
      whitespace();
      parse_term();
    }
    eventHandler.endNonterminal("single_definition", e0);
  }

  function parse_term()
  {
    eventHandler.startNonterminal("term", e0);
    parse_factor();
    for (;;)
    {
      lookahead1W(9);               // whitespace^token | ')' | ',' | '-' | ';' | ']' | '|' | '}'
      if (l1 != 15)                 // '-'
      {
        break;
      }
      consume(15);                  // '-'
      lookahead1W(8);               // metaa_identifier | terminal_string | integer | hexadecimal_number |
                                    // whitespace^token | '(' | '[' | '{'
      whitespace();
      parse_exception();
    }
    eventHandler.endNonterminal("term", e0);
  }

  function parse_exception()
  {
    eventHandler.startNonterminal("exception", e0);
    parse_factor();
    eventHandler.endNonterminal("exception", e0);
  }

  function parse_factor()
  {
    eventHandler.startNonterminal("factor", e0);
    if (l1 == 4)                    // integer
    {
      consume(4);                   // integer
      lookahead1W(1);               // whitespace^token | '*'
      consume(12);                  // '*'
    }
    lookahead1W(7);                 // metaa_identifier | terminal_string | hexadecimal_number | whitespace^token |
                                    // '(' | '[' | '{'
    whitespace();
    parse_primary();
    eventHandler.endNonterminal("factor", e0);
  }

  function parse_primary()
  {
    eventHandler.startNonterminal("primary", e0);
    switch (l1)
    {
    case 1:                         // metaa_identifier
      consume(1);                   // metaa_identifier
      break;
    case 2:                         // terminal_string
      consume(2);                   // terminal_string
      break;
    case 19:                        // '['
      parse_optional_sequence();
      break;
    case 21:                        // '{'
      parse_repeted_sequence();
      break;
    case 9:                         // '('
      parse_grouped_sequence();
      break;
    default:
      parse_character_definition();
    }
    eventHandler.endNonterminal("primary", e0);
  }

  function parse_optional_sequence()
  {
    eventHandler.startNonterminal("optional_sequence", e0);
    consume(19);                    // '['
    lookahead1W(8);                 // metaa_identifier | terminal_string | integer | hexadecimal_number |
                                    // whitespace^token | '(' | '[' | '{'
    whitespace();
    parse_definition_list();
    consume(20);                    // ']'
    eventHandler.endNonterminal("optional_sequence", e0);
  }

  function parse_repeted_sequence()
  {
    eventHandler.startNonterminal("repeted_sequence", e0);
    consume(21);                    // '{'
    lookahead1W(8);                 // metaa_identifier | terminal_string | integer | hexadecimal_number |
                                    // whitespace^token | '(' | '[' | '{'
    whitespace();
    parse_definition_list();
    consume(23);                    // '}'
    eventHandler.endNonterminal("repeted_sequence", e0);
  }

  function parse_grouped_sequence()
  {
    eventHandler.startNonterminal("grouped_sequence", e0);
    consume(9);                     // '('
    lookahead1W(8);                 // metaa_identifier | terminal_string | integer | hexadecimal_number |
                                    // whitespace^token | '(' | '[' | '{'
    whitespace();
    parse_definition_list();
    consume(11);                    // ')'
    eventHandler.endNonterminal("grouped_sequence", e0);
  }

  function parse_character_definition()
  {
    eventHandler.startNonterminal("character_definition", e0);
    switch (l1)
    {
    case 5:                         // hexadecimal_number
      lookahead2W(10);              // whitespace^token | ')' | ',' | '-' | '..' | ';' | ']' | '|' | '}'
      break;
    default:
      lk = l1;
    }
    switch (lk)
    {
    case 517:                       // hexadecimal_number '..'
      parse_character_range();
      break;
    default:
      consume(5);                   // hexadecimal_number
    }
    eventHandler.endNonterminal("character_definition", e0);
  }

  function parse_character_range()
  {
    eventHandler.startNonterminal("character_range", e0);
    consume(5);                     // hexadecimal_number
    lookahead1W(2);                 // whitespace^token | '..'
    consume(16);                    // '..'
    lookahead1W(0);                 // hexadecimal_number | whitespace^token
    consume(5);                     // hexadecimal_number
    eventHandler.endNonterminal("character_range", e0);
  }

  function parse_comment()
  {
    eventHandler.startNonterminal("comment", e0);
    consume(10);                    // '(*'
    for (;;)
    {
      lookahead1W(4);               // character | whitespace^token | '*)'
      if (l1 == 13)                 // '*)'
      {
        break;
      }
      switch (l1)
      {
      case 3:                       // character
        consume(3);                 // character
        break;
      default:
        whitespace();
        parse_whitespace();
      }
    }
    consume(13);                    // '*)'
    eventHandler.endNonterminal("comment", e0);
  }

  function parse_lexical_definition()
  {
    eventHandler.startNonterminal("lexical_definition", e0);
    consume(8);                     // '%TOKENS%'
    for (;;)
    {
      lookahead1W(5);               // metaa_identifier | whitespace^token | EOF | '(*'
      if (l1 == 7)                  // EOF
      {
        break;
      }
      switch (l1)
      {
      case 1:                       // metaa_identifier
        whitespace();
        parse_syntax_rule();
        break;
      default:
        whitespace();
        parse_comment();
      }
    }
    eventHandler.endNonterminal("lexical_definition", e0);
  }

  function parse_whitespace()
  {
    eventHandler.startNonterminal("whitespace", e0);
    consume(6);                     // whitespace^token
    eventHandler.endNonterminal("whitespace", e0);
  }

  function consume(t)
  {
    if (l1 == t)
    {
      whitespace();
      eventHandler.terminal(ISO_EBNF_EXT.TOKEN[l1], b1, e1);
      b0 = b1; e0 = e1; l1 = l2; if (l1 != 0) {
      b1 = b2; e1 = e2; l2 = 0; }
    }
    else
    {
      error(b1, e1, 0, l1, t);
    }
  }

  function whitespace()
  {
    if (e0 != b1)
    {
      eventHandler.whitespace(e0, b1);
      e0 = b1;
    }
  }

  function matchW(tokenSetId)
  {
    var code;
    for (;;)
    {
      code = match(tokenSetId);
      if (code != 6)                // whitespace^token
      {
        break;
      }
    }
    return code;
  }

  function lookahead1W(tokenSetId)
  {
    if (l1 == 0)
    {
      l1 = matchW(tokenSetId);
      b1 = begin;
      e1 = end;
    }
  }

  function lookahead2W(tokenSetId)
  {
    if (l2 == 0)
    {
      l2 = matchW(tokenSetId);
      b2 = begin;
      e2 = end;
    }
    lk = (l2 << 5) | l1;
  }

  function error(b, e, s, l, t)
  {
    throw new thisParser.ParseException(b, e, s, l, t);
  }

  var lk, b0, e0;
  var l1, b1, e1;
  var l2, b2, e2;
  var eventHandler;

  var input;
  var size;

  var begin;
  var end;

  function match(tokenSetId)
  {
    begin = end;
    var current = end;
    var result = ISO_EBNF_EXT.INITIAL[tokenSetId];
    var state = 0;

    for (var code = result & 31; code != 0; )
    {
      var charclass;
      var c0 = current < size ? input.charCodeAt(current) : 0;
      ++current;
      if (c0 < 0x80)
      {
        charclass = ISO_EBNF_EXT.MAP0[c0];
      }
      else if (c0 < 0xd800)
      {
        var c1 = c0 >> 5;
        charclass = ISO_EBNF_EXT.MAP1[(c0 & 31) + ISO_EBNF_EXT.MAP1[(c1 & 31) + ISO_EBNF_EXT.MAP1[c1 >> 5]]];
      }
      else
      {
        if (c0 < 0xdc00)
        {
          var c1 = current < size ? input.charCodeAt(current) : 0;
          if (c1 >= 0xdc00 && c1 < 0xe000)
          {
            ++current;
            c0 = ((c0 & 0x3ff) << 10) + (c1 & 0x3ff) + 0x10000;
          }
        }

        var lo = 0, hi = 0;
        for (var m = 0; ; m = (hi + lo) >> 1)
        {
          if (ISO_EBNF_EXT.MAP2[m] > c0) hi = m - 1;
          else if (ISO_EBNF_EXT.MAP2[1 + m] < c0) lo = m + 1;
          else {charclass = ISO_EBNF_EXT.MAP2[2 + m]; break;}
          if (lo > hi) {charclass = 0; break;}
        }
      }

      state = code;
      var i0 = (charclass << 5) + code - 1;
      code = ISO_EBNF_EXT.TRANSITION[(i0 & 3) + ISO_EBNF_EXT.TRANSITION[i0 >> 2]];

      if (code > 31)
      {
        result = code;
        code &= 31;
        end = current;
      }
    }

    result >>= 5;
    if (result == 0)
    {
      end = current - 1;
      var c1 = end < size ? input.charCodeAt(end) : 0;
      if (c1 >= 0xdc00 && c1 < 0xe000) --end;
      return error(begin, end, state, -1, -1);
    }

    if (end > size) end = size;
    return (result & 31) - 1;
  }

}

ISO_EBNF_EXT.XmlSerializer = function(log, indent)
{
  var input = null;
  var delayedTag = null;
  var hasChildElement = false;
  var depth = 0;

  this.reset = function(string)
  {
    log("<?xml version=\"1.0\" encoding=\"UTF-8\"?" + ">");
    input = string;
    delayedTag = null;
    hasChildElement = false;
    depth = 0;
  };

  this.startNonterminal = function(tag, begin)
  {
    if (delayedTag != null)
    {
      log("<");
      log(delayedTag);
      log(">");
    }
    delayedTag = tag;
    if (indent)
    {
      log("\n");
      for (var i = 0; i < depth; ++i)
      {
        log("  ");
      }
    }
    hasChildElement = false;
    ++depth;
  };

  this.endNonterminal = function(tag, end)
  {
    --depth;
    if (delayedTag != null)
    {
      delayedTag = null;
      log("<");
      log(tag);
      log("/>");
    }
    else
    {
      if (indent)
      {
        if (hasChildElement)
        {
          log("\n");
          for (var i = 0; i < depth; ++i)
          {
            log("  ");
          }
        }
      }
      log("</");
      log(tag);
      log(">");
    }
    hasChildElement = true;
  };

  this.terminal = function(tag, begin, end)
  {
    if (tag.charAt(0) == '\'') tag = "TOKEN";
    this.startNonterminal(tag, begin);
    characters(begin, end);
    this.endNonterminal(tag, end);
  };

  this.whitespace = function(begin, end)
  {
    characters(begin, end);
  };

  function characters(begin, end)
  {
    if (begin < end)
    {
      if (delayedTag != null)
      {
        log("<");
        log(delayedTag);
        log(">");
        delayedTag = null;
      }
      log(input.substring(begin, end)
               .replace(/&/g, "&amp;")
               .replace(/</g, "&lt;")
               .replace(/>/g, "&gt;"));
    }
  }
};

ISO_EBNF_EXT.getTokenSet = function(tokenSetId)
{
  var set = [];
  var s = tokenSetId < 0 ? - tokenSetId : ISO_EBNF_EXT.INITIAL[tokenSetId] & 31;
  for (var i = 0; i < 24; i += 32)
  {
    var j = i;
    var i0 = (i >> 5) * 27 + s - 1;
    var f = ISO_EBNF_EXT.EXPECTED[i0];
    for ( ; f != 0; f >>>= 1, ++j)
    {
      if ((f & 1) != 0)
      {
        set.push(ISO_EBNF_EXT.TOKEN[j]);
      }
    }
  }
  return set;
};

ISO_EBNF_EXT.TopDownTreeBuilder = function()
{
  var input = null;
  var stack = null;

  this.reset = function(i)
  {
    input = i;
    stack = [];
  };

  this.startNonterminal = function(name, begin)
  {
    var nonterminal = new ISO_EBNF_EXT.Nonterminal(name, begin, begin, []);
    if (stack.length > 0) addChild(nonterminal);
    stack.push(nonterminal);
  };

  this.endNonterminal = function(name, end)
  {
    stack[stack.length - 1].end = end;
    if (stack.length > 1) stack.pop();
  };

  this.terminal = function(name, begin, end)
  {
    addChild(new ISO_EBNF_EXT.Terminal(name, begin, end));
  };

  this.whitespace = function(begin, end)
  {
  };

  function addChild(s)
  {
    var current = stack[stack.length - 1];
    current.children.push(s);
  }

  this.serialize = function(e)
  {
    e.reset(input);
    stack[0].send(e);
  };
};

ISO_EBNF_EXT.Terminal = function(name, begin, end)
{
  this.begin = begin;
  this.end = end;

  this.send = function(e)
  {
    e.terminal(name, begin, end);
  };
};

ISO_EBNF_EXT.Nonterminal = function(name, begin, end, children)
{
  this.begin = begin;
  this.end = end;

  this.send = function(e)
  {
    e.startNonterminal(name, begin);
    var pos = begin;
    children.forEach
    (
      function(c)
      {
        if (pos < c.begin) e.whitespace(pos, c.begin);
        c.send(e);
        pos = c.end;
      }
    );
    if (pos < end) e.whitespace(pos, end);
    e.endNonterminal(name, end);
  };
};

ISO_EBNF_EXT.MAP0 =
[
  /*   0 */ 30, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 3, 4,
  /*  36 */ 1, 5, 1, 6, 7, 8, 9, 1, 10, 11, 12, 1, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 1, 14, 1, 15, 1, 1, 1, 16,
  /*  66 */ 16, 16, 16, 17, 16, 18, 18, 18, 18, 19, 18, 18, 20, 21, 18, 18, 18, 22, 23, 18, 18, 18, 18, 18, 18, 24, 1,
  /*  93 */ 25, 1, 26, 1, 16, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
  /* 120 */ 18, 18, 18, 27, 28, 29, 1, 1
];

ISO_EBNF_EXT.MAP1 =
[
  /*   0 */ 54, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
  /*  27 */ 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
  /*  54 */ 90, 136, 198, 167, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104,
  /*  76 */ 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 30, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1,
  /* 102 */ 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1,
  /* 138 */ 3, 4, 1, 5, 1, 6, 7, 8, 9, 1, 10, 11, 12, 1, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 1, 14, 1, 15, 1, 1, 16,
  /* 169 */ 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 27, 28,
  /* 196 */ 29, 1, 1, 16, 16, 16, 16, 17, 16, 18, 18, 18, 18, 19, 18, 18, 20, 21, 18, 18, 18, 22, 23, 18, 18, 18, 18,
  /* 223 */ 18, 18, 24, 1, 25, 1, 26
];

ISO_EBNF_EXT.MAP2 =
[
  /* 0 */ 55296, 65535, 1
];

ISO_EBNF_EXT.INITIAL =
[
  /*  0 */ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
];

ISO_EBNF_EXT.TRANSITION =
[
  /*   0 */ 249, 249, 249, 249, 249, 249, 249, 249, 249, 248, 249, 249, 259, 249, 249, 249, 254, 254, 254, 249, 259,
  /*  21 */ 249, 249, 249, 249, 258, 263, 249, 314, 249, 249, 249, 271, 268, 271, 249, 259, 249, 249, 249, 249, 276,
  /*  42 */ 249, 249, 259, 249, 345, 249, 249, 294, 297, 249, 308, 249, 249, 249, 249, 303, 306, 249, 259, 249, 249,
  /*  63 */ 249, 249, 248, 285, 320, 259, 249, 249, 249, 288, 312, 249, 264, 259, 249, 249, 249, 249, 248, 337, 249,
  /*  84 */ 259, 249, 249, 249, 249, 248, 340, 249, 259, 249, 249, 249, 299, 248, 299, 343, 259, 249, 249, 249, 249,
  /* 105 */ 248, 318, 271, 324, 318, 249, 249, 249, 248, 334, 249, 259, 249, 249, 249, 272, 248, 249, 249, 259, 249,
  /* 126 */ 249, 249, 249, 328, 331, 271, 324, 249, 249, 249, 249, 328, 331, 271, 324, 250, 249, 249, 249, 328, 331,
  /* 147 */ 249, 324, 249, 249, 249, 249, 328, 331, 249, 324, 290, 249, 249, 249, 328, 331, 249, 324, 249, 349, 249,
  /* 168 */ 249, 328, 331, 249, 324, 279, 249, 249, 249, 328, 331, 249, 324, 249, 282, 249, 249, 328, 331, 249, 362,
  /* 189 */ 249, 249, 249, 249, 366, 369, 249, 259, 249, 249, 249, 249, 248, 352, 249, 259, 249, 249, 249, 249, 248,
  /* 210 */ 249, 249, 324, 249, 249, 249, 249, 373, 376, 249, 259, 249, 249, 249, 249, 248, 355, 249, 259, 249, 249,
  /* 231 */ 249, 249, 248, 358, 249, 259, 249, 249, 249, 249, 380, 249, 249, 259, 249, 249, 249, 128, 0, 0, 0, 0, 25,
  /* 254 */ 236, 236, 236, 236, 128, 0, 0, 19, 20, 19, 0, 0, 0, 352, 128, 0, 0, 205, 0, 0, 0, 608, 128, 0, 18, 0, 23, 0,
  /* 282 */ 0, 27, 0, 0, 384, 384, 0, 416, 0, 0, 24, 0, 128, 0, 0, 20, 0, 0, 0, 14, 0, 128, 16, 16, 320, 0, 0, 0, 19,
  /* 311 */ 96, 143, 0, 0, 0, 96, 20, 181, 0, 0, 0, 448, 0, 81, 0, 19, 20, 128, 81, 81, 81, 0, 0, 0, 576, 576, 0, 480,
  /* 339 */ 480, 0, 512, 512, 0, 544, 0, 0, 288, 0, 26, 0, 0, 0, 672, 672, 0, 736, 736, 0, 768, 768, 0, 81, 22, 19, 20,
  /* 366 */ 128, 0, 0, 640, 0, 0, 0, 128, 0, 0, 704, 0, 0, 0, 128, 256, 256, 0
];

ISO_EBNF_EXT.EXPECTED =
[
  /*  0 */ 96, 4160, 65600, 262208, 8264, 1218, 1474, 2622054, 2622070, 13813824, 13879360, 64, 32, 65536, 8192, 1024,
  /* 16 */ 2, 256, 4, 4, 16, 256, 256, 256, 256, 256, 256
];

ISO_EBNF_EXT.TOKEN =
[
  "%ERROR",
  "metaa_identifier",
  "terminal_string",
  "character",
  "integer",
  "hexadecimal_number",
  "whitespace",
  "EOF",
  "'%TOKENS%'",
  "'('",
  "'(*'",
  "')'",
  "'*'",
  "'*)'",
  "','",
  "'-'",
  "'..'",
  "';'",
  "'='",
  "'['",
  "']'",
  "'{'",
  "'|'",
  "'}'"
];

// main program for use with node.js, rhino, or jrunscript

function main(args)
{
  if (typeof process !== "undefined")   // assume node.js
  {
    var command = "node";
    var arguments = process.argv.slice(2);
    var log = function(string) {process.stdout.write(string);};
    var fs = require("fs");
    var readTextFile = fs.readFileSync;
  }
  else                                  // assume rhino or jrunscript
  {
    var arguments = function()
                    {
                      var strings = [];
                      for (var i = 0; i < args.length; ++i)
                      {
                        strings[i] = String(args[i]);
                      }
                      return strings;
                    }();

    if (typeof println == "undefined")  // assume rhino
    {
      var command = "java -jar js.jar";
      var log = function(string) {java.lang.System.out.write(java.lang.String(string).getBytes("utf-8"));};
      var readTextFile = readFile;
    }
    else                                // assume jrunscript
    {
      var command = "jrunscript";
      var log = function(string) {java.lang.System.out.print(string);};
      var readTextFile = function(filename, encoding)
                         {
                           var file = new java.io.File(filename);
                           var buffer = javaByteArray(file.length());
                           new java.io.FileInputStream(file).read(buffer);
                           return String(new java.lang.String(buffer, encoding));
                         };
    }
  }

  function read(input)
  {
    if (/^{.*}$/.test(input))
    {
      return input.substring(1, input.length - 1);
    }
    else
    {
      var content = readTextFile(input, "utf-8");
      return content.length > 0 && content.charCodeAt(0) == 0xFEFF
           ? content.substring(1)
           : content;
    }
  }

  if (arguments.length == 0)
  {
    log("Usage: " + command + " ISO_EBNF_EXT.js [-i] INPUT...\n");
    log("\n");
    log("  parse INPUT, which is either a filename or literal text enclosed in curly braces\n");
    log("\n");
    log("  Option:\n");
    log("    -i     indented parse tree\n");
  }
  else
  {
    var indent = false;
    for (var i = 0; i < arguments.length; ++i)
    {
      if (arguments[i] === "-i")
      {
        indent = true;
        continue;
      }
      var input = read(String(arguments[i]));
      var s = new ISO_EBNF_EXT.XmlSerializer(log, indent);
      var parser = new ISO_EBNF_EXT(input, s);
      try
      {
        parser.parse_grammar();
      }
      catch (pe)
      {
        if (! (pe instanceof parser.ParseException))
        {
          throw pe;
        }
        else
        {
          throw parser.getErrorMessage(pe);
        }
      }
    }
  }
}

main(arguments);

// End
