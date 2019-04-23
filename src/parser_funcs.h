#pragma once

#include <string.h>
#include "parser_types.h"

static u32 Parse(const Parser* parser, Scanner* scanner)
{
    return parser->fn(parser, scanner);
}

static const Parser* GetChild(const Parser* parser)
{
    return (const Parser*)(parser->data);
}

static cstr GetLiteral(const Parser* parser)
{
    return (cstr)(parser->data);
}

static RuleFn GetRule(const Parser* parser)
{
    return (RuleFn)(parser->data);
}

static bool Valid(const Parser* parser)
{
    return parser->fn != 0;
}

static cstr GetCursor(const Scanner* scanner)
{
    return scanner->cursor;
}

static cstr GetOrigin(const Scanner* scanner)
{
    return scanner->origin;
}

// ----------------------------------------------------------------------------

static u32 PushToken(
    const Parser* parser,
    Scanner* scanner,
    cstr begin,
    cstr end)
{
    scanner->tokens.grow() =
    {
        begin,
        end,
        parser->id,
    };
    scanner->cursor = end;
    return 1;
}

static void PopTokens(
    Scanner* scanner,
    usize count)
{
    letmut& tokens = scanner->tokens;
    if (count < tokens.size())
    {
        tokens.m_size -= count;
        scanner->cursor = tokens.back().end;
    }
    else
    {
        tokens.clear();
        scanner->cursor = scanner->origin;
    }
}

static usize ParentBegin(
    const Parser* parser,
    Scanner* scanner)
{
    scanner->tokens.grow() =
    {
        scanner->cursor,
        scanner->cursor,
        parser->id,
    };
    return scanner->tokens.size() - 1;
}

static u32 ParentEnd(
    Scanner* scanner,
    usize parent,
    u32 total)
{
    scanner->tokens[parent].end = scanner->cursor;
    if (total == 0)
    {
        PopTokens(scanner, 1);
    }
    return total;
}

// ----------------------------------------------------------------------------

static bool IsLower(char c) { return (c >= 0x61) && (c <= 0x7A); }
static bool IsUpper(char c) { return (c >= 0x41) && (c <= 0x5A); }
static bool IsDigit(char c) { return (c >= 0x30) && (c <= 0x39); }
static bool IsWhitespace(char c) { return (c >= 0x09) && (c <= 0x20); }
static bool IsHexAlpha(char c) { return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }

static bool Is_(char c) { return c == '_'; }
static bool IsQuote(char c) { return c == '"'; }
static bool IsPlus(char c) { return c == '+'; }
static bool IsMinus(char c) { return c == '-'; }
static bool IsStar(char c) { return c == '*'; }
static bool IsSlash(char c) { return c == '/'; }
static bool IsBackslash(char c) { return c == '\\'; }
static bool IsSpace(char c) { return c == ' '; }
static bool IsTab(char c) { return c == '\t'; }
static bool IsCR(char c) { return c == '\r'; }
static bool IsLF(char c) { return c == '\n'; }

static bool IsBlank(char c) { return IsSpace(c) || IsTab(c); }
static bool IsPlusOrMinus(char c) { return IsPlus(c) || IsMinus(c); }
static bool IsArithmetic(char c) { return IsPlusOrMinus(c) || IsStar(c) || IsSlash(c); }
static bool IsNewline(char c) { return IsLF(c) || IsCR(c); }
static bool IsHexDigit(char c) { return IsDigit(c) || IsHexAlpha(c); }
static bool IsAlpha(char c) { return IsLower(c) || IsUpper(c); }
static bool IsAlphaNumeric(char c) { return IsAlpha(c) || IsDigit(c); }
static bool IsAlpha_(char c) { return IsAlpha(c) || Is_(c); }
static bool IsAlphaNumeric_(char c) { return  IsAlpha(c) || IsDigit(c) || Is_(c); }

// ----------------------------------------------------------------------------
// Basic Parsers

static u32 CharLiteral(const Parser* parser, Scanner* scanner)
{
    let lit = GetLiteral(parser);
    let begin = GetCursor(scanner);
    if (*lit == *begin)
    {
        return PushToken(parser, scanner, begin, begin + 1);
    }
    return 0;
}

static u32 CharLiteralIns(const Parser* parser, Scanner* scanner)
{
    let lit = GetLiteral(parser);
    let begin = GetCursor(scanner);
    if ((lit[0] == begin[0]) || (lit[1] == begin[0]))
    {
        return PushToken(parser, scanner, begin, begin + 1);
    }
    return 0;
}

static u32 StrLiteral(const Parser* parser, Scanner* scanner)
{
    let lit = GetLiteral(parser);
    let len = strlen(lit);
    let begin = GetCursor(scanner);
    if (strncmp(begin, lit, len) == 0)
    {
        return PushToken(parser, scanner, begin, begin + len);
    }
    return 0;
}

static u32 StrLiteralIns(const Parser* parser, Scanner* scanner)
{
    let lit = GetLiteral(parser);
    let len = strlen(lit);
    let begin = GetCursor(scanner);
    if (_strnicmp(begin, lit, len) == 0)
    {
        return PushToken(parser, scanner, begin, begin + len);
    }
    return 0;
}

static u32 RepRule(const Parser* parser, Scanner* scanner)
{
    let rule = GetRule(parser);
    let begin = GetCursor(scanner);
    letmut p = begin;
    while (rule(*p))
    {
        ++p;
    }
    if (p != begin)
    {
        return PushToken(parser, scanner, begin, p);
    }
    return 0;
}

// ----------------------------------------------------------------------------
// Meta Parsers

static u32 Optional(const Parser* parser, Scanner* scanner)
{
    let parent = ParentBegin(parser, scanner);
    let child = GetChild(parser);
    let total = Parse(child, scanner);
    scanner->tokens[parent].end = scanner->cursor;
    scanner->tokens[parent].id = total ? MetaToken_Some : MetaToken_None;
    return total + 1;
}

static u32 Repeat(const Parser* parser, Scanner* scanner)
{
    let parent = ParentBegin(parser, scanner);
    let child = GetChild(parser);

    u32 total = 0;
    u32 count = 0;
    do
    {
        count = Parse(child, scanner);
        total += count;
    } while (count);

    return ParentEnd(scanner, parent, total);
}

static u32 MatchMany(const Parser* parser, Scanner* scanner)
{
    let parent = ParentBegin(parser, scanner);
    letmut child = GetChild(parser);

    u32 total = 0;
    do
    {
        total += Parse(child, scanner);
        ++child;
    } while (Valid(child));

    return ParentEnd(scanner, parent, total);
}

static u32 MatchOne(const Parser* parser, Scanner* scanner)
{
    let parent = ParentBegin(parser, scanner);
    letmut child = GetChild(parser);

    u32 total = 0;
    do
    {
        total += Parse(child, scanner);
        ++child;
    } while (!total && Valid(child));

    return ParentEnd(scanner, parent, total);
}

static u32 MatchAll(const Parser* parser, Scanner* scanner)
{
    let parent = ParentBegin(parser, scanner);
    letmut child = GetChild(parser);

    u32 total = 0;
    u32 count = 0;
    do
    {
        count = Parse(child, scanner);
        total += count;
        ++child;
    } while (count && Valid(child));

    if (!count)
    {
        PopTokens(scanner, total);
        total = 0;
    }

    return ParentEnd(scanner, parent, total);
}
