#pragma once

#include "parser_funcs.h"

#define ParDef constant Parser

ParDef NullParser = { 0, 0, 0 };

enum RuleID
{
    RuleID_Begin = MetaToken_Count,

    Rule_Blank,
    Rule_Newline,
    Rule_Whitespace,
    Rule_Upper,
    Rule_Lower,
    Rule_Alpha,
    Rule_Digit,
    Rule_HexDigit,
    Rule_AlphaNumeric,
    Rule_Alpha_,
    Rule_AlphaNumeric_,

    RuleID_End
};

namespace Rule
{
    constant ParseFn Fn = RepRule;

    ParDef Blank{ Fn, Rule_Blank, IsBlank };
    ParDef Whitespace{ Fn, Rule_Whitespace, IsWhitespace };
    ParDef Upper{ Fn, Rule_Upper, IsUpper };
    ParDef Lower{ Fn, Rule_Lower, IsLower };
    ParDef Alpha{ Fn, Rule_Alpha, IsAlpha };
    ParDef Digit{ Fn, Rule_Digit, IsDigit };
    ParDef HexDigit{ Fn, Rule_HexDigit, IsHexDigit };
    ParDef AlphaNumeric{ Fn, Rule_AlphaNumeric, IsAlphaNumeric };
    ParDef Alpha_{ Fn, Rule_Alpha_, IsAlpha_ };
    ParDef AlphaNumeric_{ Fn, Rule_AlphaNumeric_, IsAlphaNumeric_ };
};

enum CharLitID
{
    CharLitID_Begin = RuleID_End,

    CharLit_LBracket,
    CharLit_RBracket,
    CharLit_LParen,
    CharLit_RParen,
    CharLit_LBrace,
    CharLit_RBrace,
    CharLit_Period,
    CharLit_Comma,
    CharLit_Colon,
    CharLit_Semicolon,
    CharLit_Quote,
    CharLit_Apostrophe,
    CharLit_LT,
    CharLit_GT,
    CharLit_Plus,
    CharLit_Minus,
    CharLit_Slash,
    CharLit_Star,
    CharLit_Backslash,

    CharLit_Zero,

    CharLitID_End
};

namespace CharLit
{
    constant ParseFn Fn = CharLiteral;

    ParDef LBracket{ Fn, CharLit_LBracket, "[" };
    ParDef RBracket{ Fn, CharLit_RBracket, "]" };
    ParDef LParen{ Fn, CharLit_LParen, "(" };
    ParDef RParen{ Fn, CharLit_RParen, ")" };
    ParDef LBrace{ Fn, CharLit_LBrace, "{" };
    ParDef RBrace{ Fn, CharLit_RBrace, "}" };
    ParDef Period{ Fn, CharLit_Period, "." };
    ParDef Comma{ Fn, CharLit_Comma, "," };
    ParDef Colon{ Fn, CharLit_Colon, ":" };
    ParDef Semicolon{ Fn, CharLit_Semicolon, ";" };
    ParDef Quote{ Fn, CharLit_Quote, "\"" };
    ParDef Apostrophe{ Fn, CharLit_Apostrophe, "'" };
    ParDef LT{ Fn, CharLit_LT, "<" };
    ParDef GT{ Fn, CharLit_GT, ">" };
    ParDef Plus{ Fn, CharLit_Plus, "+" };
    ParDef Minus{ Fn, CharLit_Minus, "-" };
    ParDef Slash{ Fn, CharLit_Slash, "/" };
    ParDef Star{ Fn, CharLit_Star, "*" };
    ParDef Backslash{ Fn, CharLit_Backslash, "\\" };

    ParDef Zero{ Fn, CharLit_Zero, "0" };
};

enum CharLitInsID
{
    CharLitInsID_Begin = CharLitID_End,

    CharLitIns_E,
    CharLitIns_X,

    CharLitInsID_End
};

namespace CharLitIns
{
    constant ParseFn Fn = CharLiteralIns;

    ParDef E{ Fn, CharLitIns_E, "eE" };
    ParDef X{ Fn, CharLitIns_X, "xX" };
};

enum StrLitID
{
    StrLitID_Begin = CharLitInsID_End,

    StrLitID_End
};

namespace StrLit
{
    constant ParseFn Fn = StrLiteral;

};

enum StrLitInsID
{
    StrLitInsID_Begin = StrLitID_End,

    StrLitInsID_End
};

namespace StrLitIns
{
    constant ParseFn Fn = StrliteralIns;

};

enum PatternID
{
    PatternID_Begin = StrLitInsID_End,

    Pattern_Sign,
    Pattern_Integer,
    Pattern_Hex,
    Pattern_Exponent,
    Pattern_Float,
    Pattern_Number,
    Pattern_Index,
    Pattern_VariableName,

    PatternID_End
};

namespace Pattern
{
    ParDef SignPattern[] =
    {
        CharLit::Plus,
        CharLit::Minus,
        NullParser,
    };
    ParDef Sign{ MatchOne, Pattern_Sign, SignPattern };

    ParDef IntegerPattern[] =
    {
        { Optional, 0, &Sign },
        Rule::Digit,
        NullParser,
    };
    ParDef Integer{ MatchAll, Pattern_Integer, IntegerPattern };

    ParDef HexPattern[] =
    {
        CharLit::Zero,
        CharLitIns::X,
        Rule::HexDigit,
        NullParser,
    };
    ParDef Hex{ MatchAll, Pattern_Hex, HexPattern };

    ParDef ExponentPattern[] =
    {
        CharLitIns::E,
        Sign,
        Rule::Digit,
        NullParser,
    };
    ParDef Exponent{ MatchAll, Pattern_Exponent, ExponentPattern };

    ParDef FloatPattern[] =
    {
        { Optional, 0, &Sign },
        Rule::Digit,
        CharLit::Period,
        { Optional, 0, &Rule::Digit },
        { Optional, 0, &Exponent },
        NullParser,
    };
    ParDef Float{ MatchAll, Pattern_Float, FloatPattern };

    ParDef NumberPattern[] =
    {
        Integer,
        Hex,
        Float,
        NullParser,
    };
    ParDef Number{ MatchOne, Pattern_Number, NumberPattern };

    ParDef IndexPattern[] =
    {
        CharLit::LBracket,
        Number,
        CharLit::RBracket,
        NullParser,
    };
    ParDef Index{ MatchAll, Pattern_Index, IndexPattern };

    ParDef VariableNamePattern[] =
    {
        Rule::Alpha_,
        { Optional, 0, &Rule::AlphaNumeric_ },
        NullParser,
    };
    ParDef VariableName{ MatchAll, Pattern_VariableName, VariableNamePattern };
};
