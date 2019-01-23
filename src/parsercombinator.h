#pragma once

#include <stdint.h>
#include <string.h>
#include "macro.h"
#include "array.h"
#include "fnv.h"

namespace ParComb
{

static inline bool IsWhitespace(const char* p)
{
    switch(*p)
    {
        case ' ':
        case '\r':
        case '\t':
        case '\n':
        return true;
    }
    return false;
}

static inline bool IsAlpha(const char* p)
{
    return (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z');
}

static inline bool IsDigit(const char* p)
{
    return *p >= '0' && *p <= '9';
}

struct Scanner
{
    const char* src;

    inline bool AtEnd() const 
    {
        return *src == '\0';
    }
    void SkipWhitespace()
    {
        const char* start = src;
        const char* p = start;

        bool loop = true;
        while(loop)
        {
            switch(p[0])
            {
                case ' ':
                case '\r':
                case '\t':
                case '\n':
                    ++p;
                    break;
                case ';':
                    while(p[0] && p[0] != '\n')
                    {
                        ++p;
                    }
                    break;
                case '/':
                    if(p[1] == '/')
                    {
                        while(p[0] && p[0] != '\n')
                        {
                            ++p;
                        }
                    }
                    else if(p[1] == '*')
                    {
                        while(p[0])
                        {
                            if(p[0] == '*' && p[1] == '/')
                            {
                                p += 2;
                                break;
                            }
                            else 
                            {
                                ++p;
                            }
                        }
                    }
                    else
                    {
                        loop = false;
                    }
                break;
                default:
                    loop = false;
                break;
            }
        }

        src = p;
    }
};

enum TokenName
{
    TN_CharLiteral = 0,
    TN_StringLiteral,
    TN_IntegralLiteral,
    TN_FloatLiteral,
    TN_Char,
    TN_String,
    TN_Quote,
    TN_Integral,
    TN_Float,
    TN_Whitespace,
    TN_Keyword,
    TN_Expression,
    TN_Declaration,

    TN_Identifier,

    TN_LPAREN,
    TN_RPAREN,
    TN_LBRACE,
    TN_RBRACE,
    TN_LBRACKET,
    TN_RBRACKET,
    TN_SEMICOLON,
    TN_COMMA,
    TN_PERIOD,
    TN_MINUS,
    TN_PLUS,
    TN_STAR,
    TN_SLASH,
    TN_BANG,
    TN_BANGEQ,
    TN_EQ,
    TN_EQEQ,
    TN_LANGBRACK,
    TN_RANGBRACK,
    TN_LANGBRACKEQ,
    TN_RANGBRACKEQ,
    
    TN_VAR,
    TN_IF,
    TN_ELSE,
    TN_AND,
    TN_OR,
    TN_FOR,
    TN_WHILE,
    TN_FUNC,
    TN_RET,
    TN_TRUE,
    TN_FALSE,
    TN_NIL,
    TN_STRUCT,
    TN_SUPER,
    TN_THIS,
    TN_PRINT,
};

struct Token
{
    const char*     start;
    int32_t         len;
    int32_t         name;
};

inline int32_t CreateToken(
    Array<Token>&   tokens,
    const char*     start,
    int32_t         len,
    int32_t         name)
{
    Token* t = &tokens.grow();
    t->start = start;
    t->len = len;
    t->name = name;
    return 1;
}

inline void FreeTokens(
    Array<Token>&   tokens, 
    int32_t         count)
{
    Assert(tokens.count() >= count);
    tokens.m_count -= count;
}

struct Parser
{
    // returns number of tokens added to array
    virtual int32_t Parse(Array<Token>& tokens, Scanner& sc) const = 0;
};

struct Char : public Parser 
{
    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        sc.SkipWhitespace();
        if(sc.AtEnd())
        {
            return 0;
        }
        const char* p = sc.src;
        if(*p)
        {
            ++sc.src;
            return CreateToken(tokens, p, 1, TN_Char);
        }
        return 0;
    }
};

struct String : public Parser 
{
    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        sc.SkipWhitespace();
        if(sc.AtEnd())
        {
            return 0;
        }
        const char* start = sc.src;
        const char* p = start;
        if(*p)
        {
            while(IsAlpha(p) || IsDigit(p))
            {
                ++p;
            }
            if(p != start)
            {
                sc.src = p;
                return CreateToken(tokens, start, p - start, TN_String);
            }
        }
        return 0;
    }
};

