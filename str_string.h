#ifndef STR_STRING_H_INCLUDED
#define STR_STRING_H_INCLUDED

#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "str_string_globals.h"


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

const char *const STR_ENCODING_ENUM_STRINGS[STR_ENCODING_ENUM_SIZE];
const char *const STR_CHAR_ERROR_ENUM_STRINGS[STR_CHAR_ERROR_ENUM_SIZE];

/*************************************
Independent Operations
*************************************/

STR_ENCODING_ENUM STR_evalEncodingFromBOM(const char *rawString, size_t byteLength);
size_t STR_evalCharCount(const char *rawString, size_t byteLength, STR_ENCODING_ENUM noBomEncoding);

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

STR_String* STR_newString(char* ascii_or_BOM_at_front, size_t byteLength);
STR_ERROR_ENUM STR_initString(STR_String *string, char* ascii_or_BOM_at_front, size_t byteLength);
STR_ERROR_ENUM STR_initStringComplex(STR_String *string, char* rawString, size_t byteLength, STR_ENCODING_ENUM noBomEncoding);

/**************/

STR_Char* STR_newChar(size_t codePoint, STR_ENCODING_ENUM encoding);
STR_ERROR_ENUM STR_initChar(STR_Char *chr, STR_codepoint codePoint, STR_ENCODING_ENUM encoding);
size_t STR_CharSize(STR_codepoint codePoint, STR_ENCODING_ENUM encoding);

/*************************************
Deallocation
*************************************/

void STR_freeString(STR_String *string);
void STR_freeStringData(STR_String *string);
void STR_freeStringNotData(STR_String *string);

/*************************************
Data Accessors
*************************************/

const char* STR_StringReadDataPointer(const STR_String *string);
char* STR_StringGetDataPointer(STR_String *string);

STR_ENCODING_ENUM STR_StringGetEncoding(const STR_String *string);
STR_ENCODING_ENUM STR_StringGetImplicitEncoding(const STR_String *string);
STR_ENCODING_ENUM STR_StringGetExplicitEncoding(const STR_String *string);

size_t STR_StringGetCharLength(const STR_String *string);
size_t STR_StringGetByteLength(const STR_String *string);

STR_bool STR_StringIsSmall(const STR_String *string);
/*************************************
Operations
*************************************/

STR_String* STR_StringConcatNew(STR_String *frontString, STR_String *backString);
void* STR_StringConcatInit(STR_String *owning, STR_String *frontString, STR_String *backString);

void STR_StringAppend(STR_String *mutatedBaseString, STR_String *stringToAdd);

/*************************************
Iteration
*************************************/

STR_ERROR_ENUM STR_initStringIteratorRaw(STR_StringIterator *iter, const char *rawString, size_t byteLength, STR_ENCODING_ENUM encoding);

/**************/

#define STR_StringForEach(constStringPointer, stringIteratorPointer) for( STR_StringIterStartRaw(constStringPointer, stringIteratorPointer); STR_StringIterNextCheck(stringIteratorPointer); STR_StringIterNext( stringIteratorPointer) )

#define STR_StringForEachANSI(constStringPointer, stringIteratorPointer) for( STR_StringIterStartANSI(constStringPointer, stringIteratorPointer); STR_StringIterNextCheckANSI(stringIteratorPointer); STR_StringIterNextANSI( stringIteratorPointer) )
#define STR_StringForEachUTF8(constStringPointer, stringIteratorPointer) for( STR_StringIterStartUTF8(constStringPointer, stringIteratorPointer); STR_StringIterNextCheckUTF8(stringIteratorPointer); STR_StringIterNextUTF8( stringIteratorPointer) )
#define STR_StringForEachUTF16(constStringPointer, stringIteratorPointer) for( STR_StringIterStartUTF16(constStringPointer, stringIteratorPointer); STR_StringIterNextCheckUTF16( stringIteratorPointer) ; STR_StringIterNextUTF16( stringIteratorPointer) )
#define STR_StringForEachUTF32(constStringPointer, stringIteratorPointer) for( STR_StringIterStartUTF32(constStringPointer, stringIteratorPointer); STR_StringIterNextCheckUTF32( stringIteratorPointer) ; STR_StringIterNextUTF32( stringIteratorPointer) )

STR_ERROR_ENUM STR_StringIterStartRaw(const STR_String *string, STR_StringIterator *iter);
STR_bool STR_StringIterNextCheck(STR_StringIterator *iter);
void STR_StringIterNext(STR_StringIterator *iter);


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

size_t STR_X_StringCalcChars(const STR_String *string);

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

/*************************************
Internal Definitions
*************************************/

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


#include "str_string_ansi.h"
#include "str_string_utf8.h"
#include "str_string_utf16.h"
#include "str_string_utf32.h"

#include "str_string_convert.h"

#endif // STR_STRING_H_INCLUDED
