// This file was generated on Wed Sep 17, 2025 16:30 (UTC-03) by REx v6.1 which is Copyright (c) 1979-2025 by Gunther Rademacher <grd@gmx.net>
// REx command line: ISO_EBNF_EXT.ebnf -cpp -tree -main

#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
  #include <fcntl.h>
  #include <io.h>
#endif
#include <vector>
#include <stdio.h>
#include <string>
#include <algorithm>

class ISO_EBNF_EXT
{
public:
  class EventHandler;

  ISO_EBNF_EXT(const wchar_t *string, EventHandler *t)
  {
    initialize(string, t);
  }

  virtual ~ISO_EBNF_EXT()
  {
  }

  class EventHandler
  {
  public:
    virtual ~EventHandler() {}

    virtual void reset(const wchar_t *string) = 0;
    virtual void startNonterminal(const wchar_t *name, int begin) = 0;
    virtual void endNonterminal(const wchar_t *name, int end) = 0;
    virtual void terminal(const wchar_t *name, int begin, int end) = 0;
    virtual void whitespace(int begin, int end) = 0;
  };

  class XmlSerializer : public EventHandler
  {
  public:
    XmlSerializer(bool indent)
    : input(0)
    , delayedTag(0)
    , indent(indent)
    , hasChildElement(false)
    , depth(0)
    {
    }

    void reset(const wchar_t *input)
    {
      fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>", stdout);

      this->input = input;
      delayedTag = 0;
      hasChildElement = false;
      depth = 0;
    }

    void startNonterminal(const wchar_t *tag, int b)
    {
      if (delayedTag != 0)
      {
        fputc('<', stdout);
        fputs(Utf8Encoder::encode(delayedTag).c_str(), stdout);
        fputc('>', stdout);
      }
      delayedTag = tag;
      if (indent)
      {
        fputc('\n', stdout);
        for (int i = 0; i < depth; ++i)
        {
          fputs("  ", stdout);
        }
      }
      hasChildElement = false;
      ++depth;
    }

    void endNonterminal(const wchar_t *tag, int e)
    {
      --depth;
      if (delayedTag != 0)
      {
        delayedTag = 0;
        fputc('<', stdout);
        fputs(Utf8Encoder::encode(tag).c_str(), stdout);
        fputs("/>", stdout);
      }
      else
      {
        if (indent)
        {
          if (hasChildElement)
          {
            fputc('\n', stdout);
            for (int i = 0; i < depth; ++i)
            {
              fputs("  ", stdout);
            }
          }
        }
        fputs("</", stdout);
        fputs(Utf8Encoder::encode(tag).c_str(), stdout);
        fputc('>', stdout);
      }
      hasChildElement = true;
    }

    void whitespace(int b, int e)
    {
      characters(b, e);
    }

    void characters(int b, int e)
    {
      if (b < e)
      {
        if (delayedTag != 0)
        {
          fputc('<', stdout);
          fputs(Utf8Encoder::encode(delayedTag).c_str(), stdout);
          fputc('>', stdout);
          delayedTag = 0;
        }
        std::string encoded = Utf8Encoder::encode(input + b, e - b);
        int size = encoded.size();
        for (int i = 0; i < size; ++i)
        {
          char c = encoded[i];
          switch (c)
          {
          case 0: break;
          case L'&': fputs("&amp;", stdout); break;
          case L'<': fputs("&lt;", stdout); break;
          case L'>': fputs("&gt;", stdout); break;
          default: fputc(c, stdout);
          }
        }
      }
    }

    void terminal(const wchar_t *tag, int b, int e)
    {
      if (tag[0] == L'\'') tag = L"TOKEN";
      startNonterminal(tag, b);
      characters(b, e);
      endNonterminal(tag, e);
    }

  private:
    const wchar_t *input;
    const wchar_t *delayedTag;
    bool indent;
    bool hasChildElement;
    int depth;
  };

  class Symbol
  {
  public:
    virtual ~Symbol() {}

    const wchar_t *name;
    int begin;
    int end;

    virtual void send(EventHandler *e) = 0;

  protected:
    Symbol(const wchar_t *name, int begin, int end)
    {
      this->name = name;
      this->begin = begin;
      this->end = end;
    }
  };

