#include "compiler.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "array.h"
#include "fnv.h"

using namespace ScriptTypes;

static inline bool IsDigit(char c)
{
    return c >= '0' && c <= '9';
}

static inline bool IsAlpha(char c)
{
    return 
        (c >= 'a' && c <= 'z') || 
        (c >= 'A' && c <= 'Z') || 
        (c == '_');
}

namespace Compiler
{

enum TokenType : uint8_t
{
    TT_LPAREN = 0,
    TT_RPAREN,
    TT_LBRACE,
    TT_RBRACE,
    TT_COMMA,
    TT_PERIOD,
    TT_MINUS,
    TT_PLUS,
    TT_SEMICOLON,
    TT_SLASH,
    TT_STAR,

    TT_BANG,
    TT_BANG_EQ,
    TT_EQ,
    TT_EQEQ,
    TT_GT,
    TT_GTE,
    TT_LT,
    TT_LTE,

    TT_IDENT,
    TT_STRING,
    TT_NUMBER,

    TT_AND,
    TT_STRUCT,
    TT_ELSE,
    TT_FOR,
    TT_FUN,
    TT_IF,
    TT_NIL,
    TT_OR,
    TT_PRINT,
    TT_RET,
    TT_SUPER,
    TT_THIS,
    TT_TRUE,
    TT_FALSE,
    TT_VAR,
    TT_WHILE,

    TT_ERROR,
    TT_EOF,
};

struct Token
{
    const char*     start;
    uint16_t        len;
    uint16_t        line;
    TokenType       type;
};

static const char* ReservedWords[] = 
{
    "and",
    "struct",
    "else",
    "if",
    "nil",
    "or",
    "print",
    "return",
    "super",
    "var",
    "while",
    "false",
    "for",
    "fun",
    "this",
    "true",
};
static const TokenType ReservedTokens[] = 
{
    TT_AND,
    TT_STRUCT,
    TT_ELSE,
    TT_IF,
    TT_NIL,
    TT_OR,
    TT_PRINT,
    TT_RET,
    TT_SUPER,
    TT_VAR,
    TT_WHILE,
    TT_FALSE,
    TT_FOR,
    TT_FUN,
    TT_THIS,
    TT_TRUE,
};
static uint64_t ReservedHashes[NELEM(ReservedWords)];

inline void InitReservedWords()
{
    for(int32_t i = 0; i < NELEM(ReservedWords); ++i)
    {
        ReservedHashes[i] = Fnv64(ReservedWords[i]);
    }
}

struct Scanner
{
    const char* src;
    const char* start;
    const char* current;
    uint16_t    line;