struct Integral : public Parser
{
    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        sc.SkipWhitespace();
        if(sc.AtEnd())
        {
            return 0;
        }
        const char* start = sc.src;
        const char* x = sc.src;
        while(*x >= '0' && *x <= '9')
        {
            ++x;
        }
        if(x != sc.src)
        {
            sc.src = x;
            int32_t len = x - start;
            return CreateToken(tokens, start, len, TN_Integral);
        }
        return 0;
    }
};

struct Float : public Parser
{    
    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        sc.SkipWhitespace();
        if(sc.AtEnd())
        {
            return 0;
        }
        const char* start = sc.src;
        const char* x = sc.src;
        while(*x >= '0' && *x <= '9')
        {
            ++x;
        }
        if(*x == '.')
        {
            const char* dot = x;
            ++x;
            while(*x >= '0' && *x <= '9')
            {
                ++x;
            }
            if(x != sc.src && x != dot)
            {
                sc.src = x;
                int32_t len = x - start;
                return CreateToken(tokens, start, len, TN_Float);
            }
        }
        return 0;
    }
};

struct Quote : public Parser 
{
    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        sc.SkipWhitespace();
        if(sc.AtEnd())
        {
            return 0;
        }
        const char* start = sc.src;
        const char* p = start;
        if(*p == '"')
        {
            ++p;
            while(*p && *p != '"')
            {
                ++p;
            }
            if(*p == '"')
            {
                ++p;
            }
            else 
            {
                return -1;
            }
            if(p != start)
            {
                int32_t len = p - start;
                sc.src = p;
                return CreateToken(tokens, start, len, TN_Quote);
            }
        }
        return 0;
    }
};

struct CharLiteral : public Parser
{
    char c;

    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        sc.SkipWhitespace();
        if(sc.AtEnd())
        {
            return 0;
        }
        const char* start = sc.src;
        if(*start == c)
        {
            ++sc.src;
            return CreateToken(tokens, start, 1, TN_CharLiteral);
        }
        return 0;
    }
};

struct StringLiteral : public Parser
{
    const char* lit;

    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        const char* start = sc.src;
        String p0;
        int32_t count = p0.Parse(tokens, sc);
        if(count)
        {
            int32_t len = strlen(lit);
            if(len == tokens.back().len && 
                memcmp(lit, tokens.back().start, len) == 0)
            {
                return count;
            }
            sc.src = start;
            FreeTokens(tokens, count);
        }
        return 0;
    }
};

struct Keyword : public Parser
{
    const char* lit;
    uint64_t    hash;
    int32_t     name;
    int32_t     len;

    Keyword(const char* x, int32_t y)
    {
        lit = x;
        hash = Fnv64(x);
        name = y;
        len = strlen(lit);
    }
    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        const char* start = sc.src;
        StringLiteral p0;
        p0.lit = lit;

        int32_t count = p0.Parse(tokens, sc);
        if(count)
        {
            tokens.back().name = name;
            return count;
        }

        sc.src = start;
        return 0;
    }
};

struct IntegralLiteral : public Parser
{
    int32_t lit;

    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        const char* start = sc.src;
        Integral p0;
        int32_t count = p0.Parse(tokens, sc);
        if(count)
        {
            int32_t value = atoi(tokens.back().start);
            if(value == lit)
            {
                tokens.back().name = TN_IntegralLiteral;
                return count;
            }
            FreeTokens(tokens, count);
        }
        sc.src = start;
        return 0;
    }
};