  class Terminal : public Symbol
  {
  public:
    Terminal(const wchar_t *name, int begin, int end)
    : Symbol(name, begin, end)
    {}

    void send(EventHandler *e)
    {
      e->terminal(name, begin, end);
    }
  };

  class Nonterminal : public Symbol
  {
  public:
    std::vector<Symbol *> *children;

    Nonterminal(const wchar_t *name, int begin, int end, std::vector<Symbol *> *children)
    : Symbol(name, begin, end)
    {
      this->children = children;
    }

    ~Nonterminal()
    {
      for (std::vector<Symbol *>::iterator child = children->begin(); child != children->end(); ++child)
        delete *child;
      delete children;
    }

    void send(EventHandler *e)
    {
      e->startNonterminal(name, begin);
      int pos = begin;
      for (std::vector<Symbol *>::iterator i = children->begin(); i != children->end(); ++i)
      {
        Symbol *c = *i;
        if (pos < c->begin) e->whitespace(pos, c->begin);
        c->send(e);
        pos = c->end;
      }
      if (pos < end) e->whitespace(pos, end);
      e->endNonterminal(name, end);
    }
  };

  class TopDownTreeBuilder : public EventHandler
  {
  public:
    TopDownTreeBuilder()
    {
      input = 0;
      stack.clear();
      top = -1;
    }

    void reset(const wchar_t *input)
    {
      this->input = input;
      top = -1;
    }

    void startNonterminal(const wchar_t *name, int begin)
    {
      Nonterminal *nonterminal = new Nonterminal(name, begin, begin, new std::vector<Symbol *>());
      if (top++ >= 0) addChild(nonterminal);
      if ((size_t) top >= stack.size())
        stack.resize(stack.size() == 0 ? 64 : stack.size() << 1);
      stack[top] = nonterminal;
    }

    void endNonterminal(const wchar_t *name, int end)
    {
      stack[top]->end = end;
      if (top > 0) --top;
    }

    void terminal(const wchar_t *name, int begin, int end)
    {
      addChild(new Terminal(name, begin, end));
    }

    void whitespace(int begin, int end)
    {
    }

    void serialize(EventHandler *e)
    {
      e->reset(input);
      stack[0]->send(e);
    }

  private:
    void addChild(Symbol *s)
    {
      Nonterminal *current = stack[top];
      current->children->push_back(s);
    }

    const wchar_t *input;
    std::vector<Nonterminal *> stack;
    int top;
  };

  static int main(int argc, char **argv)
  {
    int returnCode = 0;

    if (argc < 2)
    {
      fprintf(stderr, "Usage: %s [-i] INPUT...\n", argv[0]);
      fprintf(stderr, "\n");
      fprintf(stderr, "  parse INPUT, which is either a filename or literal text enclosed in curly braces\n");
      fprintf(stderr, "\n");
      fprintf(stderr, "  Option:\n");
      fprintf(stderr, "    -i     indented parse tree\n");
    }
    else
    {
#ifdef _WIN32
      _setmode(1, O_BINARY);
#endif

      bool indent = false;
      for (int i = 1; i < argc; ++i)
      {
        if (strcmp(argv[i], "-i") == 0)
        {
          indent = true;
          continue;
        }
        try
        {
          XmlSerializer s(indent);
          std::wstring input = read(argv[i]);
          ISO_EBNF_EXT parser(input.c_str(), &s);
          try
          {
            parser.parse_grammar();
          }
          catch (ParseException &pe)
          {
            fprintf(stderr, "\n");
            fprintf(stderr, "%s\n", Utf8Encoder::encode(parser.getErrorMessage(pe).c_str()).c_str());
            returnCode = 1;
            break;
          }
        }
        catch (FileNotFound &fnf)
        {
          fprintf(stderr, "error: file not found: %s\n", fnf.getFilename().c_str());
          returnCode = 1;
          break;
        }
        catch (MalformedInputException &mie)
        {
          fprintf(stderr, "error: UTF-8 decoding error in %s at offset %d\n",
            argv[i], static_cast<int>(mie.getOffset()));
          returnCode = 1;
          break;
        }
      }
    }
    return returnCode;
  }