    inline void Init(const char* source)
    {
        MemZero(*this);
        src = source;
        InitReservedWords();
    }
    inline Token MakeError(const char* msg) const 
    {
        Token t;
        MemZero(t);
        t.type = TT_ERROR;
        t.line = line;
        t.start = msg;
        return t;
    }
    inline Token MakeToken(TokenType type) const
    {
        Token t;
        MemZero(t);
        t.type = type;
        t.line = line;
        t.start = start;
        t.len = current - start;
        return t;
    }
    inline char Advance() 
    {
        char c = *current;
        ++current;
        return c;
    }
    inline bool Match(char expected) 
    {
        if(*current != expected)
        {
            return false;
        }
        ++current;
        return true;
    }
    inline char Peek() const 
    {
        return *current;
    }
    inline bool AtEnd() const 
    {
        return *current == '\0';
    }
    inline char PeekNext() const 
    {
        if(AtEnd())
        {
            return '\0';
        }
        return current[1];
    }
    Token String()
    {
        while(Peek() != '"' && !AtEnd())
        {
            if(Peek() == '\n')
            {
                ++line;
            }
            Advance();
        }
        if(AtEnd())
        {
            return MakeError("Unterminated string.");
        }
        Advance();
        return MakeToken(TT_STRING);
    }
    Token Number()
    {
        while(IsDigit(Peek()))
        {
            Advance();
        }
        if(Peek() == '.')
        {
            if(IsDigit(PeekNext()))
            {
                Advance();
                while(IsDigit(Peek()))
                {
                    Advance();
                }
                return MakeToken(TT_NUMBER);
            }
            return MakeError("Unexpected character.");
        }
        return MakeToken(TT_NUMBER);
    }
    inline TokenType IdentifierType() const
    {
        const uint16_t len = current - start;
        const uint64_t hash = Fnv64(start, len);
        for(int32_t i = 0; i < NELEM(ReservedWords); ++i)
        {
            if(ReservedHashes[i] == hash && 
                memcmp(start, ReservedWords[i], len) == 0)
            {
                return ReservedTokens[i];
            }
        }
        return TT_IDENT;
    }
    inline Token Identifier()
    {
        while(IsAlpha(Peek()) || IsDigit(Peek()))
        {
            Advance();
        }
        return MakeToken(IdentifierType());
    }
    inline void SkipWhitespace()
    {
        while(1)
        {
            char c = Peek();
            switch(c)
            {
                case ' ':
                case '\r':
                case '\t':
                    Advance();
                    break;
                case '\n':
                    ++line;
                    Advance();
                    break;
                case '/':
                    if(PeekNext() == '/')
                    {
                        while(Peek() != '\n' && !AtEnd())
                        {
                            Advance();
                        }
                    }
                    else
                    {
                        return;
                    }
                default:
                    return;
            }
        }
    }
    Token ScanToken()
    {
        start = current;

        char c = Advance();
        if(AtEnd())
        {
            return MakeToken(TT_EOF);
        }
        if(IsAlpha(c))
        {
            return Identifier();
        }
        if(IsDigit(c))
        {
            return Number();
        }
        switch(c)
        {
            case '(': return MakeToken(TT_LPAREN);
            case ')': return MakeToken(TT_RPAREN);
            case '{': return MakeToken(TT_LBRACE);
            case '}': return MakeToken(TT_RBRACE);
            case ';': return MakeToken(TT_SEMICOLON);
            case ',': return MakeToken(TT_COMMA);
            case '.': return MakeToken(TT_PERIOD);
            case '-': return MakeToken(TT_MINUS);
            case '+': return MakeToken(TT_PLUS);
            case '/': return MakeToken(TT_SLASH);
            case '*': return MakeToken(TT_STAR);
            case '!':
                return MakeToken(Match('=') ? TT_BANG_EQ : TT_BANG);
            case '=':
                return MakeToken(Match('=') ? TT_EQEQ : TT_EQ);
            case '<':
                return MakeToken(Match('=') ? TT_LTE : TT_LT);
            case '>':
                return MakeToken(Match('=') ? TT_GTE : TT_GT);
            case '"':
                return String();
        }

        return MakeError("Unexpected character.");
    }
};

struct Parser
{
    Token previous;
    Token current;
    bool hadError;
    bool panicMode;
};

enum Precedence
{
    PREC_NONE = 0,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY,
};

enum ParseFn
{
    NONE = 0,
    PF_GROUPING,
    PF_UNARY,
    PF_BINARY, 
    PF_FLOAT,
    PF_INT,
};

struct ParseRule
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
};

static const ParseRule rules[] = 
{
    { PF_GROUPING, NONE,    PREC_CALL },       // TOKEN_LEFT_PAREN      
    { NONE,     NONE,    PREC_NONE },       // TOKEN_RIGHT_PAREN     
    { NONE,     NONE,    PREC_NONE },       // TOKEN_LEFT_BRACE
    { NONE,     NONE,    PREC_NONE },       // TOKEN_RIGHT_BRACE     
    { NONE,     NONE,    PREC_NONE },       // TOKEN_COMMA           
    { NONE,     NONE,    PREC_CALL },       // TOKEN_DOT             
    { PF_UNARY,    PF_BINARY,  PREC_TERM },       // TOKEN_MINUS           
    { NONE,     PF_BINARY,  PREC_TERM },       // TOKEN_PLUS            
    { NONE,     NONE,    PREC_NONE },       // TOKEN_SEMICOLON       
    { NONE,     PF_BINARY,  PREC_FACTOR },     // TOKEN_SLASH           
    { NONE,     PF_BINARY,  PREC_FACTOR },     // TOKEN_STAR            
    { NONE,     NONE,    PREC_NONE },       // TOKEN_BANG            
    { NONE,     NONE,    PREC_EQUALITY },   // TOKEN_BANG_EQUAL      
    { NONE,     NONE,    PREC_NONE },       // TOKEN_EQUAL           
    { NONE,     NONE,    PREC_EQUALITY },   // TOKEN_EQUAL_EQUAL     
    { NONE,     NONE,    PREC_COMPARISON }, // TOKEN_GREATER         
    { NONE,     NONE,    PREC_COMPARISON }, // TOKEN_GREATER_EQUAL   
    { NONE,     NONE,    PREC_COMPARISON }, // TOKEN_LESS            
    { NONE,     NONE,    PREC_COMPARISON }, // TOKEN_LESS_EQUAL      
    { NONE,     NONE,    PREC_NONE },       // TOKEN_IDENTIFIER      
    { NONE,     NONE,    PREC_NONE },       // TOKEN_STRING          
    { PF_FLOAT,    NONE,    PREC_NONE },       // TOKEN_FLOAT          
    { PF_INT,      NONE,    PREC_NONE },       // TOKEN_INT     
    { NONE,     NONE,    PREC_AND },        // TOKEN_AND             
    { NONE,     NONE,    PREC_NONE },       // TOKEN_CLASS           
    { NONE,     NONE,    PREC_NONE },       // TOKEN_ELSE            
    { NONE,     NONE,    PREC_NONE },       // TOKEN_FALSE           
    { NONE,     NONE,    PREC_NONE },       // TOKEN_FOR             
    { NONE,     NONE,    PREC_NONE },       // TOKEN_FUN             
    { NONE,     NONE,    PREC_NONE },       // TOKEN_IF              
    { NONE,     NONE,    PREC_NONE },       // TOKEN_NIL             
    { NONE,     NONE,    PREC_OR },         // TOKEN_OR              
    { NONE,     NONE,    PREC_NONE },       // TOKEN_PRINT           
    { NONE,     NONE,    PREC_NONE },       // TOKEN_RETURN          
    { NONE,     NONE,    PREC_NONE },       // TOKEN_SUPER           
    { NONE,     NONE,    PREC_NONE },       // TOKEN_THIS            
    { NONE,     NONE,    PREC_NONE },       // TOKEN_TRUE            
    { NONE,     NONE,    PREC_NONE },       // TOKEN_VAR             
    { NONE,     NONE,    PREC_NONE },       // TOKEN_WHILE           
    { NONE,     NONE,    PREC_NONE },       // TOKEN_ERROR           
    { NONE,     NONE,    PREC_NONE },       // TOKEN_EOF             
}; 