struct FloatLiteral : public Parser
{
    float lit;

    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        const char* start = sc.src;
        Float p0;
        int32_t count = p0.Parse(tokens, sc);
        if(count)
        {
            float value = (float)atof(tokens.back().start);
            if(value == lit)
            {
                tokens.back().name = TN_FloatLiteral;
                return count;
            }
            FreeTokens(tokens, count);
        }
        sc.src = start;
        return 0;
    }
};

// returns result of first matching parser
struct Or : public Parser
{
    const Parser* p0;
    const Parser* p1;

    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        int32_t count0 = p0->Parse(tokens, sc);
        if(count0)
        {
            return count0;
        }
        int32_t count1 = p1->Parse(tokens, sc);
        if(count1)
        {
            return count1;
        }
        return 0;
    }
};

// returns combined results if both match
struct And : public Parser
{
    const Parser* p0;
    const Parser* p1;

    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        const char* start = sc.src;
        int32_t count0 = p0->Parse(tokens, sc);
        if(count0 == 0)
        {
            sc.src = start;
            return 0;
        }
        int32_t count1 = p1->Parse(tokens, sc);
        if(count1 == 0)
        {
            FreeTokens(tokens, count0);
            sc.src = start;
            return 0;
        }
        return count0 + count1;
    }
};

// returns result of first matching parser
struct Any : public Parser
{
    const Parser*   parsers;
    size_t          count;

    inline Any(const Parser* a, size_t b)
    {
        parsers = a;
        count = b;
    }
    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final
    {
        const char* start = sc.src;
        for(size_t i = 0; i < count; ++i)
        {
            int32_t ct = parsers[i].Parse(tokens, sc);
            if(ct)
            {
                return ct;
            }
        }
        sc.src = start;
        return 0;
    }
};

// returns all results if all parsers match
struct All : public Parser
{
    const Parser*   parsers;
    size_t          count;

    inline All(const Parser* a, size_t b)
    {
        parsers = a;
        count = b;
    }
    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final
    {
        const char* start = sc.src;
        int32_t ct = 0;
        for(size_t i = 0; i < count; ++i)
        {
            int32_t cti = parsers[i].Parse(tokens, sc);
            if(cti)
            {
                ct += cti;
            }
            else
            {
                FreeTokens(tokens, ct);
                sc.src = start;
                return 0;
            }
        }
        return ct;
    }
};

// returns result of middle parser if all 3 match
struct Between : public Parser
{
    const Parser* p0;
    const Parser* p1;
    const Parser* p2;

    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        const char* start = sc.src;
        int32_t count0 = p0->Parse(tokens, sc);
        if(!count0)
        {
            sc.src = start;
            return 0;
        }
        FreeTokens(tokens, count0);

        int32_t count1 = p1->Parse(tokens, sc);
        if(!count1)
        {
            sc.src = start;
            return 0;
        }
        
        int32_t count2 = p2->Parse(tokens, sc);
        if(!count2)
        {
            FreeTokens(tokens, count1);
            sc.src = start;
            return 0;
        }
        FreeTokens(tokens, count2);

        return count1;
    }
};

// returns result of first parser if both match
struct First : public Parser
{
    const Parser* p0;
    const Parser* p1;

    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        const char* start = sc.src;
        int32_t count0 = p0->Parse(tokens, sc);
        if(!count0)
        {
            sc.src = start;
            return 0;
        }
        int32_t count1 = p1->Parse(tokens, sc);
        if(!count1)
        {
            FreeTokens(tokens, count0);
            sc.src = start;
            return 0;
        }
        FreeTokens(tokens, count1);

        return count0;
    }
};

// returns result of second parser if both match
struct Second : public Parser
{
    const Parser* p0;
    const Parser* p1;

    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        const char* start = sc.src;
        int32_t count0 = p0->Parse(tokens, sc);
        if(!count0)
        {
            sc.src = start;
            return 0;
        }
        FreeTokens(tokens, count0);