  class ParseException
  {
  private:
    int begin, end, offending, expected, state;
    friend class ISO_EBNF_EXT;

  protected:
    ParseException(int b, int e, int s, int o, int x)
    : begin(b), end(e), offending(o), expected(x), state(s)
    {
    }

  public:
    const wchar_t *getMessage() const
    {
      return offending < 0
           ? L"lexical analysis failed"
           : L"syntax error";
    }

    int getBegin() const {return begin;}
    int getEnd() const {return end;}
    int getState() const {return state;}
    int getOffending() const {return offending;}
    int getExpected() const {return expected;}
  };

  void initialize(const wchar_t *source, EventHandler *parsingEventHandler)
  {
    eventHandler = parsingEventHandler;
    input = source;
    reset(0, 0, 0);
  }

  const wchar_t *getInput() const
  {
    return input;
  }

  int getTokenOffset() const
  {
    return b0;
  }

  int getTokenEnd() const
  {
    return e0;
  }

  void reset(int l, int b, int e)
  {
            b0 = b; e0 = b;
    l1 = l; b1 = b; e1 = e;
    end = e;
    eventHandler->reset(input);
  }

  void reset()
  {
    reset(0, 0, 0);
  }

  static const wchar_t *getOffendingToken(ParseException e)
  {
    return e.getOffending() < 0 ? 0 : TOKEN[e.getOffending()];
  }

  static void getExpectedTokenSet(const ParseException &e, const wchar_t **set, int size)
  {
    if (e.expected < 0)
    {
      getTokenSet(- e.state, set, size);
    }
    else if (size == 1)
    {
      set[0] = 0;
    }
    else if (size > 1)
    {
      set[0] = TOKEN[e.expected];
      set[1] = 0;
    }
  }

  static std::wstring to_wstring(int i)
  {
    const wchar_t *sign = i < 0 ? L"-" : L"";
    std::wstring a;
    do
    {
      a += L'0' + abs(i % 10);
      i /= 10;
    }
    while (i != 0);
    a += sign;
    std::reverse(a.begin(), a.end());
    return a;
  }

  std::wstring getErrorMessage(const ParseException &e)
  {
    std::wstring message(e.getMessage());
    const wchar_t *found = getOffendingToken(e);
    if (found != 0)
    {
      message += L", found ";
      message += found;
    }
    const wchar_t *expected[64];
    getExpectedTokenSet(e, expected, sizeof expected / sizeof *expected);
    message += L"\nwhile expecting ";
    const wchar_t *delimiter(expected[1] ? L"[" : L"");
    for (const wchar_t **x = expected; *x; ++x)
    {
      message += delimiter;
      message += *x;
      delimiter = L", ";
    }
    message += expected[1] ? L"]\n" : L"\n";
    int size = e.getEnd() - e.getBegin();
    if (size != 0 && found == 0)
    {
      message += L"after successfully scanning ";
      message += to_wstring(size);
      message += L" characters beginning ";
    }
    int line = 1;
    int column = 1;
    for (int i = 0; i < e.getBegin(); ++i)
    {
      if (input[i] == L'\n')
      {
        ++line;
        column = 1;
      }
      else
      {
        ++column;
      }
    }
    message += L"at line ";
    message += to_wstring(line);
    message += L", column ";
    message += to_wstring(column);
    message += L":\n...";
    const wchar_t *w = input + e.getBegin();
    for (int i = 0; i < 64 && *w; ++i)
    {
      message += *w++;
    }
    message += L"...";
    return message;
  }

  void parse_grammar()
  {
    eventHandler->startNonterminal(L"grammar", e0);
    lookahead1W(6);                 // identifier | whitespace^token | EOF | '%TOKENS%' | '(*'
    whitespace();
    parse_syntax_definition();
    if (l1 == 8)                    // '%TOKENS%'
    {
      whitespace();
      parse_lexical_definition();
    }
    consume(7);                     // EOF
    eventHandler->endNonterminal(L"grammar", e0);
  }

private:

