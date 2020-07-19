#ifndef STR_STRING_H_INCLUDED
#define STR_STRING_H_INCLUDED

#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define STR_SSO_SIZE 32
#define STR_CHAR_BITS 8

#define STR_CONST_LITERAL_STRING_LENGTH(stringLiteralConstant) ((sizeof(stringLiteralConstant)/sizeof(stringLiteralConstant[0]))-1)
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

typedef size_t STR_codepoint;
typedef struct STR_Char{
    unsigned char byteCount;
    unsigned char bytes[32/CHAR_BIT];
}STR_Char;

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

typedef struct STR_StringParts{
    char *str;
    size_t length;
    size_t byteLength;
    STR_ENCODING_ENUM encoding;

    char initSSO[
        (sizeof(char[STR_SSO_SIZE])- sizeof(char*)-
         (sizeof(size_t)*2) - sizeof(STR_ENCODING_ENUM)) -1
    ];
    char SSO_FLAG;
}STR_StringParts;

typedef union STR_String{
    STR_StringParts parts;
    char sso[STR_SSO_SIZE];
}STR_String;

const char *const STR_ENCODING_ENUM_STRINGS[STR_ENCODING_ENUM_SIZE];
const char *const STR_CHAR_ERROR_ENUM_STRINGS[STR_CHAR_ERROR_ENUM_SIZE];


typedef struct STR_StringIterator{
    STR_codepoint character;
    size_t charPos;

    size_t iterator;
    STR_CHAR_ERROR_ENUM error;

    STR_ENDIAN_ENUM endianess;
    size_t maxLen;
    const char* str;
    STR_ENCODING_ENUM encoding;
}STR_StringIterator;


/*************************************
Independent Operations
*************************************/

STR_ENCODING_ENUM STR_getEncodingFromBOM(const char *rawString, size_t rawLength);
size_t STR_evalCharCount(const char *rawString, size_t byteLength, STR_ENCODING_ENUM encoding);

const char* STR_EncodingToString(STR_ENCODING_ENUM encoding);
const char* STR_CharErrorToString(STR_CHAR_ERROR_ENUM encoding);

/*************************************
Initialization
*************************************/

#define STR_initStringGlobalLiteralBOM(constCharPtrWithBOM) STR_X_initStringStructForm(constCharPtrWithBOM, (sizeof(constCharPtrWithBOM)/sizeof(char))-1, STR_ENCODING_UNKOWN)
#define STR_initStringGlobalLiteral(constCharPtrWithBOM, STR_ENCODING_ENUM_VALUE) STR_X_initStringStructForm(constCharPtrWithBOM, (sizeof(constCharPtrWithBOM)/sizeof(char))-1, STR_ENCODING_ENUM_VALUE)
#define STR_initStringGlobalComplex(rawStringPtr, byteLength, STR_ENCODING_ENUM_VALUE) STR_X_initStringStructForm(rawStringPtr, byteLength, STR_ENCODING_ENUM_VALUE)

#define STR_initStringLiteralBOM(constCharPtrWithBOM) STR_X_initStringStructForm(constCharPtrWithBOM, (sizeof(constCharPtrWithBOM)/sizeof(char))-1, STR_ENCODING_UNKOWN)
#define STR_initStringLiteral(constCharPtrWithBOM, STR_ENCODING_ENUM_VALUE) STR_X_initStringStructForm(constCharPtrWithBOM, (sizeof(constCharPtrWithBOM)/sizeof(char))-1, STR_ENCODING_ENUM_VALUE)

STR_String* STR_newString(const char* ascii_or_BOM_at_front);
void* STR_initString(STR_String *string, const char* ascii_or_BOM_at_front);
void* STR_initStringFrom(STR_String *string, const char* ascii_or_BOM_at_front);

void STR_initStringComplex(STR_String *string, char* rawData, size_t byteLength, STR_ENCODING_ENUM encoding);

/**************/

STR_Char* STR_newChar(size_t codePoint, STR_ENCODING_ENUM encoding);
STR_ERROR_ENUM STR_initChar(STR_Char *chr, size_t codePoint, STR_ENCODING_ENUM encoding);

STR_Char* STR_newCharANSI(size_t codePoint);
STR_Char* STR_newCharUTF8(size_t codePoint);
STR_Char* STR_newCharUTF16(size_t codePoint, STR_ENDIAN_ENUM endianess);
STR_Char* STR_newCharUTF32(size_t codePoint, STR_ENDIAN_ENUM endianess);

