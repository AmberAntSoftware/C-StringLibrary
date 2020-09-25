#ifndef STR_STRING_GLOBALS_H_INCLUDED
#define STR_STRING_GLOBALS_H_INCLUDED

#define STR_SSO_SIZE 32
#define STR_CHAR_BITS 8

//sizeof(constCharPtrWithBOM)/sizeof(char)
#define STR_LITERAL_STRING_LENGTH(stringLiteralConstant) ((sizeof(stringLiteralConstant)/sizeof(stringLiteralConstant[0]))-1)
#define STR_CONST_LITERAL_STRING_SIZE(stringLiteralConstant) (STR_CONST_LITERAL_STRING_LENGTH(stringLiteralConstant)+1)

/*
https://en.wikipedia.org/wiki/Byte_order_mark
*/

/*
//newline characters according to python
\n      Line Feed (LF)
\r      Carriage Return (CR)
\r\n    Carriage Return + Line Feed (CR+LF)
\x0b    Line Tabulation (VT)
\x0c    Form Feed (FF)
\x1c    File Separator (FS)
\x1d    Group Separator (GS)
\x1e    Record Separator (RS)
\x85    Next Line (NEL)
\u2028  Line Separator (LS)
\u2029  Paragraph Separator (PS)
*/

typedef enum STR_bool{
    STR_FALSE = 0,
    STR_TRUE = 1
}STR_bool;

typedef enum STR_ERROR_ENUM{
    STR_ERROR_NO_OP = 0,
    STR_ERROR_SUCCESS,
    STR_ERROR_FAILURE
}STR_ERROR_ENUM;

typedef enum STR_ENCODING_ENUM{
    STR_ENCODING_UNKOWN = 0,
    STR_ENCODING_ANSI,

    STR_ENCODING_UTF1,
    STR_ENCODING_UTF7_0,
    STR_ENCODING_UTF7_1,
    STR_ENCODING_UTF7_2,
    STR_ENCODING_UTF7_3,
    STR_ENCODING_UTF7_4,

    STR_ENCODING_UTF8,
    STR_ENCODING_UTF16_BE,
    STR_ENCODING_UTF16_LE,
    STR_ENCODING_UTF32_BE,
    STR_ENCODING_UTF32_LE,

    STR_ENCODING_SCSU,
    STR_ENCODING_GB18030,
    STR_ENCODING_UTF_EBCDIC,
    STR_ENCODING_BOCU1,

    STR_ENCODING_ENUM_SIZE
}STR_ENCODING_ENUM;

typedef enum STR_ENDIAN_ENUM{
    STR_ENDIAN_BIG,
    STR_ENDIAN_LITTLE,

    STR_ENDIAN_ENUM_SIZE
}STR_ENDIAN_ENUM;

typedef enum STR_CHAR_ERROR_ENUM{
    STR_CHAR_ERROR_NONE = 0,
    STR_CHAR_ERROR_DONE,
    STR_CHAR_ERROR_ENCODE_MISMATCH,
    STR_CHAR_ERROR_ILLEGAL_VALUE,
    STR_CHAR_ERROR_MISSING_DATA,
    STR_CHAR_ERROR_ASCII_8BIT,
    STR_CHAR_ERROR_OVERFLOW,
    STR_CHAR_ERROR_TRAILING_DATA,
    STR_CHAR_ERROR_UNPAIRED_DATA,
    STR_CHAR_ERROR_ILLEGAL_FORMAT,
    STR_CHAR_ERROR_UNSUPPORTED_ENC,

    STR_CHAR_ERROR_ENUM_SIZE
}STR_CHAR_ERROR_ENUM;


typedef size_t STR_codepoint;
typedef struct STR_Char{
    unsigned char byteCount;
    unsigned char bytes[(32/CHAR_BIT)+((32- ((32/CHAR_BIT)*CHAR_BIT))!=0) ];
}STR_Char;


#endif // STR_STRING_GLOBALS_H_INCLUDED