  void parse_syntax_definition()
  {
    eventHandler->startNonterminal(L"syntax_definition", e0);
    for (;;)
    {
      lookahead1W(6);               // identifier | whitespace^token | EOF | '%TOKENS%' | '(*'
      if (l1 != 1                   // identifier
       && l1 != 10)                 // '(*'
      {
        break;
      }
      switch (l1)
      {
      case 1:                       // identifier
        whitespace();
        parse_rule();
        break;
      default:
        whitespace();
        parse_comment();
        break;
      }
    }
    eventHandler->endNonterminal(L"syntax_definition", e0);
  }

  void parse_rule()
  {
    eventHandler->startNonterminal(L"rule", e0);
    consume(1);                     // identifier
    lookahead1W(3);                 // whitespace^token | '='
    consume(17);                    // '='
    lookahead1W(9);                 // identifier | terminal | integer | hex_number | whitespace^token | '(' | '[' | '{'
    whitespace();
    parse_expression();
    consume(16);                    // ';'
    eventHandler->endNonterminal(L"rule", e0);
  }

  void parse_expression()
  {
    eventHandler->startNonterminal(L"expression", e0);
    parse_term();
    for (;;)
    {
      if (l1 != 21)                 // '|'
      {
        break;
      }
      consume(21);                  // '|'
      lookahead1W(9);               // identifier | terminal | integer | hex_number | whitespace^token | '(' | '[' | '{'
      whitespace();
      parse_term();
    }
    eventHandler->endNonterminal(L"expression", e0);
  }

  void parse_term()
  {
    eventHandler->startNonterminal(L"term", e0);
    parse_factor();
    for (;;)
    {
      lookahead1W(8);               // whitespace^token | ')' | ',' | ';' | ']' | '|' | '}'
      if (l1 != 14)                 // ','
      {
        break;
      }
      consume(14);                  // ','
      lookahead1W(9);               // identifier | terminal | integer | hex_number | whitespace^token | '(' | '[' | '{'
      whitespace();
      parse_factor();
    }
    eventHandler->endNonterminal(L"term", e0);
  }

  void parse_factor()
  {
    eventHandler->startNonterminal(L"factor", e0);
    if (l1 == 4)                    // integer
    {
      consume(4);                   // integer
      lookahead1W(1);               // whitespace^token | '*'
      consume(12);                  // '*'
    }
    lookahead1W(7);                 // identifier | terminal | hex_number | whitespace^token | '(' | '[' | '{'
    whitespace();
    parse_primary();
    eventHandler->endNonterminal(L"factor", e0);
  }

  void parse_primary()
  {
    eventHandler->startNonterminal(L"primary", e0);
    switch (l1)
    {
    case 1:                         // identifier
      consume(1);                   // identifier
      break;
    case 2:                         // terminal
      consume(2);                   // terminal
      break;
    case 18:                        // '['
      parse_optional();
      break;
    case 20:                        // '{'
      parse_repetition();
      break;
    case 9:                         // '('
      parse_group();
      break;
    default:
      parse_char_range();
      break;
    }
    eventHandler->endNonterminal(L"primary", e0);
  }

  void parse_optional()
  {
    eventHandler->startNonterminal(L"optional", e0);
    consume(18);                    // '['
    lookahead1W(9);                 // identifier | terminal | integer | hex_number | whitespace^token | '(' | '[' | '{'
    whitespace();
    parse_expression();
    consume(19);                    // ']'
    eventHandler->endNonterminal(L"optional", e0);
  }

  void parse_repetition()
  {
    eventHandler->startNonterminal(L"repetition", e0);
    consume(20);                    // '{'
    lookahead1W(9);                 // identifier | terminal | integer | hex_number | whitespace^token | '(' | '[' | '{'
    whitespace();
    parse_expression();
    consume(22);                    // '}'
    eventHandler->endNonterminal(L"repetition", e0);
  }

  void parse_group()
  {
    eventHandler->startNonterminal(L"group", e0);
    consume(9);                     // '('
    lookahead1W(9);                 // identifier | terminal | integer | hex_number | whitespace^token | '(' | '[' | '{'
    whitespace();
    parse_expression();
    consume(11);                    // ')'
    eventHandler->endNonterminal(L"group", e0);
  }