struct Compiler
{
    Scanner         scanner;
    Parser          parser;
    CompileResult*  pResult;

    void Error(Token token, const char* msg = nullptr)
    {
        if(parser.panicMode)
        {
            return;
        }
        parser.panicMode = true;
        fprintf(stderr, "[line %d] Error:", token.line);
        if(token.start)
        {
            fprintf(stderr, " %s ", token.start);
        }
        if(msg)
        {
            fprintf(stderr, " %s ", msg);
        }
        fprintf(stderr, " at %.*s", token.len, token.start);
        parser.hadError = true;
    }
    void Consume(TokenType type, const char* msg)
    {
        if(parser.current.type == type)
        {
            Advance();
            return;
        }
        Error(parser.current, msg);
    }
    void Advance()
    {
        parser.previous = parser.current;

        while(1)
        {
            parser.current = scanner.ScanToken();
            if(parser.current.type != TT_ERROR)
            {
                break;
            }
            Error(parser.current);
        }
    }
    void EmitByte(uint8_t x)
    {
        pResult->code.grow() = x;
        //pResult->m_lineNos.grow() = parser.previous.line;
    }
    void EmitBytes(uint8_t a, uint8_t b)
    {
        EmitByte(a);
        EmitByte(b);
    }
    void EmitValue(Value value)
    {
        const uint8_t* v = (const uint8_t*)&value;
        for(int32_t i = 0; i < sizeof(value); ++i)
        {
            EmitByte(v[i]);
        }
    }
    void Number()
    {
        double f64 = strtod(parser.previous.start, nullptr);
        EmitByte(Op::PUSH);
        EmitValue(f64);
    }
    void Grouping()
    {
        Expression();
        Consume(TT_RPAREN, "Expected ')' after expression.");
    }
    void Unary()
    {
        TokenType opType = parser.previous.type;
        ParsePrecedence(PREC_UNARY);
        switch(opType)
        {
            case TT_MINUS:
                EmitByte(Op::NEG);
            break;
        }
    }
    void Binary()
    {
        TokenType opType = parser.previous.type;

        ParseRule rule = rules[opType];
        ParsePrecedence((Precedence)(rule.precedence + 1));

        switch(opType)
        {
            case TT_PLUS: EmitByte(Op::ADD); break;
            case TT_MINUS: EmitByte(Op::SUB); break;
            case TT_STAR: EmitByte(Op::MUL); break;
            case TT_SLASH: EmitByte(Op::DIV); break;
        }
    }
    void ParsePrecedence(Precedence prec)
    {

    }
    void EmitReturn()
    {
        EmitByte(Op::RET);
    }
    void EndCompiler()
    {
        EmitReturn();
    }
    void Expression()
    {
        ParsePrecedence(PREC_ASSIGNMENT);
    }
    bool Compile(const char* src, CompileResult& result)
    {
        MemZero(*this);
        pResult = &result;
        result.code.clear();
        scanner.Init(src);

        while(1)
        {
            Token token = scanner.ScanToken();
            if(token.type == TT_EOF)
            {
                break;
            }
        }

        Consume(TT_EOF, "Expect end of expression.");
        EndCompiler();
        return !parser.hadError;
    }
};

bool Compile(const char* src, CompileResult& result)
{
    Compiler c;
    return c.Compile(src, result);
}

}; // namespace Compiler