// This file was generated on Wed Sep 11, 2024 14:23 (UTC-03) by REx v5.57 which is Copyright (c) 1979-2023 by Gunther Rademacher <grd@gmx.net>
// REx command line: REx.ebnf -tree -main -cpp

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

class REx
{
public:
  class EventHandler;

  REx(const wchar_t *string, EventHandler *t)
  {
    initialize(string, t);
  }

  virtual ~REx()
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
          REx parser(input.c_str(), &s);
          try
          {
            parser.parse_Grammar();
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
    friend class REx;

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
    l2 = 0; b2 = 0; e2 = 0;
    l3 = 0; b3 = 0; e3 = 0;
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

  void parse_Grammar()
  {
    eventHandler->startNonterminal(L"Grammar", e0);
    lookahead1W(11);                // Whitespace | Name | '<?'
    whitespace();
    parse_Prolog();
    whitespace();
    parse_SyntaxDefinition();
    if (l1 == 29)                   // '<?TOKENS?>'
    {
      whitespace();
      parse_LexicalDefinition();
    }
    if (l1 == 28)                   // '<?ENCORE?>'
    {
      whitespace();
      parse_Encore();
    }
    consume(11);                    // EOF
    eventHandler->endNonterminal(L"Grammar", e0);
  }

private:

  void parse_Prolog()
  {
    eventHandler->startNonterminal(L"Prolog", e0);
    for (;;)
    {
      lookahead1W(11);              // Whitespace | Name | '<?'
      if (l1 != 27)                 // '<?'
      {
        break;
      }
      whitespace();
      parse_ProcessingInstruction();
    }
    eventHandler->endNonterminal(L"Prolog", e0);
  }

  void parse_ProcessingInstruction()
  {
    eventHandler->startNonterminal(L"ProcessingInstruction", e0);
    consume(27);                    // '<?'
    lookahead1(0);                  // Name
    consume(2);                     // Name
    lookahead1(8);                  // Space | '?>'
    if (l1 == 3)                    // Space
    {
      for (;;)
      {
        consume(3);                 // Space
        lookahead1(17);             // Space | DirPIContents | '?>'
        if (l1 != 3)                // Space
        {
          break;
        }
      }
      if (l1 == 4)                  // DirPIContents
      {
        consume(4);                 // DirPIContents
      }
    }
    lookahead1(2);                  // '?>'
    consume(33);                    // '?>'
    eventHandler->endNonterminal(L"ProcessingInstruction", e0);
  }

  void parse_SyntaxDefinition()
  {
    eventHandler->startNonterminal(L"SyntaxDefinition", e0);
    for (;;)
    {
      whitespace();
      parse_SyntaxProduction();
      if (l1 != 2)                  // Name
      {
        break;
      }
    }
    eventHandler->endNonterminal(L"SyntaxDefinition", e0);
  }

  void parse_SyntaxProduction()
  {
    eventHandler->startNonterminal(L"SyntaxProduction", e0);
    consume(2);                     // Name
    lookahead1W(4);                 // Whitespace | '::='
    consume(25);                    // '::='
    lookahead1W(34);                // Whitespace | Name | StringLiteral | EOF | '(' | '/' | '/*' | '<?' |
                                    // '<?ENCORE?>' | '<?TOKENS?>' | '|'
    whitespace();
    parse_SyntaxChoice();
    for (;;)
    {
      lookahead1W(21);              // Whitespace | Name | EOF | '/*' | '<?ENCORE?>' | '<?TOKENS?>'
      if (l1 != 23)                 // '/*'
      {
        break;
      }
      whitespace();
      parse_Option();
    }
    eventHandler->endNonterminal(L"SyntaxProduction", e0);
  }

  void parse_SyntaxChoice()
  {
    eventHandler->startNonterminal(L"SyntaxChoice", e0);
    parse_SyntaxSequence();
    if (l1 == 22                    // '/'
     || l1 == 41)                   // '|'
    {
      switch (l1)
      {
      case 41:                      // '|'
        for (;;)
        {
          consume(41);              // '|'
          lookahead1W(33);          // Whitespace | Name | StringLiteral | EOF | '(' | ')' | '/*' | '<?' |
                                    // '<?ENCORE?>' | '<?TOKENS?>' | '|'
          whitespace();
          parse_SyntaxSequence();
          if (l1 != 41)             // '|'
          {
            break;
          }
        }
        break;
      default:
        for (;;)
        {
          consume(22);              // '/'
          lookahead1W(32);          // Whitespace | Name | StringLiteral | EOF | '(' | ')' | '/' | '/*' | '<?' |
                                    // '<?ENCORE?>' | '<?TOKENS?>'
          whitespace();
          parse_SyntaxSequence();
          if (l1 != 22)             // '/'
          {
            break;
          }
        }
        break;
      }
    }
    eventHandler->endNonterminal(L"SyntaxChoice", e0);
  }

  void parse_SyntaxSequence()
  {
    eventHandler->startNonterminal(L"SyntaxSequence", e0);
    for (;;)
    {
      lookahead1W(35);              // Whitespace | Name | StringLiteral | EOF | '(' | ')' | '/' | '/*' | '<?' |
                                    // '<?ENCORE?>' | '<?TOKENS?>' | '|'
      switch (l1)
      {
      case 2:                       // Name
        lookahead2W(40);            // Whitespace | Name | StringLiteral | CaretName | EOF | '(' | ')' | '*' | '+' |
                                    // '/' | '/*' | '::=' | '<?' | '<?ENCORE?>' | '<?TOKENS?>' | '?' | '|'
        break;
      default:
        lk = l1;
        break;
      }
      if (lk == 11                  // EOF
       || lk == 16                  // ')'
       || lk == 22                  // '/'
       || lk == 23                  // '/*'
       || lk == 28                  // '<?ENCORE?>'
       || lk == 29                  // '<?TOKENS?>'
       || lk == 41                  // '|'
       || lk == 1602)               // Name '::='
      {
        break;
      }
      whitespace();
      parse_SyntaxItem();
    }
    eventHandler->endNonterminal(L"SyntaxSequence", e0);
  }

  void parse_SyntaxItem()
  {
    eventHandler->startNonterminal(L"SyntaxItem", e0);
    parse_SyntaxPrimary();
    lookahead1W(38);                // Whitespace | Name | StringLiteral | EOF | '(' | ')' | '*' | '+' | '/' | '/*' |
                                    // '<?' | '<?ENCORE?>' | '<?TOKENS?>' | '?' | '|'
    if (l1 == 17                    // '*'
     || l1 == 19                    // '+'
     || l1 == 32)                   // '?'
    {
      switch (l1)
      {
      case 32:                      // '?'
        consume(32);                // '?'
        break;
      case 17:                      // '*'
        consume(17);                // '*'
        break;
      default:
        consume(19);                // '+'
        break;
      }
    }
    eventHandler->endNonterminal(L"SyntaxItem", e0);
  }

  void parse_SyntaxPrimary()
  {
    eventHandler->startNonterminal(L"SyntaxPrimary", e0);
    switch (l1)
    {
    case 15:                        // '('
      consume(15);                  // '('
      lookahead1W(25);              // Whitespace | Name | StringLiteral | '(' | ')' | '/' | '<?' | '|'
      whitespace();
      parse_SyntaxChoice();
      consume(16);                  // ')'
      break;
    case 27:                        // '<?'
      parse_ProcessingInstruction();
      break;
    default:
      parse_NameOrString();
      break;
    }
    eventHandler->endNonterminal(L"SyntaxPrimary", e0);
  }

  void parse_LexicalDefinition()
  {
    eventHandler->startNonterminal(L"LexicalDefinition", e0);
    consume(29);                    // '<?TOKENS?>'
    for (;;)
    {
      lookahead1W(22);              // Whitespace | Name | StringLiteral | EOF | EquivalenceLookAhead | '.' |
                                    // '<?ENCORE?>'
      if (l1 == 11                  // EOF
       || l1 == 28)                 // '<?ENCORE?>'
      {
        break;
      }
      switch (l1)
      {
      case 2:                       // Name
        lookahead2W(23);            // Whitespace | CaretName | '::=' | '<<' | '>>' | '?' | '\\'
        break;
      default:
        lk = l1;
        break;
      }
      switch (lk)
      {
      case 21:                      // '.'
      case 1602:                    // Name '::='
      case 2050:                    // Name '?'
        whitespace();
        parse_LexicalProduction();
        break;
      case 2306:                    // Name '\\'
        whitespace();
        parse_Delimiter();
        break;
      case 12:                      // EquivalenceLookAhead
        whitespace();
        parse_Equivalence();
        break;
      default:
        whitespace();
        parse_Preference();
        break;
      }
    }
    eventHandler->endNonterminal(L"LexicalDefinition", e0);
  }

  void parse_LexicalProduction()
  {
    eventHandler->startNonterminal(L"LexicalProduction", e0);
    switch (l1)
    {
    case 2:                         // Name
      consume(2);                   // Name
      break;
    default:
      consume(21);                  // '.'
      break;
    }
    lookahead1W(15);                // Whitespace | '::=' | '?'
    if (l1 == 32)                   // '?'
    {
      consume(32);                  // '?'
    }
    lookahead1W(4);                 // Whitespace | '::='
    consume(25);                    // '::='
    lookahead1W(37);                // Whitespace | Name | StringLiteral | CharCode | EOF | EquivalenceLookAhead | '$' |
                                    // '&' | '(' | '.' | '/*' | '<?ENCORE?>' | '[' | '[^' | '|'
    whitespace();
    parse_ContextChoice();
    for (;;)
    {
      lookahead1W(24);              // Whitespace | Name | StringLiteral | EOF | EquivalenceLookAhead | '.' | '/*' |
                                    // '<?ENCORE?>'
      if (l1 != 23)                 // '/*'
      {
        break;
      }
      whitespace();
      parse_Option();
    }
    eventHandler->endNonterminal(L"LexicalProduction", e0);
  }

  void parse_ContextChoice()
  {
    eventHandler->startNonterminal(L"ContextChoice", e0);
    parse_ContextExpression();
    for (;;)
    {
      lookahead1W(27);              // Whitespace | Name | StringLiteral | EOF | EquivalenceLookAhead | '.' | '/*' |
                                    // '<?ENCORE?>' | '|'
      if (l1 != 41)                 // '|'
      {
        break;
      }
      consume(41);                  // '|'
      lookahead1W(37);              // Whitespace | Name | StringLiteral | CharCode | EOF | EquivalenceLookAhead | '$' |
                                    // '&' | '(' | '.' | '/*' | '<?ENCORE?>' | '[' | '[^' | '|'
      whitespace();
      parse_ContextExpression();
    }
    eventHandler->endNonterminal(L"ContextChoice", e0);
  }

  void parse_LexicalChoice()
  {
    eventHandler->startNonterminal(L"LexicalChoice", e0);
    parse_LexicalSequence();
    for (;;)
    {
      lookahead1W(14);              // Whitespace | ')' | '|'
      if (l1 != 41)                 // '|'
      {
        break;
      }
      consume(41);                  // '|'
      lookahead1W(31);              // Whitespace | Name | StringLiteral | CharCode | '$' | '(' | ')' | '.' | '[' |
                                    // '[^' | '|'
      whitespace();
      parse_LexicalSequence();
    }
    eventHandler->endNonterminal(L"LexicalChoice", e0);
  }

  void parse_ContextExpression()
  {
    eventHandler->startNonterminal(L"ContextExpression", e0);
    parse_LexicalSequence();
    lookahead1W(30);                // Whitespace | Name | StringLiteral | EOF | EquivalenceLookAhead | '&' | '.' |
                                    // '/*' | '<?ENCORE?>' | '|'
    if (l1 == 14)                   // '&'
    {
      consume(14);                  // '&'
      lookahead1W(26);              // Whitespace | Name | StringLiteral | CharCode | '$' | '(' | '.' | '[' | '[^'
      whitespace();
      parse_LexicalItem();
    }
    eventHandler->endNonterminal(L"ContextExpression", e0);
  }

  void parse_LexicalSequence()
  {
    eventHandler->startNonterminal(L"LexicalSequence", e0);
    switch (l1)
    {
    case 2:                         // Name
      lookahead2W(51);              // Whitespace | Name | StringLiteral | CaretName | CharCode | EOF |
                                    // EquivalenceLookAhead | '$' | '&' | '(' | ')' | '*' | '+' | '-' | '.' | '/*' |
                                    // '::=' | '<<' | '<?ENCORE?>' | '>>' | '?' | '[' | '[^' | '\\' | '|'
      switch (lk)
      {
      case 2050:                    // Name '?'
        lookahead3W(43);            // Whitespace | Name | StringLiteral | CharCode | EOF | EquivalenceLookAhead | '$' |
                                    // '&' | '(' | ')' | '-' | '.' | '/*' | '::=' | '<?ENCORE?>' | '[' | '[^' | '|'
        break;
      }
      break;
    case 5:                         // StringLiteral
      lookahead2W(49);              // Whitespace | Name | StringLiteral | CaretName | CharCode | EOF |
                                    // EquivalenceLookAhead | '$' | '&' | '(' | ')' | '*' | '+' | '-' | '.' | '/*' |
                                    // '<<' | '<?ENCORE?>' | '>>' | '?' | '[' | '[^' | '|'
      break;
    case 21:                        // '.'
      lookahead2W(47);              // Whitespace | Name | StringLiteral | CharCode | EOF | EquivalenceLookAhead | '$' |
                                    // '&' | '(' | ')' | '*' | '+' | '-' | '.' | '/*' | '::=' | '<?ENCORE?>' | '?' |
                                    // '[' | '[^' | '|'
      switch (lk)
      {
      case 2069:                    // '.' '?'
        lookahead3W(43);            // Whitespace | Name | StringLiteral | CharCode | EOF | EquivalenceLookAhead | '$' |
                                    // '&' | '(' | ')' | '-' | '.' | '/*' | '::=' | '<?ENCORE?>' | '[' | '[^' | '|'
        break;
      }
      break;
    default:
      lk = l1;
      break;
    }
    switch (lk)
    {
    case 11:                        // EOF
    case 12:                        // EquivalenceLookAhead
    case 14:                        // '&'
    case 16:                        // ')'
    case 23:                        // '/*'
    case 28:                        // '<?ENCORE?>'
    case 41:                        // '|'
    case 386:                       // Name CaretName
    case 389:                       // StringLiteral CaretName
    case 1602:                      // Name '::='
    case 1621:                      // '.' '::='
    case 1666:                      // Name '<<'
    case 1669:                      // StringLiteral '<<'
    case 1986:                      // Name '>>'
    case 1989:                      // StringLiteral '>>'
    case 2306:                      // Name '\\'
    case 104450:                    // Name '?' '::='
    case 104469:                    // '.' '?' '::='
      break;
    default:
      parse_LexicalItem();
      lookahead1W(41);              // Whitespace | Name | StringLiteral | CharCode | EOF | EquivalenceLookAhead | '$' |
                                    // '&' | '(' | ')' | '-' | '.' | '/*' | '<?ENCORE?>' | '[' | '[^' | '|'
      switch (l1)
      {
      case 20:                      // '-'
        consume(20);                // '-'
        lookahead1W(26);            // Whitespace | Name | StringLiteral | CharCode | '$' | '(' | '.' | '[' | '[^'
        whitespace();
        parse_LexicalItem();
        break;
      default:
        for (;;)
        {
          lookahead1W(39);          // Whitespace | Name | StringLiteral | CharCode | EOF | EquivalenceLookAhead | '$' |
                                    // '&' | '(' | ')' | '.' | '/*' | '<?ENCORE?>' | '[' | '[^' | '|'
          switch (l1)
          {
          case 2:                   // Name
            lookahead2W(50);        // Whitespace | Name | StringLiteral | CaretName | CharCode | EOF |
                                    // EquivalenceLookAhead | '$' | '&' | '(' | ')' | '*' | '+' | '.' | '/*' | '::=' |
                                    // '<<' | '<?ENCORE?>' | '>>' | '?' | '[' | '[^' | '\\' | '|'
            switch (lk)
            {
            case 2050:              // Name '?'
              lookahead3W(42);      // Whitespace | Name | StringLiteral | CharCode | EOF | EquivalenceLookAhead | '$' |
                                    // '&' | '(' | ')' | '.' | '/*' | '::=' | '<?ENCORE?>' | '[' | '[^' | '|'
              break;
            }
            break;
          case 5:                   // StringLiteral
            lookahead2W(48);        // Whitespace | Name | StringLiteral | CaretName | CharCode | EOF |
                                    // EquivalenceLookAhead | '$' | '&' | '(' | ')' | '*' | '+' | '.' | '/*' | '<<' |
                                    // '<?ENCORE?>' | '>>' | '?' | '[' | '[^' | '|'
            break;
          case 21:                  // '.'
            lookahead2W(46);        // Whitespace | Name | StringLiteral | CharCode | EOF | EquivalenceLookAhead | '$' |
                                    // '&' | '(' | ')' | '*' | '+' | '.' | '/*' | '::=' | '<?ENCORE?>' | '?' | '[' |
                                    // '[^' | '|'
            switch (lk)
            {
            case 2069:              // '.' '?'
              lookahead3W(42);      // Whitespace | Name | StringLiteral | CharCode | EOF | EquivalenceLookAhead | '$' |
                                    // '&' | '(' | ')' | '.' | '/*' | '::=' | '<?ENCORE?>' | '[' | '[^' | '|'
              break;
            }
            break;
          default:
            lk = l1;
            break;
          }
          if (lk == 11              // EOF
           || lk == 12              // EquivalenceLookAhead
           || lk == 14              // '&'
           || lk == 16              // ')'
           || lk == 23              // '/*'
           || lk == 28              // '<?ENCORE?>'
           || lk == 41              // '|'
           || lk == 386             // Name CaretName
           || lk == 389             // StringLiteral CaretName
           || lk == 1602            // Name '::='
           || lk == 1621            // '.' '::='
           || lk == 1666            // Name '<<'
           || lk == 1669            // StringLiteral '<<'
           || lk == 1986            // Name '>>'
           || lk == 1989            // StringLiteral '>>'
           || lk == 2306            // Name '\\'
           || lk == 104450          // Name '?' '::='
           || lk == 104469)         // '.' '?' '::='
          {
            break;
          }
          whitespace();
          parse_LexicalItem();
        }
        break;
      }
      break;
    }
    eventHandler->endNonterminal(L"LexicalSequence", e0);
  }

  void parse_LexicalItem()
  {
    eventHandler->startNonterminal(L"LexicalItem", e0);
    parse_LexicalPrimary();
    lookahead1W(45);                // Whitespace | Name | StringLiteral | CharCode | EOF | EquivalenceLookAhead | '$' |
                                    // '&' | '(' | ')' | '*' | '+' | '-' | '.' | '/*' | '<?ENCORE?>' | '?' | '[' |
                                    // '[^' | '|'
    if (l1 == 17                    // '*'
     || l1 == 19                    // '+'
     || l1 == 32)                   // '?'
    {
      switch (l1)
      {
      case 32:                      // '?'
        consume(32);                // '?'
        break;
      case 17:                      // '*'
        consume(17);                // '*'
        break;
      default:
        consume(19);                // '+'
        break;
      }
    }
    eventHandler->endNonterminal(L"LexicalItem", e0);
  }

  void parse_LexicalPrimary()
  {
    eventHandler->startNonterminal(L"LexicalPrimary", e0);
    switch (l1)
    {
    case 2:                         // Name
    case 21:                        // '.'
      switch (l1)
      {
      case 2:                       // Name
        consume(2);                 // Name
        break;
      default:
        consume(21);                // '.'
        break;
      }
      break;
    case 5:                         // StringLiteral
      consume(5);                   // StringLiteral
      break;
    case 15:                        // '('
      consume(15);                  // '('
      lookahead1W(31);              // Whitespace | Name | StringLiteral | CharCode | '$' | '(' | ')' | '.' | '[' |
                                    // '[^' | '|'
      whitespace();
      parse_LexicalChoice();
      consume(16);                  // ')'
      break;
    case 13:                        // '$'
      consume(13);                  // '$'
      break;
    case 7:                         // CharCode
      consume(7);                   // CharCode
      break;
    default:
      parse_CharClass();
      break;
    }
    eventHandler->endNonterminal(L"LexicalPrimary", e0);
  }

  void parse_NameOrString()
  {
    eventHandler->startNonterminal(L"NameOrString", e0);
    switch (l1)
    {
    case 2:                         // Name
      consume(2);                   // Name
      lookahead1W(44);              // Whitespace | Name | StringLiteral | CaretName | EOF | EquivalenceLookAhead |
                                    // '(' | ')' | '*' | '+' | '.' | '/' | '/*' | '<<' | '<?' | '<?ENCORE?>' |
                                    // '<?TOKENS?>' | '>>' | '?' | '|'
      if (l1 == 6)                  // CaretName
      {
        whitespace();
        parse_Context();
      }
      break;
    default:
      consume(5);                   // StringLiteral
      lookahead1W(44);              // Whitespace | Name | StringLiteral | CaretName | EOF | EquivalenceLookAhead |
                                    // '(' | ')' | '*' | '+' | '.' | '/' | '/*' | '<<' | '<?' | '<?ENCORE?>' |
                                    // '<?TOKENS?>' | '>>' | '?' | '|'
      if (l1 == 6)                  // CaretName
      {
        whitespace();
        parse_Context();
      }
      break;
    }
    eventHandler->endNonterminal(L"NameOrString", e0);
  }

  void parse_Context()
  {
    eventHandler->startNonterminal(L"Context", e0);
    consume(6);                     // CaretName
    eventHandler->endNonterminal(L"Context", e0);
  }

  void parse_CharClass()
  {
    eventHandler->startNonterminal(L"CharClass", e0);
    switch (l1)
    {
    case 34:                        // '['
      consume(34);                  // '['
      break;
    default:
      consume(35);                  // '[^'
      break;
    }
    for (;;)
    {
      lookahead1(19);               // CharCode | Char | CharRange | CharCodeRange
      switch (l1)
      {
      case 8:                       // Char
        consume(8);                 // Char
        break;
      case 7:                       // CharCode
        consume(7);                 // CharCode
        break;
      case 9:                       // CharRange
        consume(9);                 // CharRange
        break;
      default:
        consume(10);                // CharCodeRange
        break;
      }
      lookahead1(20);               // CharCode | Char | CharRange | CharCodeRange | ']'
      if (l1 == 37)                 // ']'
      {
        break;
      }
    }
    consume(37);                    // ']'
    eventHandler->endNonterminal(L"CharClass", e0);
  }

  void parse_Option()
  {
    eventHandler->startNonterminal(L"Option", e0);
    consume(23);                    // '/*'
    for (;;)
    {
      lookahead1(9);                // Space | 'ws'
      if (l1 != 3)                  // Space
      {
        break;
      }
      consume(3);                   // Space
    }
    consume(40);                    // 'ws'
    lookahead1(1);                  // ':'
    consume(24);                    // ':'
    for (;;)
    {
      lookahead1(18);               // Space | 'definition' | 'explicit'
      if (l1 != 3)                  // Space
      {
        break;
      }
      consume(3);                   // Space
    }
    switch (l1)
    {
    case 39:                        // 'explicit'
      consume(39);                  // 'explicit'
      break;
    default:
      consume(38);                  // 'definition'
      break;
    }
    for (;;)
    {
      lookahead1(7);                // Space | '*/'
      if (l1 != 3)                  // Space
      {
        break;
      }
      consume(3);                   // Space
    }
    consume(18);                    // '*/'
    eventHandler->endNonterminal(L"Option", e0);
  }

  void parse_Preference()
  {
    eventHandler->startNonterminal(L"Preference", e0);
    parse_NameOrString();
    lookahead1W(16);                // Whitespace | '<<' | '>>'
    switch (l1)
    {
    case 31:                        // '>>'
      consume(31);                  // '>>'
      for (;;)
      {
        lookahead1W(10);            // Whitespace | Name | StringLiteral
        whitespace();
        parse_NameOrString();
        lookahead1W(22);            // Whitespace | Name | StringLiteral | EOF | EquivalenceLookAhead | '.' |
                                    // '<?ENCORE?>'
        switch (l1)
        {
        case 2:                     // Name
          lookahead2W(36);          // Whitespace | Name | StringLiteral | CaretName | EOF | EquivalenceLookAhead |
                                    // '.' | '::=' | '<<' | '<?ENCORE?>' | '>>' | '?' | '\\'
          switch (lk)
          {
          case 386:                 // Name CaretName
            lookahead3W(28);        // Whitespace | Name | StringLiteral | EOF | EquivalenceLookAhead | '.' | '<<' |
                                    // '<?ENCORE?>' | '>>'
            break;
          }
          break;
        case 5:                     // StringLiteral
          lookahead2W(29);          // Whitespace | Name | StringLiteral | CaretName | EOF | EquivalenceLookAhead |
                                    // '.' | '<<' | '<?ENCORE?>' | '>>'
          switch (lk)
          {
          case 389:                 // StringLiteral CaretName
            lookahead3W(28);        // Whitespace | Name | StringLiteral | EOF | EquivalenceLookAhead | '.' | '<<' |
                                    // '<?ENCORE?>' | '>>'
            break;
          }
          break;
        default:
          lk = l1;
          break;
        }
        if (lk == 11                // EOF
         || lk == 12                // EquivalenceLookAhead
         || lk == 21                // '.'
         || lk == 28                // '<?ENCORE?>'
         || lk == 1602              // Name '::='
         || lk == 1666              // Name '<<'
         || lk == 1669              // StringLiteral '<<'
         || lk == 1986              // Name '>>'
         || lk == 1989              // StringLiteral '>>'
         || lk == 2050              // Name '?'
         || lk == 2306              // Name '\\'
         || lk == 106882            // Name CaretName '<<'
         || lk == 106885            // StringLiteral CaretName '<<'
         || lk == 127362            // Name CaretName '>>'
         || lk == 127365)           // StringLiteral CaretName '>>'
        {
          break;
        }
      }
      break;
    default:
      consume(26);                  // '<<'
      for (;;)
      {
        lookahead1W(10);            // Whitespace | Name | StringLiteral
        whitespace();
        parse_NameOrString();
        lookahead1W(22);            // Whitespace | Name | StringLiteral | EOF | EquivalenceLookAhead | '.' |
                                    // '<?ENCORE?>'
        switch (l1)
        {
        case 2:                     // Name
          lookahead2W(36);          // Whitespace | Name | StringLiteral | CaretName | EOF | EquivalenceLookAhead |
                                    // '.' | '::=' | '<<' | '<?ENCORE?>' | '>>' | '?' | '\\'
          switch (lk)
          {
          case 386:                 // Name CaretName
            lookahead3W(28);        // Whitespace | Name | StringLiteral | EOF | EquivalenceLookAhead | '.' | '<<' |
                                    // '<?ENCORE?>' | '>>'
            break;
          }
          break;
        case 5:                     // StringLiteral
          lookahead2W(29);          // Whitespace | Name | StringLiteral | CaretName | EOF | EquivalenceLookAhead |
                                    // '.' | '<<' | '<?ENCORE?>' | '>>'
          switch (lk)
          {
          case 389:                 // StringLiteral CaretName
            lookahead3W(28);        // Whitespace | Name | StringLiteral | EOF | EquivalenceLookAhead | '.' | '<<' |
                                    // '<?ENCORE?>' | '>>'
            break;
          }
          break;
        default:
          lk = l1;
          break;
        }
        if (lk == 11                // EOF
         || lk == 12                // EquivalenceLookAhead
         || lk == 21                // '.'
         || lk == 28                // '<?ENCORE?>'
         || lk == 1602              // Name '::='
         || lk == 1666              // Name '<<'
         || lk == 1669              // StringLiteral '<<'
         || lk == 1986              // Name '>>'
         || lk == 1989              // StringLiteral '>>'
         || lk == 2050              // Name '?'
         || lk == 2306              // Name '\\'
         || lk == 106882            // Name CaretName '<<'
         || lk == 106885            // StringLiteral CaretName '<<'
         || lk == 127362            // Name CaretName '>>'
         || lk == 127365)           // StringLiteral CaretName '>>'
        {
          break;
        }
      }
      break;
    }
    eventHandler->endNonterminal(L"Preference", e0);
  }

  void parse_Delimiter()
  {
    eventHandler->startNonterminal(L"Delimiter", e0);
    consume(2);                     // Name
    lookahead1W(6);                 // Whitespace | '\\'
    consume(36);                    // '\\'
    for (;;)
    {
      lookahead1W(10);              // Whitespace | Name | StringLiteral
      whitespace();
      parse_NameOrString();
      lookahead1W(22);              // Whitespace | Name | StringLiteral | EOF | EquivalenceLookAhead | '.' |
                                    // '<?ENCORE?>'
      switch (l1)
      {
      case 2:                       // Name
        lookahead2W(36);            // Whitespace | Name | StringLiteral | CaretName | EOF | EquivalenceLookAhead |
                                    // '.' | '::=' | '<<' | '<?ENCORE?>' | '>>' | '?' | '\\'
        switch (lk)
        {
        case 386:                   // Name CaretName
          lookahead3W(28);          // Whitespace | Name | StringLiteral | EOF | EquivalenceLookAhead | '.' | '<<' |
                                    // '<?ENCORE?>' | '>>'
          break;
        }
        break;
      case 5:                       // StringLiteral
        lookahead2W(29);            // Whitespace | Name | StringLiteral | CaretName | EOF | EquivalenceLookAhead |
                                    // '.' | '<<' | '<?ENCORE?>' | '>>'
        switch (lk)
        {
        case 389:                   // StringLiteral CaretName
          lookahead3W(28);          // Whitespace | Name | StringLiteral | EOF | EquivalenceLookAhead | '.' | '<<' |
                                    // '<?ENCORE?>' | '>>'
          break;
        }
        break;
      default:
        lk = l1;
        break;
      }
      if (lk == 11                  // EOF
       || lk == 12                  // EquivalenceLookAhead
       || lk == 21                  // '.'
       || lk == 28                  // '<?ENCORE?>'
       || lk == 1602                // Name '::='
       || lk == 1666                // Name '<<'
       || lk == 1669                // StringLiteral '<<'
       || lk == 1986                // Name '>>'
       || lk == 1989                // StringLiteral '>>'
       || lk == 2050                // Name '?'
       || lk == 2306                // Name '\\'
       || lk == 106882              // Name CaretName '<<'
       || lk == 106885              // StringLiteral CaretName '<<'
       || lk == 127362              // Name CaretName '>>'
       || lk == 127365)             // StringLiteral CaretName '>>'
      {
        break;
      }
    }
    eventHandler->endNonterminal(L"Delimiter", e0);
  }

  void parse_Equivalence()
  {
    eventHandler->startNonterminal(L"Equivalence", e0);
    consume(12);                    // EquivalenceLookAhead
    lookahead1W(12);                // Whitespace | StringLiteral | '['
    whitespace();
    parse_EquivalenceCharRange();
    lookahead1W(5);                 // Whitespace | '=='
    consume(30);                    // '=='
    lookahead1W(12);                // Whitespace | StringLiteral | '['
    whitespace();
    parse_EquivalenceCharRange();
    eventHandler->endNonterminal(L"Equivalence", e0);
  }

  void parse_EquivalenceCharRange()
  {
    eventHandler->startNonterminal(L"EquivalenceCharRange", e0);
    switch (l1)
    {
    case 5:                         // StringLiteral
      consume(5);                   // StringLiteral
      break;
    default:
      consume(34);                  // '['
      lookahead1(19);               // CharCode | Char | CharRange | CharCodeRange
      switch (l1)
      {
      case 8:                       // Char
        consume(8);                 // Char
        break;
      case 7:                       // CharCode
        consume(7);                 // CharCode
        break;
      case 9:                       // CharRange
        consume(9);                 // CharRange
        break;
      default:
        consume(10);                // CharCodeRange
        break;
      }
      lookahead1(3);                // ']'
      consume(37);                  // ']'
      break;
    }
    eventHandler->endNonterminal(L"EquivalenceCharRange", e0);
  }

  void parse_Encore()
  {
    eventHandler->startNonterminal(L"Encore", e0);
    consume(28);                    // '<?ENCORE?>'
    for (;;)
    {
      lookahead1W(13);              // Whitespace | EOF | '<?'
      if (l1 != 27)                 // '<?'
      {
        break;
      }
      whitespace();
      parse_ProcessingInstruction();
    }
    eventHandler->endNonterminal(L"Encore", e0);
  }

  void consume(int t)
  {
    if (l1 == t)
    {
      whitespace();
      eventHandler->terminal(TOKEN[l1], b1, e1);
      b0 = b1; e0 = e1; l1 = l2; if (l1 != 0) {
      b1 = b2; e1 = e2; l2 = l3; if (l2 != 0) {
      b2 = b3; e2 = e3; l3 = 0; }}
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
      if (code != 1)                // Whitespace
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

  void lookahead2W(int tokenSetId)
  {
    if (l2 == 0)
    {
      l2 = matchW(tokenSetId);
      b2 = begin;
      e2 = end;
    }
    lk = (l2 << 6) | l1;
  }

  void lookahead3W(int tokenSetId)
  {
    if (l3 == 0)
    {
      l3 = matchW(tokenSetId);
      b3 = begin;
      e3 = end;
    }
    lk |= l3 << 12;
  }

  void lookahead1(int tokenSetId)
  {
    if (l1 == 0)
    {
      l1 = match(tokenSetId);
      b1 = begin;
      e1 = end;
    }
  }

  int error(int b, int e, int s, int l, int t)
  {
    throw ParseException(b, e, s, l, t);
  }

  int lk, b0, e0;
  int l1, b1, e1;
  int l2, b2, e2;
  int l3, b3, e3;
  EventHandler *eventHandler;

  const wchar_t *input;
  int begin;
  int end;

  int match(int tokenSetId)
  {
    bool nonbmp = false;
    begin = end;
    int current = end;
    int result = INITIAL[tokenSetId];
    int state = 0;

    for (int code = result & 255; code != 0; )
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
        int c1 = c0 >> 3;
        charclass = MAP1[(c0 & 7) + MAP1[(c1 & 31) + MAP1[c1 >> 5]]];
      }
      else
      {
        if (c0 < 0xdc00)
        {
          int c1 = input[current];
          if (c1 >= 0xdc00 && c1 < 0xe000)
          {
            nonbmp = true;
            ++current;
            c0 = ((c0 & 0x3ff) << 10) + (c1 & 0x3ff) + 0x10000;
          }
        }
        int lo = 0, hi = 1;
        for (int m = 1; ; m = (hi + lo) >> 1)
        {
          if (MAP2[m] > c0) hi = m - 1;
          else if (MAP2[2 + m] < c0) lo = m + 1;
          else {charclass = MAP2[4 + m]; break;}
          if (lo > hi) {charclass = 0; break;}
        }
      }

      state = code;
      int i0 = (charclass << 8) + code - 1;
      code = TRANSITION[(i0 & 7) + TRANSITION[i0 >> 3]];
      if (code > 255)
      {
        result = code;
        code &= 255;
        end = current;
      }
    }

    result >>= 8;
    if (result == 0)
    {
      end = current - 1;
      int c1 = input[end];
      if (c1 >= 0xdc00 && c1 < 0xe000) --end;
      return error(begin, end, state, -1, -1);
    }

    if ((result & 64) != 0)
    {
      end = begin;
      if (nonbmp)
      {
        for (int i = result >> 7; i > 0; --i)
        {
          int c1 = input[end++];
          if (c1 >= 0xd800 && c1 < 0xdc00) ++end;
        }
      }
      else
      {
        end += (result >> 7);
      }
    }
    else
    {
      if (nonbmp)
      {
        for (int i = result >> 7; i > 0; --i)
        {
          int c1 = input[--end];
          if (c1 >= 0xdc00 && c1 < 0xe000) --end;
        }
      }
      else
      {
        end -= result >> 7;
      }
    }

    if (input[begin] == 0) end = begin;
    return (result & 63) - 1;
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
    int s = tokenSetId < 0 ? - tokenSetId : INITIAL[tokenSetId] & 255;
    for (int i = 0; i < 42; i += 32)
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
    int i0 = t * 177 + s - 1;
    return EXPECTED[(i0 & 3) + EXPECTED[i0 >> 2]];
  }

  static const int MAP0[];
  static const int MAP1[];
  static const int MAP2[];
  static const int INITIAL[];
  static const int TRANSITION[];
  static const int EXPECTED[];
  static const wchar_t *TOKEN[];
};

const int REx::MAP0[] =
{
/*   0 */ 52, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 4, 5, 6,
/*  36 */ 7, 4, 8, 9, 10, 11, 12, 13, 4, 14, 15, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 4, 19, 20, 21, 22, 4,
/*  65 */ 23, 23, 24, 23, 25, 23, 26, 26, 26, 26, 27, 26, 26, 28, 29, 26, 26, 30, 31, 32, 26, 26, 26, 26, 26, 26, 33,
/*  92 */ 34, 35, 36, 26, 4, 23, 23, 37, 38, 39, 40, 26, 26, 41, 26, 26, 42, 26, 43, 44, 45, 26, 26, 46, 47, 26, 26, 48,
/* 120 */ 49, 26, 26, 4, 50, 4, 4, 4
};

const int REx::MAP1[] =
{
/*    0 */ 216, 291, 323, 383, 415, 908, 351, 815, 815, 447, 479, 511, 543, 575, 621, 882, 589, 681, 815, 815, 815, 815,
/*   22 */ 815, 815, 815, 815, 815, 815, 815, 815, 713, 745, 821, 649, 815, 815, 815, 815, 815, 815, 815, 815, 815, 815,
/*   44 */ 815, 815, 815, 815, 777, 809, 815, 815, 815, 815, 815, 815, 815, 815, 815, 815, 815, 815, 815, 815, 815, 815,
/*   66 */ 815, 815, 815, 815, 815, 815, 815, 815, 815, 815, 815, 815, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247,
/*   88 */ 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247,
/*  110 */ 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247,
/*  132 */ 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247,
/*  154 */ 247, 247, 247, 247, 247, 259, 815, 815, 815, 815, 815, 815, 815, 815, 815, 815, 815, 815, 247, 247, 247, 247,
/*  176 */ 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247,
/*  198 */ 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 247, 853, 940, 949, 941, 941,
/*  220 */ 957, 965, 973, 979, 987, 1010, 1018, 1035, 1053, 1071, 1079, 1087, 1262, 1262, 1262, 1262, 1262, 1262, 1433,
/*  239 */ 1262, 1254, 1254, 1255, 1254, 1254, 1254, 1255, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254,
/*  257 */ 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254,
/*  275 */ 1254, 1254, 1254, 1254, 1256, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1254, 1254,
/*  293 */ 1254, 1254, 1254, 1254, 1342, 1255, 1253, 1252, 1254, 1254, 1254, 1254, 1254, 1255, 1254, 1254, 1254, 1254,
/*  311 */ 1254, 1254, 1254, 1254, 1258, 1418, 1254, 1254, 1254, 1254, 1062, 1421, 1254, 1254, 1254, 1262, 1262, 1262,
/*  329 */ 1262, 1262, 1262, 1262, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1261, 1262, 1420,
/*  347 */ 1260, 1262, 1388, 1262, 1262, 1262, 1262, 1262, 1253, 1254, 1254, 1259, 1131, 1308, 1387, 1262, 1382, 1388,
/*  365 */ 1131, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1344, 1254, 1255, 1142, 1382, 1297, 1196, 1382, 1388,
/*  383 */ 1382, 1382, 1382, 1382, 1382, 1382, 1382, 1382, 1384, 1262, 1262, 1262, 1388, 1262, 1262, 1262, 1367, 1231,
/*  401 */ 1254, 1254, 1251, 1254, 1254, 1254, 1254, 1255, 1255, 1407, 1252, 1254, 1258, 1262, 1253, 1100, 1254, 1254,
/*  419 */ 1254, 1254, 1254, 1254, 1254, 1254, 1253, 1100, 1254, 1254, 1254, 1254, 1109, 1262, 1254, 1254, 1254, 1254,
/*  437 */ 1254, 1254, 1122, 1042, 1254, 1254, 1254, 1123, 1256, 1260, 1446, 1254, 1254, 1254, 1254, 1254, 1254, 1160,
/*  455 */ 1382, 1384, 1197, 1254, 1178, 1382, 1262, 1262, 1446, 1122, 1343, 1254, 1254, 1252, 1060, 1192, 1169, 1181,
/*  473 */ 1433, 1207, 1178, 1382, 1260, 1262, 1218, 1241, 1343, 1254, 1254, 1252, 1397, 1192, 1184, 1181, 1262, 1229,
/*  491 */ 1434, 1382, 1239, 1262, 1446, 1230, 1251, 1254, 1254, 1252, 1249, 1160, 1272, 1114, 1262, 1262, 994, 1382,
/*  509 */ 1262, 1262, 1446, 1122, 1343, 1254, 1254, 1252, 1340, 1160, 1198, 1181, 1434, 1207, 1045, 1382, 1262, 1262,
/*  527 */ 1002, 1023, 1285, 1281, 1063, 1023, 1133, 1045, 1199, 1196, 1433, 1262, 1433, 1382, 1262, 1262, 1446, 1100,
/*  545 */ 1252, 1254, 1254, 1252, 1101, 1045, 1273, 1196, 1435, 1262, 1045, 1382, 1262, 1262, 1002, 1100, 1252, 1254,
/*  563 */ 1254, 1252, 1101, 1045, 1273, 1196, 1435, 1264, 1045, 1382, 1262, 1262, 1002, 1100, 1252, 1254, 1254, 1252,
/*  581 */ 1254, 1045, 1170, 1196, 1433, 1262, 1045, 1382, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262,
/*  599 */ 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1254, 1254, 1254, 1254, 1256, 1262, 1254, 1254,
/*  617 */ 1254, 1254, 1255, 1262, 1253, 1254, 1254, 1254, 1254, 1255, 1293, 1387, 1305, 1383, 1382, 1388, 1262, 1262,
/*  635 */ 1262, 1262, 1210, 1317, 1419, 1253, 1327, 1337, 1293, 1152, 1352, 1384, 1382, 1388, 1262, 1262, 1262, 1262,
/*  653 */ 1264, 1027, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1259, 1262, 1262, 1262, 1262, 1262,
/*  671 */ 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1249, 1396, 1259, 1262, 1262, 1262, 1262, 1405,
/*  689 */ 1261, 1405, 1062, 1416, 1329, 1061, 1209, 1262, 1262, 1262, 1262, 1264, 1262, 1319, 1263, 1283, 1259, 1262,
/*  707 */ 1262, 1262, 1262, 1429, 1261, 1431, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254,
/*  725 */ 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1258, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254,
/*  743 */ 1254, 1260, 1254, 1254, 1256, 1256, 1254, 1254, 1254, 1254, 1256, 1256, 1254, 1408, 1254, 1254, 1254, 1256,
/*  761 */ 1254, 1254, 1254, 1254, 1254, 1254, 1100, 1134, 1221, 1257, 1123, 1258, 1254, 1257, 1221, 1257, 1092, 1262,
/*  779 */ 1262, 1262, 1253, 1309, 1168, 1262, 1253, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1257, 999,
/*  797 */ 1253, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1443, 1418, 1254, 1254, 1254, 1254, 1257,
/*  815 */ 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262,
/*  833 */ 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1382, 1385, 1365, 1262,
/*  851 */ 1262, 1262, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254, 1254,
/*  869 */ 1254, 1254, 1254, 1254, 1258, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1388, 1382,
/*  887 */ 1388, 1375, 1357, 1254, 1253, 1254, 1254, 1254, 1260, 1381, 1382, 1273, 1386, 1272, 1381, 1382, 1384, 1381,
/*  905 */ 1365, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1262, 1253, 1254, 1254, 1254, 1255, 1431, 1253, 1254, 1254,
/*  923 */ 1254, 1255, 1262, 1381, 1382, 1166, 1382, 1382, 1148, 1362, 1262, 1254, 1254, 1254, 1259, 1259, 1262, 52, 0,
/*  942 */ 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 1, 0, 0, 3, 4, 5, 6, 7, 4, 8, 9, 10, 11, 12, 13, 4, 14, 15, 16, 17, 17,
/*  975 */ 17, 17, 17, 17, 17, 17, 18, 4, 19, 20, 21, 22, 4, 23, 23, 24, 23, 25, 23, 26, 4, 4, 4, 4, 4, 51, 51, 4, 4,
/* 1004 */ 51, 51, 4, 26, 26, 26, 26, 26, 26, 27, 26, 26, 28, 29, 26, 26, 30, 31, 32, 26, 26, 26, 4, 4, 4, 26, 26, 4, 4,
/* 1033 */ 26, 4, 26, 26, 26, 33, 34, 35, 36, 26, 4, 4, 26, 26, 4, 4, 4, 4, 51, 51, 4, 23, 23, 37, 38, 39, 40, 26, 4,
/* 1062 */ 26, 4, 4, 4, 26, 26, 4, 4, 4, 26, 41, 26, 26, 42, 26, 43, 44, 45, 26, 26, 46, 47, 26, 26, 48, 49, 26, 26, 4,
/* 1091 */ 50, 4, 4, 4, 4, 4, 51, 4, 26, 26, 26, 26, 26, 26, 4, 26, 26, 26, 26, 26, 4, 51, 51, 51, 51, 4, 51, 51, 51, 4,
/* 1121 */ 4, 26, 26, 26, 26, 26, 4, 4, 26, 26, 51, 26, 26, 26, 26, 26, 26, 26, 4, 26, 4, 26, 26, 26, 26, 4, 26, 51, 51,
/* 1150 */ 4, 51, 51, 51, 4, 51, 51, 26, 4, 4, 26, 26, 4, 4, 51, 26, 51, 51, 4, 51, 51, 51, 51, 51, 4, 4, 51, 51, 26,
/* 1179 */ 26, 51, 51, 4, 4, 51, 51, 51, 4, 4, 4, 4, 51, 26, 26, 4, 4, 51, 4, 51, 51, 51, 51, 4, 4, 4, 51, 51, 4, 4, 4,
/* 1210 */ 4, 26, 26, 4, 26, 4, 4, 26, 4, 4, 51, 4, 4, 26, 26, 26, 4, 26, 26, 4, 26, 26, 26, 26, 4, 26, 4, 26, 26, 51,
/* 1240 */ 51, 26, 26, 26, 4, 4, 4, 4, 26, 26, 4, 26, 26, 4, 26, 26, 26, 26, 26, 26, 26, 26, 4, 4, 4, 4, 4, 4, 4, 4, 26,
/* 1271 */ 4, 51, 51, 51, 51, 51, 51, 4, 51, 51, 4, 26, 26, 4, 26, 4, 26, 26, 26, 26, 4, 4, 26, 51, 26, 26, 51, 51, 51,
/* 1300 */ 51, 51, 26, 26, 51, 26, 26, 26, 26, 26, 26, 51, 51, 51, 51, 51, 51, 26, 4, 26, 4, 4, 26, 4, 4, 26, 26, 4, 26,
/* 1329 */ 26, 26, 4, 26, 4, 26, 4, 26, 4, 4, 26, 26, 4, 26, 26, 4, 4, 26, 26, 26, 26, 26, 4, 26, 26, 26, 26, 26, 4, 51,
/* 1359 */ 4, 4, 4, 4, 51, 51, 4, 51, 4, 4, 4, 4, 4, 4, 26, 51, 4, 4, 4, 4, 4, 51, 4, 51, 51, 51, 51, 51, 51, 51, 51, 4,
/* 1391 */ 4, 4, 4, 4, 4, 4, 26, 4, 26, 26, 4, 26, 26, 4, 4, 4, 4, 4, 26, 4, 26, 4, 26, 4, 26, 4, 26, 4, 4, 4, 4, 4, 26,
/* 1424 */ 26, 26, 26, 26, 26, 4, 4, 4, 26, 4, 4, 4, 4, 4, 4, 4, 51, 51, 4, 26, 26, 26, 4, 51, 51, 51, 4, 26, 26, 26
};

const int REx::MAP2[] =
{
/* 0 */ 57344, 65536, 65533, 1114111, 4, 4
};

const int REx::INITIAL[] =
{
/*  0 */ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
/* 30 */ 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52
};

const int REx::TRANSITION[] =
{
/*    0 */ 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729,
/*   18 */ 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3275, 1696, 1706, 1698,
/*   36 */ 1698, 1698, 1813, 2663, 2078, 1728, 2455, 2082, 2150, 2220, 1738, 2805, 3305, 1747, 1758, 3076, 1775, 3257,
/*   54 */ 1797, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3086, 1809, 1713, 1698, 1698, 1698, 1813, 1729,
/*   72 */ 2078, 1728, 2125, 2082, 2959, 2220, 1821, 2895, 2405, 1831, 3001, 3270, 1842, 2439, 1797, 1729, 1729, 1729,
/*   90 */ 1729, 1729, 1729, 1729, 1729, 1729, 3275, 1696, 1720, 1698, 1698, 1698, 1813, 2827, 2078, 1872, 1897, 2188,
/*  108 */ 2150, 2863, 1738, 2805, 3305, 1747, 1758, 3076, 1775, 3257, 1797, 1729, 1729, 1729, 1729, 1729, 1729, 1729,
/*  126 */ 1729, 1729, 1729, 1729, 2347, 1729, 1729, 1729, 1729, 1823, 2078, 1872, 2067, 2188, 2150, 2880, 2811, 2205,
/*  144 */ 1750, 2776, 1890, 3416, 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 2902,
/*  162 */ 3230, 1921, 1921, 1921, 1925, 2685, 2078, 1872, 2067, 2188, 2150, 2880, 2811, 2205, 1750, 2776, 1890, 3416,
/*  180 */ 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 2073, 1849, 3286, 1855,
/*  198 */ 1860, 1823, 2078, 1937, 2341, 2188, 2150, 2880, 2507, 2205, 1834, 2776, 1890, 3416, 3384, 3117, 2683, 1729,
/*  216 */ 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 2347, 3048, 1864, 3054, 3059, 1823, 2078, 1872,
/*  234 */ 2067, 2188, 2150, 2880, 2811, 2205, 1750, 2776, 1890, 3416, 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729,
/*  252 */ 1729, 1729, 1729, 1729, 1729, 1729, 2347, 2108, 2109, 1950, 1955, 1823, 2078, 1872, 2067, 2188, 2150, 2880,
/*  270 */ 2811, 2205, 1750, 2776, 1890, 3416, 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729,
/*  288 */ 1729, 3008, 2326, 2470, 2470, 2470, 2474, 2813, 2078, 1872, 2067, 2188, 2150, 2880, 2811, 2205, 1750, 2776,
/*  306 */ 1890, 3416, 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 2347, 1967,
/*  324 */ 1974, 1979, 1983, 1823, 2078, 1872, 2067, 2188, 2150, 2880, 2811, 2205, 1750, 2776, 1890, 3416, 3384, 3117,
/*  342 */ 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 2368, 2347, 1995, 2002, 2008, 2012, 1823,
/*  360 */ 2078, 1872, 2067, 2188, 2150, 2880, 2811, 2205, 1750, 2776, 1890, 3416, 3384, 3117, 2683, 1729, 1729, 1729,
/*  378 */ 1729, 1729, 1729, 1729, 1729, 1729, 2083, 1729, 2347, 1729, 2642, 2024, 2028, 1823, 2078, 2036, 2248, 2188,
/*  396 */ 2294, 3363, 2402, 2396, 2052, 2049, 2060, 2954, 2091, 2591, 2106, 1729, 1729, 1729, 1729, 1729, 1729, 1729,
/*  414 */ 1729, 1729, 1729, 1729, 2347, 1729, 3033, 2117, 2121, 1823, 2078, 1872, 2067, 2188, 2150, 2880, 2811, 2205,
/*  432 */ 1750, 2776, 1890, 3416, 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729,
/*  450 */ 2347, 1729, 1729, 2133, 2137, 1823, 1942, 2145, 2067, 2167, 2174, 2200, 2811, 2213, 1750, 2776, 1890, 3416,
/*  468 */ 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3341, 2357, 2352, 2358,
/*  486 */ 2228, 1823, 2078, 1872, 2067, 2188, 2241, 2880, 2811, 2205, 1750, 2776, 1890, 3416, 3384, 3117, 2683, 1729,
/*  504 */ 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3063, 1782, 1789, 2262, 2270, 2276, 2281, 1882, 2078, 2289,
/*  522 */ 3348, 2188, 2233, 2971, 2632, 2626, 2779, 2302, 2313, 2931, 2334, 3177, 2366, 1729, 1729, 1729, 1729, 1729,
/*  540 */ 1729, 1729, 1729, 1729, 1729, 1729, 2347, 1729, 1729, 1729, 1877, 1823, 2078, 2537, 2067, 2376, 2389, 2413,
/*  558 */ 2811, 2418, 1750, 3378, 1890, 2734, 2426, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729,
/*  576 */ 2434, 1730, 2254, 1729, 1959, 2447, 2451, 2463, 2078, 1872, 2067, 2188, 2150, 2754, 2811, 2205, 1750, 2155,
/*  594 */ 3171, 3081, 3384, 2159, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 2912, 2486, 2494,
/*  612 */ 2519, 2525, 2533, 1823, 2545, 1872, 2557, 2188, 2150, 2880, 2811, 2205, 1750, 2776, 1890, 3416, 3384, 3117,
/*  630 */ 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3105, 1729, 2347, 1729, 1729, 1729, 1729, 2571,
/*  648 */ 2078, 1872, 2181, 2188, 2150, 3189, 2811, 2579, 2305, 2776, 2599, 3411, 2619, 1913, 2640, 1729, 1729, 1729,
/*  666 */ 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 2650, 1987, 2478, 2478, 2657, 1823, 3204, 1872, 2098, 2841,
/*  684 */ 2150, 2880, 2811, 2205, 1750, 2776, 1890, 3416, 3384, 2676, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729,
/*  702 */ 1729, 1729, 1765, 1767, 2693, 1729, 3421, 2700, 2704, 1823, 2716, 2729, 2742, 2750, 2150, 2880, 2811, 2205,
/*  720 */ 1750, 2776, 1890, 3209, 2762, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3031, 3028,
/*  738 */ 3399, 2787, 2787, 2787, 2790, 1823, 2078, 2845, 2067, 2376, 2389, 2413, 2811, 2418, 1750, 3378, 1890, 2734,
/*  756 */ 2426, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3031, 3028, 3399, 2787, 2787, 2787,
/*  774 */ 2790, 1823, 2078, 2845, 2067, 2376, 2389, 2413, 2926, 2418, 1750, 3378, 1890, 2734, 2426, 3117, 2683, 1729,
/*  792 */ 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3031, 3028, 3399, 2787, 2787, 2787, 2790, 1823, 2078, 2845,
/*  810 */ 2067, 2798, 2389, 2413, 2811, 2418, 2381, 3378, 1890, 2821, 2426, 3117, 2683, 1729, 1729, 1729, 1729, 1729,
/*  828 */ 1729, 1729, 1729, 1729, 3031, 3028, 3399, 2787, 2787, 2787, 2790, 1823, 2078, 2835, 2067, 2188, 2241, 2880,
/*  846 */ 2811, 2205, 1750, 2776, 1890, 3416, 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729,
/*  864 */ 3031, 3028, 3399, 2787, 2787, 2787, 2790, 1823, 2078, 2835, 2067, 2188, 2241, 2880, 2585, 2205, 1750, 2776,
/*  882 */ 1890, 3416, 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3031, 3028, 3399, 2787,
/*  900 */ 2787, 2787, 2790, 1823, 2078, 2835, 2067, 2188, 2241, 2606, 2811, 2205, 1750, 2994, 1890, 3416, 3384, 3117,
/*  918 */ 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3031, 3028, 3399, 2787, 2787, 2787, 2790, 1823,
/*  936 */ 2078, 2835, 2067, 2188, 2241, 2936, 2811, 2205, 2769, 2776, 1890, 3416, 3384, 3117, 2683, 1729, 1729, 1729,
/*  954 */ 1729, 1729, 1729, 1729, 1729, 1729, 3031, 3028, 3399, 2787, 2787, 2787, 2790, 1823, 2078, 2835, 2067, 2188,
/*  972 */ 2241, 2880, 2811, 2205, 1750, 3140, 1890, 3416, 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729,
/*  990 */ 1729, 1729, 3031, 3028, 3399, 2787, 2787, 2787, 2790, 1823, 2078, 2835, 2067, 2188, 2241, 2880, 2811, 2205,
/* 1008 */ 1750, 2776, 1890, 3406, 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3031, 3028,
/* 1026 */ 3399, 2787, 2787, 2787, 2790, 1823, 2078, 2835, 2067, 2853, 2241, 2880, 2811, 2205, 1750, 2776, 1890, 3416,
/* 1044 */ 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 2907, 3320, 3326, 2016, 2871,
/* 1062 */ 2876, 1823, 2078, 1872, 2067, 2188, 2150, 2880, 2811, 2205, 1750, 2776, 1890, 3416, 3384, 3117, 2683, 1729,
/* 1080 */ 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1799, 1729, 3354, 1729, 1801, 1729, 3359, 3245, 2078, 1872,
/* 1098 */ 2067, 2188, 2150, 2880, 2811, 2205, 1750, 2776, 1890, 3416, 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729,
/* 1116 */ 1729, 1729, 1729, 1729, 2859, 1729, 1903, 1729, 1729, 1729, 1729, 1823, 2078, 1728, 2708, 2549, 2888, 1739,
/* 1134 */ 2219, 2920, 1750, 2776, 1890, 3416, 3146, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729,
/* 1152 */ 1729, 1729, 2563, 2667, 2668, 2949, 2967, 1823, 2078, 2979, 3224, 2188, 2150, 2880, 2811, 2205, 1750, 2776,
/* 1170 */ 1890, 3416, 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3031, 3028, 3399, 2787,
/* 1188 */ 2787, 2787, 2790, 1823, 2078, 2845, 2067, 2376, 2389, 2413, 2811, 2418, 2987, 3378, 1890, 2734, 2426, 3117,
/* 1206 */ 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3031, 3028, 3021, 2787, 2787, 2787, 2790, 1823,
/* 1224 */ 2078, 2845, 2067, 2376, 2389, 2413, 2811, 2418, 1750, 3378, 1890, 2734, 2426, 3117, 2683, 1729, 1729, 1729,
/* 1242 */ 1729, 1729, 1729, 1729, 1729, 1729, 3031, 3028, 3041, 2787, 2787, 2787, 2790, 1823, 2041, 2845, 2067, 2376,
/* 1260 */ 2389, 2413, 2811, 2418, 1750, 3378, 1890, 2734, 2426, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729,
/* 1278 */ 1729, 1729, 3031, 3028, 3399, 2787, 2787, 2787, 2790, 1823, 2078, 2845, 2067, 3071, 2389, 2413, 2811, 2418,
/* 1296 */ 1750, 3378, 1890, 2734, 2426, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3031, 3028,
/* 1314 */ 3399, 2787, 2787, 2787, 2790, 1823, 2078, 2835, 2067, 2188, 3094, 2880, 3102, 2941, 1750, 3113, 3125, 3416,
/* 1332 */ 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3031, 3028, 3399, 2787, 2787, 2787,
/* 1350 */ 2790, 1823, 2078, 2835, 2067, 2188, 2241, 3133, 2811, 2205, 1750, 2776, 1890, 3416, 3384, 3117, 2683, 1729,
/* 1368 */ 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3031, 3028, 3399, 2787, 2787, 2787, 2790, 1823, 2078, 2835,
/* 1386 */ 2067, 2188, 2241, 2880, 3154, 2205, 1750, 2776, 1890, 3416, 3384, 3164, 2683, 1729, 1729, 1729, 1729, 1729,
/* 1404 */ 1729, 1729, 1729, 1729, 3031, 3028, 3399, 2787, 2787, 2787, 2790, 1823, 2078, 2835, 2067, 2188, 2241, 2880,
/* 1422 */ 2811, 2205, 1750, 2776, 1890, 2721, 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729,
/* 1440 */ 3031, 3028, 3399, 2787, 2787, 2787, 2790, 1823, 2078, 2835, 2067, 3185, 2241, 2880, 2811, 2205, 1750, 2776,
/* 1458 */ 1890, 3416, 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3031, 3028, 3399, 2787,
/* 1476 */ 2787, 2787, 2790, 2511, 2078, 2835, 2067, 2188, 3197, 2880, 2811, 2611, 1750, 2776, 3217, 3416, 3238, 3117,
/* 1494 */ 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3031, 3028, 3399, 2787, 2787, 2787, 2790, 1823,
/* 1512 */ 2078, 2835, 2067, 2188, 2241, 2880, 2811, 2205, 1750, 3253, 1890, 3265, 3384, 3117, 2683, 1729, 1729, 1729,
/* 1530 */ 1729, 1729, 1729, 1729, 1729, 1729, 3031, 3301, 3399, 2787, 2787, 2787, 2790, 1823, 2078, 2835, 2320, 2188,
/* 1548 */ 2241, 2880, 3283, 2501, 3447, 2776, 1890, 1908, 3294, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729,
/* 1566 */ 1729, 1729, 3031, 3028, 3399, 2787, 2787, 2787, 2790, 1823, 3013, 3313, 2067, 2188, 3334, 2880, 2811, 2205,
/* 1584 */ 3371, 2776, 3392, 3416, 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 3156,
/* 1602 */ 2347, 3429, 3434, 3439, 3443, 1823, 2078, 1872, 2067, 2188, 2150, 2880, 2811, 2205, 1750, 2776, 1890, 3416,
/* 1620 */ 3384, 3117, 2683, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 2347, 1729, 1729, 1729,
/* 1638 */ 1877, 1823, 2078, 1872, 2067, 2188, 2241, 2880, 2811, 2205, 1750, 2776, 1890, 3416, 3384, 3117, 2683, 1729,
/* 1656 */ 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 2192, 1929, 3462, 3455, 3455, 3465, 1729, 1729, 1729,
/* 1674 */ 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729, 1729,
/* 1692 */ 1729, 1729, 1729, 1729, 1084, 1084, 567, 567, 567, 567, 567, 567, 567, 567, 567, 1084, 1084, 0, 0, 567, 567,
/* 1713 */ 567, 1024, 1024, 0, 0, 567, 567, 567, 1084, 1084, 2376, 2376, 567, 567, 567, 35072, 0, 0, 0, 0, 0, 0, 0, 0,
/* 1737 */ 57, 625, 0, 0, 0, 0, 0, 0, 0, 100, 649, 650, 0, 0, 0, 0, 0, 144, 134, 0, 100, 144, 144, 134, 134, 0, 100,
/* 1764 */ 663, 0, 0, 54, 0, 0, 0, 0, 0, 0, 8448, 0, 162, 163, 163, 163, 163, 163, 0, 0, 56, 56, 56, 56, 56, 56, 68, 0,
/* 1792 */ 2376, 2376, 74, 56, 56, 177, 0, 0, 0, 0, 0, 0, 0, 59, 0, 0, 0, 1024, 1024, 567, 567, 567, 567, 567, 567, 0,
/* 1818 */ 0, 567, 0, 635, 0, 0, 0, 0, 0, 0, 0, 63, 64, 649, 663, 0, 0, 0, 0, 0, 144, 134, 149, 100, 0, 162, 174, 174,
/* 1846 */ 174, 174, 174, 0, 0, 79, 0, 0, 0, 0, 79, 79, 79, 0, 79, 79, 79, 79, 0, 0, 0, 0, 0, 3584, 0, 3584, 35072, 0,
/* 1874 */ 0, 0, 96, 0, 0, 0, 0, 821, 0, 0, 0, 0, 4864, 0, 63, 64, 144, 144, 134, 134, 0, 100, 100, 0, 0, 96, 0, 85,
/* 1902 */ 598, 0, 68, 0, 0, 9728, 0, 0, 0, 144, 167, 0, 0, 0, 144, 19856, 173, 166, 144, 63, 63, 63, 63, 63, 63, 63,
/* 1928 */ 63, 0, 0, 0, 0, 0, 3072, 3072, 0, 35072, 0, 0, 0, 97, 0, 0, 0, 68, 68, 0, 0, 92, 0, 3840, 3840, 3840, 0,
/* 1955 */ 3840, 3840, 3840, 3840, 0, 0, 0, 0, 57, 0, 0, 0, 0, 4096, 4096, 0, 0, 0, 0, 4096, 4096, 4096, 4096, 0, 4096,
/* 1980 */ 4096, 4096, 4096, 4096, 4096, 4096, 4096, 0, 0, 0, 0, 67, 67, 0, 0, 0, 4352, 0, 0, 0, 0, 0, 4352, 4352, 0,
/* 2005 */ 4352, 0, 0, 4352, 4352, 4352, 4352, 4352, 4352, 4352, 4352, 0, 0, 0, 0, 77, 9043, 0, 9043, 4608, 0, 0, 0,
/* 2028 */ 4608, 4608, 4608, 4608, 0, 0, 0, 85, 35072, 6229, 0, 0, 96, 0, 0, 0, 68, 68, 90, 0, 0, 101, 637, 0, 0, 0, 0,
/* 2055 */ 0, 145, 134, 0, 150, 145, 145, 157, 134, 0, 101, 101, 0, 0, 96, 0, 100, 598, 0, 68, 0, 73, 73, 0, 0, 0, 68,
/* 2082 */ 68, 0, 0, 0, 0, 0, 0, 0, 61, 0, 145, 164, 164, 163, 164, 164, 0, 0, 96, 0, 100, 598, 0, 66816, 145, 0, 0, 0,
/* 2110 */ 0, 0, 0, 0, 3840, 0, 3840, 5120, 0, 0, 0, 5120, 5120, 5120, 5120, 0, 0, 0, 0, 85, 567, 0, 68, 0, 5376, 0,
/* 2136 */ 5376, 0, 5376, 0, 5376, 821, 0, 0, 0, 35164, 0, 0, 0, 96, 0, 0, 0, 100, 100, 100, 598, 0, 0, 0, 0, 0, 144,
/* 2163 */ 144, 144, 134, 144, 68, 0, 0, 2560, 0, 0, 0, 109, 1890, 0, 100, 100, 100, 598, 0, 0, 96, 0, 100, 598, 6656,
/* 2188 */ 68, 0, 0, 2560, 0, 0, 0, 0, 0, 3072, 0, 0, 0, 116, 0, 0, 119, 0, 0, 100, 636, 636, 636, 636, 0, 0, 135, 100,
/* 2216 */ 636, 636, 636, 636, 0, 0, 0, 0, 0, 110, 0, 112, 5632, 5632, 5632, 5632, 821, 0, 0, 0, 100, 567, 100, 615, 0,
/* 2241 */ 0, 1890, 0, 100, 100, 100, 598, 0, 0, 96, 0, 101, 598, 0, 68, 0, 2376, 2376, 0, 0, 57, 74, 5944, 56, 74, 56,
/* 2267 */ 56, 74, 56, 5962, 74, 5962, 5962, 56, 74, 5962, 74, 74, 74, 5962, 74, 74, 74, 74, 0, 0, 56, 598, 35072, 598,
/* 2291 */ 0, 0, 96, 0, 0, 0, 101, 101, 101, 625, 0, 136, 638, 0, 0, 0, 0, 0, 144, 148, 0, 100, 110, 144, 147, 147, 0,
/* 2318 */ 649, 136, 0, 0, 96, 0, 102, 598, 0, 68, 0, 2376, 2376, 0, 64, 0, 0, 172, 165, 147, 165, 165, 165, 0, 0, 97,
/* 2344 */ 0, 100, 598, 0, 68, 0, 2376, 2376, 0, 0, 0, 0, 5632, 5632, 0, 5632, 5632, 5632, 5632, 5632, 5632, 5632, 172,
/* 2367 */ 0, 0, 0, 0, 0, 0, 0, 4352, 0, 68, 0, 0, 2560, 2154, 0, 0, 0, 143, 144, 134, 0, 100, 0, 1890, 2147, 100, 100,
/* 2394 */ 100, 598, 0, 0, 101, 637, 637, 636, 637, 0, 0, 0, 0, 0, 0, 133, 110, 0, 100, 0, 2154, 0, 0, 119, 0, 122, 100,
/* 2421 */ 636, 636, 636, 636, 0, 0, 144, 163, 163, 163, 163, 163, 168, 0, 6400, 0, 0, 57, 0, 0, 0, 144, 144, 174, 177,
/* 2446 */ 144, 57, 0, 57, 57, 0, 0, 57, 57, 0, 0, 0, 0, 85, 598, 0, 68, 87, 0, 0, 0, 0, 0, 63, 64, 64, 64, 64, 64, 64,
/* 2476 */ 64, 64, 0, 0, 0, 0, 67, 0, 0, 0, 66, 68, 0, 2376, 2376, 75, 76, 66, 76, 65, 0, 76, 81, 81, 76, 0, 0, 102,
/* 2504 */ 636, 636, 636, 636, 0, 0, 130, 0, 0, 0, 0, 0, 10496, 63, 64, 82, 82, 82, 82, 81, 76, 82, 76, 76, 76, 84, 76,
/* 2531 */ 76, 76, 81, 81, 81, 81, 0, 0, 0, 0, 96, 0, 0, 0, 0, 6912, 0, 68, 68, 0, 0, 0, 0, 0, 0, 110, 6912, 0, 96,
/* 2560 */ 6912, 100, 598, 0, 68, 0, 2376, 2376, 0, 0, 1870, 0, 7936, 0, 0, 0, 0, 63, 64, 19712, 0, 100, 636, 636, 636,
/* 2585 */ 636, 0, 0, 0, 0, 132, 0, 0, 0, 176, 145, 145, 164, 145, 144, 144, 148, 19860, 0, 100, 100, 0, 0, 117, 0, 119,
/* 2611 */ 0, 0, 100, 636, 636, 636, 650, 0, 0, 173, 166, 166, 166, 19878, 166, 0, 0, 136, 638, 615, 638, 638, 0, 0, 0,
/* 2636 */ 0, 0, 0, 134, 173, 0, 0, 0, 0, 0, 0, 0, 4608, 0, 67, 68, 0, 2376, 2376, 0, 0, 67, 67, 67, 67, 0, 8704, 0, 0,
/* 2665 */ 0, 1084, 0, 0, 0, 0, 0, 1870, 0, 0, 0, 0, 7424, 7680, 144, 144, 144, 163, 144, 0, 0, 0, 0, 0, 0, 0, 1536, 64,
/* 2693 */ 0, 69, 0, 2376, 2376, 0, 0, 8448, 0, 0, 0, 8448, 8448, 8448, 8448, 0, 0, 0, 0, 100, 598, 0, 68, 7168, 0, 0,
/* 2719 */ 88, 89, 0, 0, 0, 144, 163, 0, 169, 0, 35072, 0, 94, 95, 96, 0, 0, 0, 144, 163, 168, 0, 0, 95, 7262, 96, 7262,
/* 2746 */ 100, 598, 0, 88, 89, 0, 0, 2560, 0, 0, 0, 0, 119, 0, 0, 0, 171, 144, 163, 163, 163, 163, 163, 0, 0, 142, 0,
/* 2773 */ 144, 134, 0, 100, 636, 0, 0, 0, 0, 0, 144, 147, 0, 636, 821, 821, 821, 821, 821, 821, 821, 821, 0, 0, 0, 68,
/* 2799 */ 0, 0, 2560, 2154, 107, 107, 0, 0, 635, 636, 636, 636, 636, 0, 0, 0, 0, 0, 0, 0, 63, 1536, 0, 160, 0, 144,
/* 2825 */ 163, 168, 0, 0, 0, 1084, 0, 0, 63, 64, 35072, 0, 0, 0, 96, 1890, 0, 0, 0, 2560, 0, 0, 0, 0, 96, 1890, 0, 0,
/* 2853 */ 68, 0, 0, 2560, 0, 108, 0, 0, 0, 9728, 0, 0, 0, 0, 119, 110, 0, 112, 0, 9043, 9043, 9043, 77, 9043, 9043,
/* 2878 */ 9043, 9043, 0, 0, 0, 0, 119, 0, 0, 100, 110, 0, 0, 100, 100, 100, 598, 0, 0, 635, 649, 649, 649, 649, 0, 0,
/* 2904 */ 63, 0, 63, 0, 0, 0, 0, 8960, 0, 0, 0, 65, 0, 65, 0, 0, 0, 110, 100, 636, 636, 636, 636, 0, 0, 0, 131, 0, 0,
/* 2933 */ 0, 144, 165, 0, 0, 0, 118, 119, 0, 0, 100, 636, 636, 636, 636, 139, 1870, 0, 0, 0, 1870, 0, 0, 0, 145, 164,
/* 2959 */ 0, 0, 0, 100, 100, 100, 567, 0, 1870, 1870, 1870, 1870, 0, 0, 0, 0, 119, 120, 0, 100, 35072, 0, 0, 0, 96, 0,
/* 2985 */ 0, 9216, 140, 0, 0, 0, 144, 134, 0, 100, 636, 0, 0, 0, 0, 155, 144, 144, 110, 110, 0, 100, 663, 0, 0, 64, 0,
/* 3012 */ 64, 0, 0, 0, 68, 68, 0, 91, 0, 0, 68, 70, 2376, 2376, 821, 821, 0, 0, 821, 821, 0, 0, 0, 0, 0, 0, 0, 5120, 0,
/* 3041 */ 0, 68, 71, 2376, 2376, 821, 821, 0, 0, 3584, 0, 0, 0, 0, 3584, 3584, 3584, 0, 3584, 3584, 3584, 3584, 0, 0,
/* 3065 */ 0, 0, 56, 56, 56, 0, 68, 104, 0, 2560, 2154, 0, 0, 0, 156, 157, 0, 0, 0, 0, 163, 0, 0, 0, 0, 567, 567, 567,
/* 3093 */ 1024, 0, 1890, 0, 100, 100, 100, 598, 114, 636, 0, 129, 0, 0, 0, 0, 0, 58, 0, 0, 100, 636, 0, 153, 0, 0, 0,
/* 3120 */ 144, 144, 144, 163, 144, 144, 144, 134, 134, 0, 100, 100, 159, 115, 0, 0, 0, 119, 0, 0, 100, 636, 0, 0, 0,
/* 3145 */ 154, 0, 144, 163, 163, 163, 163, 163, 110, 636, 128, 0, 0, 0, 0, 0, 0, 10752, 0, 9984, 0, 0, 144, 144, 144,
/* 3170 */ 163, 144, 144, 134, 134, 0, 100, 0, 0, 0, 163, 144, 172, 165, 174, 68, 0, 105, 2560, 0, 0, 0, 0, 119, 121, 0,
/* 3196 */ 100, 0, 1890, 0, 100, 100, 112, 598, 0, 0, 8192, 68, 8704, 0, 0, 0, 144, 163, 0, 0, 170, 144, 156, 134, 134,
/* 3221 */ 0, 100, 100, 0, 0, 9312, 0, 100, 598, 0, 68, 0, 2376, 2376, 0, 63, 0, 0, 144, 163, 163, 163, 163, 175, 0, 0,
/* 3247 */ 9472, 0, 0, 0, 63, 64, 100, 636, 152, 0, 0, 0, 0, 144, 144, 174, 175, 144, 10240, 0, 0, 144, 163, 0, 0, 0,
/* 3273 */ 156, 162, 0, 0, 0, 0, 567, 567, 567, 1084, 639, 0, 0, 0, 0, 0, 0, 0, 79, 0, 79, 0, 146, 163, 163, 163, 163,
/* 3300 */ 163, 0, 62, 821, 821, 0, 0, 0, 0, 133, 134, 0, 100, 35165, 0, 0, 0, 96, 1890, 99, 0, 68, 0, 2376, 2376, 0,
/* 3326 */ 77, 0, 9040, 77, 77, 77, 77, 9040, 111, 1890, 0, 100, 100, 100, 598, 0, 68, 0, 2376, 2376, 0, 5632, 0, 0, 96,
/* 3351 */ 0, 100, 615, 0, 68, 0, 2376, 2376, 0, 0, 59, 59, 0, 0, 0, 0, 119, 0, 0, 101, 0, 141, 0, 0, 144, 134, 0, 100,
/* 3379 */ 636, 0, 0, 2957, 0, 0, 144, 163, 163, 163, 163, 163, 0, 144, 144, 134, 134, 158, 100, 100, 0, 68, 0, 2376,
/* 3403 */ 2376, 821, 821, 0, 0, 161, 144, 163, 0, 0, 0, 144, 166, 0, 0, 0, 144, 163, 0, 0, 0, 0, 8448, 0, 8448, 0, 0,
/* 3430 */ 10752, 0, 10752, 0, 0, 10752, 10752, 10752, 0, 10752, 10752, 10752, 10752, 10752, 10752, 10752, 10752, 0, 0,
/* 3449 */ 0, 0, 146, 134, 0, 100, 3072, 3072, 3072, 3072, 3072, 3072, 3072, 3072, 0, 0, 3072, 3072, 3072, 3072, 0, 0,
/* 3471 */ 0, 0
};

const int REx::EXPECTED[] =
{
/*   0 */ 89, 99, 156, 96, 103, 110, 114, 118, 122, 126, 130, 134, 138, 142, 150, 160, 164, 167, 106, 179, 187, 203,
/*  22 */ 167, 191, 200, 144, 207, 215, 146, 213, 216, 144, 211, 215, 145, 212, 221, 217, 225, 223, 214, 221, 225, 221,
/*  44 */ 194, 234, 255, 258, 229, 233, 238, 170, 242, 173, 175, 246, 248, 252, 183, 264, 182, 196, 182, 182, 153, 182,
/*  66 */ 92, 182, 182, 182, 180, 182, 92, 182, 182, 182, 180, 182, 91, 181, 182, 182, 180, 91, 182, 182, 262, 182, 182,
/*  89 */ 4, 16777216, 0, 0, 0, 64, 128, 34, 134219778, 65538, 33554434, 1073741826, 2, 262152, -2080374782, 24, 8,
/* 106 */ 1920, 8388610, 805306368, 268435456, 1920, 813697030, 270538790, -2046820286, 278927398, 138510374, 2138278,
/* 117 */ 278927398, -1809835994, -1809835930, 278943782, 2203814, 952207398, 948013094, 952141862, 952207398,
/* 126 */ -1776281498, 278984870, 952862758, 279050406, 986417254, 280098982, 312604838, 313653414, -1125410714,
/* 135 */ 280754342, 313260198, 314308774, -1800668954, -1799620378, -1767114522, -1766065946, 4, 0, 2, 2, 2, 0, 0,
/* 149 */ 1024, 33554432, 1073741824, 0, 8, 0, 0, 8, 8, 38, 134217734, 262144, 0, 32, 32, 134217728, 67108864,
/* 166 */ (int) 0x80000000, 16, 0, 0, 512, 0, 0, 512, 17, 524, 513, 524, 524, 4096, 64, 128, 0, 0, 0, 0, 16, 335544320,
/* 188 */ 939524096, 4096, 1006632960, 1152, 805306368, 268435456, 4096, 0, 0, 2, 64, 128, 4096, 64, 128, 2, 2,
/* 205 */ 33554432, 16, 0, 1152, 268435456, 536870912, 0, 1024, 268435456, 536870912, 4096, 4096, 4096, 2, 2, 0, 4096,
/* 222 */ 4096, 4096, 4096, 0, 268435456, 536870912, 4096, 1, 0, 2, 192, 0, 32, 0, 0, 16, 17, 0, 512, 12, 524, 0, 512,
/* 245 */ 512, 524, 513, 525, 525, 525, 541, 541, 0, 2, 0, 2, 256, 0, 4, 0, 512, 0, 64, 0, 0, 256, 0
};

const wchar_t *REx::TOKEN[] =
{
  L"(0)",
  L"Whitespace",
  L"Name",
  L"Space",
  L"DirPIContents",
  L"StringLiteral",
  L"CaretName",
  L"CharCode",
  L"Char",
  L"CharRange",
  L"CharCodeRange",
  L"EOF",
  L"EquivalenceLookAhead",
  L"'$'",
  L"'&'",
  L"'('",
  L"')'",
  L"'*'",
  L"'*/'",
  L"'+'",
  L"'-'",
  L"'.'",
  L"'/'",
  L"'/*'",
  L"':'",
  L"'::='",
  L"'<<'",
  L"'<?'",
  L"'<?ENCORE?>'",
  L"'<?TOKENS?>'",
  L"'=='",
  L"'>>'",
  L"'?'",
  L"'?>'",
  L"'['",
  L"'[^'",
  L"'\\\\'",
  L"']'",
  L"'definition'",
  L"'explicit'",
  L"'ws'",
  L"'|'"
};

int main(int argc, char **argv)
{
  return REx::main(argc, argv);
}

// End