  void parse_char_range()
  {
    eventHandler->startNonterminal(L"char_range", e0);
    consume(5);                     // hex_number
    lookahead1W(2);                 // whitespace^token | '..'
    consume(15);                    // '..'
    lookahead1W(0);                 // hex_number | whitespace^token
    consume(5);                     // hex_number
    eventHandler->endNonterminal(L"char_range", e0);
  }

  void parse_comment()
  {
    eventHandler->startNonterminal(L"comment", e0);
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
        break;
      }
    }
    consume(13);                    // '*)'
    eventHandler->endNonterminal(L"comment", e0);
  }

  void parse_lexical_definition()
  {
    eventHandler->startNonterminal(L"lexical_definition", e0);
    consume(8);                     // '%TOKENS%'
    for (;;)
    {
      lookahead1W(5);               // identifier | whitespace^token | EOF | '(*'
      if (l1 == 7)                  // EOF
      {
        break;
      }
      switch (l1)
      {
      case 1:                       // identifier
        whitespace();
        parse_rule();
        break;
      default:
        whitespace();
        parse_comment();
        break;
      }
    }
    eventHandler->endNonterminal(L"lexical_definition", e0);
  }

  void parse_whitespace()
  {
    eventHandler->startNonterminal(L"whitespace", e0);
    consume(6);                     // whitespace^token
    eventHandler->endNonterminal(L"whitespace", e0);
  }

  void consume(int t)
  {
    if (l1 == t)
    {
      whitespace();
      eventHandler->terminal(TOKEN[l1], b1, e1);
      b0 = b1; e0 = e1; l1 = 0;
    }
    else
    {
      error(b1, e1, 0, l1, t);
    }
  }

  void whitespace()
  {
    if (e0 != b1)
    {
      eventHandler->whitespace(e0, b1);
      e0 = b1;
    }
  }

  int matchW(int tokenSetId)
  {
    int code;
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

  void lookahead1W(int tokenSetId)
  {
    if (l1 == 0)
    {
      l1 = matchW(tokenSetId);
      b1 = begin;
      e1 = end;
    }
  }

  int error(int b, int e, int s, int l, int t)
  {
    throw ParseException(b, e, s, l, t);
  }

  int     b0, e0;
  int l1, b1, e1;
  EventHandler *eventHandler;

  const wchar_t *input;
  int begin;
  int end;

  int match(int tokenSetId)
  {
    begin = end;
    int current = end;
    int result = INITIAL[tokenSetId];
    int state = 0;

    for (int code = result & 31; code != 0; )
    {
      int charclass;
      int c0 = input[current];
      ++current;
      if (c0 < 0x80)
      {
        charclass = MAP0[c0];
      }
      else if (c0 < 0xd800)
      {
        int c1 = c0 >> 5;
        charclass = MAP1[(c0 & 31) + MAP1[(c1 & 31) + MAP1[c1 >> 5]]];
      }
      else
      {
        if (c0 < 0xdc00)
        {
          int c1 = input[current];
          if (c1 >= 0xdc00 && c1 < 0xe000)
          {
            ++current;
            c0 = ((c0 & 0x3ff) << 10) + (c1 & 0x3ff) + 0x10000;
          }
        }
        int lo = 0, hi = 0;
        for (int m = 0; ; m = (hi + lo) >> 1)
        {
          if (MAP2[m] > c0) hi = m - 1;
          else if (MAP2[1 + m] < c0) lo = m + 1;
          else {charclass = MAP2[2 + m]; break;}
          if (lo > hi) {charclass = 0; break;}
        }
      }

      state = code;
      int i0 = (charclass << 5) + code - 1;
      code = TRANSITION[(i0 & 3) + TRANSITION[i0 >> 2]];
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
      int c1 = input[end];
      if (c1 >= 0xdc00 && c1 < 0xe000) --end;
      return error(begin, end, state, -1, -1);
    }

    if (input[begin] == 0) end = begin;
    return (result & 31) - 1;
  }

  class FileNotFound
  {
  public:
    FileNotFound(std::string name) : filename(name) {}
    const std::string &getFilename() const {return filename;}

  private:
    std::string filename;
  };

  class MalformedInputException
  {
  public:
    MalformedInputException(size_t offset) : offset(offset) {}
    size_t getOffset() const {return offset;}

  private:
    size_t offset;
  };

  class Utf8Encoder
  {
  public:
    static std::string encode(const wchar_t *unencoded)
    {
      return encode(unencoded, wcslen(unencoded));
    }

    static std::string encode(const wchar_t *unencoded, size_t size)
    {
      std::string encoded;
      encoded.reserve(size + 3);

      for (size_t i = 0; i < size; ++i)
      {
        if (encoded.size() + 4 >= encoded.capacity()) encoded.reserve(encoded.capacity() * 2);

        int w = unencoded[i];
        if (w < 0x80)
        {
          encoded += w;
        }
        else if (w < 0x800)
        {
          encoded += 0xc0 | (w >> 6);
          encoded += 0x80 | (w & 0x3f);
        }
        else if (w < 0xd800)
        {
          encoded += 0xe0 | ( w          >> 12);
          encoded += 0x80 | ((w & 0xfff) >>  6);
          encoded += 0x80 | ( w &  0x3f       );
        }
        else if (w < 0xe000)
        {
          if (++i >= size)
          {
            throw MalformedInputException(i - 1);
          }
          int w2 = unencoded[i];
          if (w2 < 0xdc00 || w2 > 0xdfff)
          {
            throw MalformedInputException(i - 1);
          }
          w = (((w  & 0x3ff) << 10) | (w2 & 0x3ff)) + 0x10000;
          encoded += 0xf0 | ( w            >> 18);
          encoded += 0x80 | ((w & 0x3ffff) >> 12);
          encoded += 0x80 | ((w &   0xfff) >>  6);
          encoded += 0x80 | ( w &    0x3f       );
        }
        else if (w < 0x10000)
        {
          encoded += 0xe0 | ( w          >> 12);
          encoded += 0x80 | ((w & 0xfff) >>  6);
          encoded += 0x80 | ( w &  0x3f       );
        }
        else if (w < 0x110000)
        {
          encoded += 0xf0 | ( w            >> 18);
          encoded += 0x80 | ((w & 0x3ffff) >> 12);
          encoded += 0x80 | ((w &   0xfff) >>  6);
          encoded += 0x80 | ( w &    0x3f       );
        }
        else
        {
          throw MalformedInputException(i);
        }
      }
      return encoded;
    }
  };

  class Utf8Decoder
  {
  public:
    static std::wstring decode(const char *string)
    {
      return decode(string, strlen(string));
    }

    static std::wstring decode(const char *string, size_t size)
    {
      std::wstring decoded;
      decoded.reserve(size + 1);

      for (size_t consumed = 0; consumed < size; )
      {
        if (decoded.size() + 2 >= decoded.capacity()) decoded.reserve(decoded.capacity() * 2);

        size_t bytes;
        int codepoint = decodeChar(string + consumed, &bytes);

        if (bytes == 0)
        {
          throw MalformedInputException(consumed);
        }

        consumed += bytes;

        if (codepoint < 0x10000)
        {
          decoded += codepoint;
        }
        else
        {
          codepoint -= 0x10000;
          decoded += 0x0d800 | (codepoint >> 10);
          decoded += 0x0dc00 | (codepoint & 0x3ff);
        }
      }

      return decoded;
    }

  private:
    static int decodeChar(const char *input, size_t *size)
    {
      int codepoint = input[0];
      if ((codepoint & 0x80) == 0)
      {
        *size = 1;
      }
      else if (   (codepoint & 0x60) == 0x40
               && (input[1]  & 0xc0) == 0x80)
      {
        codepoint = ((codepoint & 0x1f) << 6)
                  |  (input[1]  & 0x3f);
        *size = codepoint < 0x80 ? 0 : 2;
      }
      else if (   (codepoint & 0x70) == 0x60
               && (input[1]  & 0xc0) == 0x80
               && (input[2]  & 0xc0) == 0x80)
      {
        codepoint = ((codepoint &  0xf) << 12)
                  | ((input[1]  & 0x3f) <<  6)
                  |  (input[2]  & 0x3f);
        *size = codepoint < 0x800 ? 0 : 3;
      }
      else if (   (codepoint & 0x78) == 0x70
               && (input[1]  & 0xc0) == 0x80
               && (input[2]  & 0xc0) == 0x80
               && (input[3]  & 0xc0) == 0x80)
      {
        codepoint  = ((codepoint &  0x7) << 18)
                   | ((input[1]  & 0x3f) << 12)
                   | ((input[2]  & 0x3f) <<  6)
                   | ( input[3]  & 0x3f       );
        *size = codepoint < 0x10000 || codepoint > 0x10ffff ? 0 : 4;
      }
      else
      {
        *size = 0;
      }
      return codepoint;
    }
  };

  static std::wstring read(const char *input)
  {
    size_t l = strlen(input);
    if (l > 0 && input[0] == '{' && input[l - 1] == '}')
    {
      return Utf8Decoder::decode(input + 1, l - 2);
    }
    else
    {
      FILE *file = fopen(input, "rb");
      if (file == 0)
      {
        throw FileNotFound(std::string(input));
      }

      std::string content;
      content.reserve(4096);

      for (int c = getc(file); c != EOF; c = getc(file))
      {
        if (content.size() + 1 >= content.capacity()) content.reserve(content.capacity() * 2);
        content += c;
      }

      fclose(file);

      if (content.size() >= 3
       && (unsigned char) content[0] == 0xef
       && (unsigned char) content[1] == 0xbb
       && (unsigned char) content[2] == 0xbf)
      {
        content.erase(0, 3);
      }

      return Utf8Decoder::decode(content.c_str());
    }
  }

  static void getTokenSet(int tokenSetId, const wchar_t **set, int size)
  {
    int s = tokenSetId < 0 ? - tokenSetId : INITIAL[tokenSetId] & 31;
    for (int i = 0; i < 23; i += 32)
    {
      int j = i;
      for (unsigned int f = ec(i >> 5, s); f != 0; f >>= 1, ++j)
      {
        if ((f & 1) != 0)
        {
          if (size > 1)
          {
            set[0] = TOKEN[j];
            ++set;
            --size;
          }
        }
      }
    }
    if (size > 0)
    {
      set[0] = 0;
    }
  }

  static int ec(int t, int s)
  {
    int i0 = t * 26 + s - 1;
    return EXPECTED[i0];
  }

  static const int MAP0[];
  static const int MAP1[];
  static const int MAP2[];
  static const int INITIAL[];
  static const int TRANSITION[];
  static const int EXPECTED[];
  static const wchar_t *TOKEN[];
};