STR_ERROR_ENUM STR_initCharANSI(STR_Char *chr, size_t codePoint);
STR_ERROR_ENUM STR_initCharUTF8(STR_Char *chr, size_t codePoint);
STR_ERROR_ENUM STR_initCharUTF16(STR_Char *chr, size_t codePoint, STR_ENDIAN_ENUM endianess);
STR_ERROR_ENUM STR_initCharUTF32(STR_Char *chr, size_t codePoint, STR_ENDIAN_ENUM endianess);

/**************/

void STR_initCharIter(const STR_String *string, STR_StringIterator *iter);

/*************************************
Deallocation
*************************************/

void STR_freeString(STR_String *string);
void STR_freeStringData(STR_String *string);

/*************************************
Data Accessors
*************************************/

const char* STR_StringGetDataPointer(const STR_String *string);

STR_ENCODING_ENUM STR_StringGetEncoding(const STR_String *string);
STR_ENCODING_ENUM STR_StringGetImplicitEncoding(const STR_String *string);
STR_ENCODING_ENUM STR_StringGetExplicitEncoding(const STR_String *string);

size_t STR_StringGetLength(const STR_String *string);
size_t STR_StringGetByteLength(const STR_String *string);

STR_bool STR_StringIsSmall(const STR_String *string);

/*************************************
Operations
*************************************/

STR_String* STR_StringConcatNew(STR_String *frontString, STR_String *backString);
void* STR_StringConcatInit(STR_String *owning, STR_String *frontString, STR_String *backString);

void STR_StringAppend(STR_String *mutatedBaseString, STR_String *stringToAdd);




size_t STR_X_StringCalcChars(const STR_String *string);



/*************************************
Iteration
*************************************/

STR_ERROR_ENUM STR_initStringIterator(STR_StringIterator *iter, const char *rawData, size_t dataByteLength, STR_ENCODING_ENUM explicitEncoding);

#define STR_StringforEach(constStringPointer, charIteratorPointer) for( STR_initCharIter(constStringPointer, charIteratorPointer); STR_X_checkRunover(charIteratorPointer); STR_X_nextChar( charIteratorPointer) )

#define STR_StringforEach_ANSI(constStringPointer, charIteratorPointer) for( STR_X_ANSI_initChar(constStringPointer, charIteratorPointer); STR_X_ANSI_checkRunover(charIteratorPointer); STR_X_ANSI_nextChar( charIteratorPointer) )
#define STR_StringforEach_UTF8(constStringPointer, charIteratorPointer) for( STR_X_UTF8_initChar(constStringPointer, charIteratorPointer); STR_X_UTF8_checkRunover(charIteratorPointer); STR_X_UTF8_nextChar( charIteratorPointer) )
#define STR_StringforEach_UTF16(constStringPointer, charIteratorPointer) for( STR_X_UTF16_initChar(constStringPointer, charIteratorPointer); STR_X_UTF16_checkRunover( charIteratorPointer) ; STR_X_UTF16_nextChar( charIteratorPointer) )
#define STR_StringforEach_UTF32(constStringPointer, charIteratorPointer) for( STR_X_UTF32_initChar(constStringPointer, charIteratorPointer); STR_X_UTF32_checkRunover( charIteratorPointer) ; STR_X_UTF32_nextChar( charIteratorPointer) )

STR_bool STR_X_checkRunover(STR_StringIterator *iter);
void STR_X_nextChar(STR_StringIterator *iter);

void STR_X_ANSI_initChar(const STR_String *string, STR_StringIterator *iter);
STR_bool STR_X_ANSI_checkRunover(STR_StringIterator *iter);
void STR_X_ANSI_nextChar(STR_StringIterator *iter);

void STR_X_UTF8_initChar(const STR_String *string, STR_StringIterator *iter);
STR_bool STR_X_UTF8_checkRunover(STR_StringIterator *iter);
void STR_X_UTF8_nextChar(STR_StringIterator *iter);

void STR_X_UTF16_initChar(const STR_String *string, STR_StringIterator *iter);
STR_bool STR_X_UTF16_checkRunover(STR_StringIterator *iter);
void STR_X_UTF16_nextChar(STR_StringIterator *iter);

void STR_X_UTF32_initChar(const STR_String *string, STR_StringIterator *iter);
STR_bool STR_X_UTF32_checkRunover(STR_StringIterator *iter);
void STR_X_UTF32_nextChar(STR_StringIterator *iter);



/*************************************
Small Strings
*************************************/

#define STR_SSO_CHAR_LENGTH_MASK 0x1F
#define STR_SSO_BYTE_LENGTH_MASK 0x1F
#define STR_SSO_LONG_BIT 0x20
#define STR_SSO_ENCODE_BIT 0x40
#define STR_SSO_ENCODE_VAL_MASK 0xE0
#define STR_SSO_ENCODE_SHIFT 5