        int32_t count1 = p1->Parse(tokens, sc);
        if(!count1)
        {
            sc.src = start;
            return 0;
        }

        return count1;
    }
};

static const Keyword ms_keywords[] = 
{
    Keyword("(", TN_LPAREN),
    Keyword(")", TN_RPAREN),
    Keyword("{", TN_LBRACE),
    Keyword("}", TN_RBRACE),
    Keyword("[", TN_LBRACKET),
    Keyword("]", TN_RBRACKET),
    Keyword(";", TN_SEMICOLON),
    Keyword(",", TN_COMMA),
    Keyword(".", TN_PERIOD),
    Keyword("-", TN_MINUS),
    Keyword("+", TN_PLUS),
    Keyword("*", TN_STAR),
    Keyword("/", TN_SLASH),
    Keyword("!", TN_BANG),
    Keyword("!=", TN_BANGEQ),
    Keyword("=", TN_EQ),
    Keyword("==", TN_EQEQ),
    Keyword("<", TN_LANGBRACK),
    Keyword(">", TN_RANGBRACK),
    Keyword("<=", TN_LANGBRACKEQ),
    Keyword(">=", TN_RANGBRACKEQ),
    Keyword("var", TN_VAR),
    Keyword("if", TN_IF),
    Keyword("else", TN_ELSE),
    Keyword("and", TN_AND),
    Keyword("or", TN_OR),
    Keyword("for", TN_FOR),
    Keyword("while", TN_WHILE),
    Keyword("func", TN_FUNC),
    Keyword("return", TN_RET),
    Keyword("true", TN_TRUE),
    Keyword("false", TN_FALSE),
    Keyword("nil", TN_NIL),
    Keyword("struct", TN_STRUCT),
    Keyword("super", TN_SUPER),
    Keyword("this", TN_THIS),
    Keyword("print", TN_PRINT),
};

struct ReservedWords : public Parser
{
    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        const char* start = sc.src;
        String str;
        int32_t ct = str.Parse(tokens, sc);
        if(ct == 1)
        {
            Token& tok = tokens.back();
            uint64_t hash = Fnv64(tok.start, tok.len);
            for(const Keyword& kw : ms_keywords)
            {
                if( kw.hash == hash     &&  
                    kw.len  == tok.len  && 
                    memcmp(kw.lit, tok.start, tok.len) == 0)
                {
                    tok.name = kw.name;
                    return 1;
                }
            }
        }
        FreeTokens(tokens, ct);
        sc.src = start;
        return 0;
    }
};

struct Identifier : public Parser
{
    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        sc.SkipWhitespace();
        if(sc.AtEnd())
        {
            return 0;
        }
        // all identifiers start with an alphabetical character
        if(!IsAlpha(sc.src))
        {
            return 0;
        }

        const ReservedWords rw;
        const String str;
        int32_t ctrw = rw.Parse(tokens, sc);
        if(ctrw)
        {
            return ctrw;
        }

        int32_t ctstr = str.Parse(tokens, sc);
        if(ctstr)
        {
            tokens.back().name = TN_Identifier;
        }
        return ctstr;
    }
};

struct TopLevel : public Parser
{
    inline int32_t Parse(Array<Token>& tokens, Scanner& sc) const final 
    {
        const Identifier ident;
        const Quote quote;
        const Integral integral;
        const Float flt;
        int32_t ct = 0;
        while(!sc.AtEnd())
        {
            int32_t ctPre = ct;
            {
                ct += ident.Parse(tokens, sc);
                ct += quote.Parse(tokens, sc);
                ct += integral.Parse(tokens, sc);
                ct += flt.Parse(tokens, sc);
            }
            if(ct == ctPre)
            {
                break;
            }
        }
        return ct;
    }
};

}; // ParComb