const int ISO_EBNF_EXT::MAP0[] =
{
/*   0 */ 29, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 3, 4,
/*  36 */ 1, 5, 1, 6, 7, 8, 9, 1, 10, 1, 11, 1, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 1, 13, 1, 14, 1, 1, 1, 15, 15,
/*  67 */ 15, 15, 16, 15, 17, 17, 17, 17, 18, 17, 17, 19, 20, 17, 17, 17, 21, 22, 17, 17, 17, 17, 17, 17, 23, 1, 24, 1,
/*  95 */ 25, 1, 15, 15, 15, 15, 15, 15, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
/* 123 */ 26, 27, 28, 1, 1
};

const int ISO_EBNF_EXT::MAP1[] =
{
/*   0 */ 54, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
/*  27 */ 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
/*  54 */ 90, 136, 198, 167, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104,
/*  76 */ 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 29, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1,
/* 103 */ 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 3, 4,
/* 140 */ 1, 5, 1, 6, 7, 8, 9, 1, 10, 1, 11, 1, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 1, 13, 1, 14, 1, 1, 15, 15, 15,
/* 171 */ 15, 15, 15, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 26, 27, 28, 1, 1,
/* 199 */ 15, 15, 15, 15, 16, 15, 17, 17, 17, 17, 18, 17, 17, 19, 20, 17, 17, 17, 21, 22, 17, 17, 17, 17, 17, 17, 23, 1,
/* 227 */ 24, 1, 25
};