#define STR_SSO_CONTROL_ADDR (STR_SSO_SIZE)-1

#define STR_SSO_HIGH_BIT_MASK ((unsigned char)(((unsigned char)(~0))-(STR_SSO_SIZE -1)))
#define STR_SSO_LOW_BIT_MASK ((unsigned char)(~(unsigned char)STR_SSO_HIGH_BIT_MASK))

#define STR_IS_SMALL_STRING_PTR(str) STR_IS_SMALL_STRING_CHAR((str)->sso[STR_SSO_SIZE-1])
#define STR_IS_SMALL_STRING_CHAR(charCheck) ( ((charCheck) & STR_SSO_HIGH_BIT_MASK) == 0 )

STR_ERROR_ENUM STR_X_initStringSmallComplex(STR_String *string, char *data, unsigned char byteLength, STR_ENCODING_ENUM encoding);
unsigned char STR_X_StringSmallGetByteLength(const STR_String *string);
STR_ENCODING_ENUM STR_X_StringSmallGetEncoding(const STR_String *string);
unsigned char STR_X_StringSmallGetCharLength(const STR_String *string);



/*************************************
Internal Utilities https://www.hyrumslaw.com/
*************************************/

unsigned char STR_X_UTF8_CalcMetaSize(unsigned char charByte);

#define STR_X_initStringStructForm(stringLiteralConstant, byteLength, STR_ENCODING_ENUM_VALUE) {\
{\
stringLiteralConstant, \
0,\
byteLength, \
STR_ENCODING_ENUM_VALUE, \
{~0}, ~0 \
}\
}

//https://stackoverflow.com/questions/2100331/c-macro-definition-to-determine-big-endian-or-little-endian-machine
#ifndef STR_BIG_ENDIAN

#define STR_X_LITTLE_ENDIAN 0x41424344UL
#define STR_X_BIG_ENDIAN    0x44434241UL
#define STR_X_ENDIAN_ORDER  ('ABCD')

#define STR_X_STRINGIFY_TOKEN(token) #token

#if STR_X_ENDIAN_ORDER == STR_X_LITTLE_ENDIAN
    #undef STR_X_ENDIAN_ORDER
    #undef STR_X_LITTLE_ENDIAN
    #undef STR_X_BIG_ENDIAN
    #define STR_BIG_ENDIAN 0
    #define STR_LITTLE_ENDIAN 1
#elif STR_X_ENDIAN_ORDER == STR_X_BIG_ENDIAN
    #undef STR_X_ENDIAN_ORDER
    #undef STR_X_LITTLE_ENDIAN
    #undef STR_X_BIG_ENDIAN
    #define STR_BIG_ENDIAN 1
    #define STR_LITTLE_ENDIAN 0
#endif


#endif

const char *const STR_BOM_ANSI;
#define STR_BOM_ANSI_SIZE 0
const char *const STR_BOM_UTF8;
#define STR_BOM_UTF8_SIZE 3
const char *const STR_BOM_UTF16_BE;
#define STR_BOM_UTF16_BE_SIZE 2
const char *const STR_BOM_UTF16_LE;
#define STR_BOM_UTF16_LE_SIZE 2
const char *const STR_BOM_UTF32_BE;
#define STR_BOM_UTF32_BE_SIZE 4
const char *const STR_BOM_UTF32_LE;
#define STR_BOM_UTF32_LE_SIZE 4

///TODO later support guaranteed
const char *const STR_BOM_UTF_EBCDIC;
#define STR_BOM_UTF_EBCDIC_SIZE 4
//regular ebcdic

///TODO not_support but maybe later
const char *const STR_BOM_UTF1;
#define STR_BOM_UTF1_SIZE 3
const char *const STR_BOM_SCSU;
#define STR_BOM_SCSU_SIZE 3
const char *const STR_BOM_BOCU1;
#define STR_BOM_BOCU1_SIZE 3
const char *const STR_BOM_GB18030;
#define STR_BOM_GB18030_SIZE 4

///TODO potentially but not likely ever since superseded
const char *const STR_BOM_UTF7__0;
#define STR_BOM_UTF7__0_SIZE 4
const char *const STR_BOM_UTF7__1;
#define STR_BOM_UTF7__1_SIZE 4
const char *const STR_BOM_UTF7__2;
#define STR_BOM_UTF7__2_SIZE 4
const char *const STR_BOM_UTF7__3;
#define STR_BOM_UTF7__3_SIZE 4
const char *const STR_BOM_UTF7__4;
#define STR_BOM_UTF7__4_SIZE 4

const char *const STR_ILLEGAL_BOM;
const char *const STR_UNSUPPORTED_BOM;

#endif // STR_STRING_H_INCLUDED