const int ISO_EBNF_EXT::MAP2[] =
{
/* 0 */ 55296, 65535, 1
};

const int ISO_EBNF_EXT::INITIAL[] =
{
/*  0 */ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};

const int ISO_EBNF_EXT::TRANSITION[] =
{
/*   0 */ 241, 241, 241, 241, 241, 241, 241, 241, 241, 240, 241, 241, 309, 241, 241, 241, 246, 246, 247, 241, 309, 241,
/*  22 */ 241, 241, 241, 251, 253, 241, 306, 241, 241, 241, 262, 259, 261, 241, 309, 241, 241, 241, 241, 267, 241, 241,
/*  44 */ 309, 241, 322, 241, 241, 283, 285, 241, 270, 241, 241, 241, 241, 291, 303, 241, 309, 241, 241, 241, 241, 240,
/*  66 */ 295, 331, 309, 241, 241, 241, 319, 300, 241, 287, 309, 241, 241, 241, 241, 240, 316, 241, 309, 241, 241, 241,
/*  88 */ 255, 240, 241, 328, 309, 241, 241, 241, 241, 240, 334, 263, 312, 241, 241, 241, 241, 240, 338, 241, 309, 241,
/* 110 */ 241, 241, 296, 240, 241, 241, 309, 241, 241, 241, 241, 342, 279, 263, 309, 241, 241, 241, 241, 342, 279, 263,
/* 132 */ 309, 324, 241, 241, 241, 342, 276, 263, 309, 241, 241, 241, 241, 342, 276, 263, 309, 273, 241, 241, 241, 342,
/* 154 */ 276, 263, 309, 242, 241, 241, 241, 342, 276, 263, 309, 346, 241, 241, 241, 342, 276, 263, 309, 241, 350, 241,
/* 176 */ 241, 342, 276, 263, 354, 241, 241, 241, 241, 358, 360, 241, 309, 241, 241, 241, 241, 240, 364, 241, 309, 241,
/* 198 */ 241, 241, 241, 240, 241, 263, 309, 241, 241, 241, 241, 368, 370, 241, 309, 241, 241, 241, 241, 240, 374, 241,
/* 220 */ 309, 241, 241, 241, 241, 240, 378, 241, 309, 241, 241, 241, 241, 382, 241, 241, 309, 241, 241, 241, 128, 0, 0,
/* 243 */ 0, 0, 25, 235, 235, 235, 235, 0, 128, 0, 0, 18, 0, 0, 13, 0, 128, 0, 0, 204, 0, 0, 0, 80, 128, 0, 17, 0, 18,
/* 272 */ 96, 0, 23, 0, 0, 80, 0, 0, 80, 0, 204, 128, 0, 0, 19, 0, 0, 352, 0, 128, 15, 15, 320, 384, 0, 0, 0, 576, 142,
/* 301 */ 0, 0, 0, 320, 0, 0, 96, 19, 0, 18, 19, 0, 18, 19, 180, 480, 0, 0, 0, 416, 0, 0, 288, 0, 0, 24, 0, 512, 0, 0,
/* 331 */ 0, 448, 0, 0, 180, 0, 204, 544, 0, 0, 0, 128, 80, 80, 80, 22, 0, 0, 0, 26, 0, 0, 0, 21, 18, 19, 0, 128, 0, 0,
/* 361 */ 608, 0, 0, 640, 0, 0, 0, 128, 0, 0, 672, 0, 0, 704, 0, 0, 0, 736, 0, 0, 0, 128, 256, 256, 0
};

const int ISO_EBNF_EXT::EXPECTED[] =
{
/*  0 */ 96, 4160, 32832, 131136, 8264, 1218, 1474, 1311334, 6899776, 1311350, 64, 32, 32768, 8192, 1024, 2, 256, 4, 4,
/* 19 */ 16, 256, 256, 256, 256, 256, 256
};

const wchar_t *ISO_EBNF_EXT::TOKEN[] =
{
  L"%ERROR",
  L"identifier",
  L"terminal",
  L"character",
  L"integer",
  L"hex_number",
  L"whitespace",
  L"EOF",
  L"'%TOKENS%'",
  L"'('",
  L"'(*'",
  L"')'",
  L"'*'",
  L"'*)'",
  L"','",
  L"'..'",
  L"';'",
  L"'='",
  L"'['",
  L"']'",
  L"'{'",
  L"'|'",
  L"'}'"
};

int main(int argc, char **argv)
{
  return ISO_EBNF_EXT::main(argc, argv);
}

// End
