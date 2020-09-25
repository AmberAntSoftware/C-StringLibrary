#include "str_string.h"

/*************************************
Independent Operations
*************************************/

STR_ENCODING_ENUM STR_evalEncodingFromBOM(const char* rawString, size_t byteLength){

    if(byteLength < 1){
        return STR_ENCODING_UNKOWN;
    }

    switch(rawString[0]){
        case('\x00'):
            if(byteLength > 2 && rawString[1] == '\x00' && rawString[2] == '\xFE'
               && rawString[3] == '\xFF'){
                return STR_ENCODING_UTF32_BE;
            }
            return STR_ENCODING_UNKOWN;
        case('\x0E'):
            if(byteLength > 2 && rawString[1]=='\xFE' && rawString[2]=='\xFF'){
                return STR_ENCODING_SCSU;
            }
            return STR_ENCODING_UNKOWN;
        case('\x2B'):
            if(byteLength < 4 || rawString[1] != '\x2F' || rawString[2] != '\x76'){
                return STR_ENCODING_UNKOWN;
            }
            switch(rawString[3]){
                case('\x2B'):
                    return STR_ENCODING_UTF7_2;
                case('\x2F'):
                    return STR_ENCODING_UTF7_3;
                case('\x38'):
                    if(rawString[3]=='\x2D'){
                        return STR_ENCODING_UTF7_4;
                    }
                    return STR_ENCODING_UTF7_0;
                case('\x39'):
                    return STR_ENCODING_UTF7_1;
            }
            return STR_ENCODING_UNKOWN;
        case('\x84'):
            if(byteLength < 4 || rawString[1] != '\x31' || rawString[2] != '\x95' ||
               rawString[3] != '\x33'){
                return STR_ENCODING_UNKOWN;
            }
            return STR_ENCODING_GB18030;
        case('\xDD'):
            if(byteLength < 4 || rawString[1] != '\x73' || rawString[2] != '\x66' ||
               rawString[3] != '\x73'){
                return STR_ENCODING_UNKOWN;
            }
            return STR_ENCODING_UTF_EBCDIC;
        case('\xEF'):
            if(byteLength < 3 || rawString[1] != '\xBB' || rawString[2] != '\xBF'){
                return STR_ENCODING_UNKOWN;
            }
            return STR_ENCODING_UTF8;
        case('\xF7'):
            if(byteLength < 3 || rawString[1] != '\x64' || rawString[2] != '\x4C'){
                return STR_ENCODING_UNKOWN;
            }
            return STR_ENCODING_UTF1;
        case('\xFB'):
            if(byteLength < 3 || rawString[1] != '\xEE' || rawString[2] != '\x28'){
                return STR_ENCODING_UNKOWN;
            }
            return STR_ENCODING_BOCU1;
        case('\xFE'):
            if(byteLength < 2 || rawString[1] != '\xFF'){
                return STR_ENCODING_UNKOWN;
            }
            return STR_ENCODING_UTF16_BE;
        case('\xFF'):
            if(byteLength < 2 || rawString[1] != '\xFE'){
                return STR_ENCODING_UNKOWN;
            }
            if(byteLength > 3 && rawString[2] == '\00' && rawString[3] == '\00'){
                return STR_ENCODING_UTF32_LE;
            }
            return STR_ENCODING_UTF16_LE;
        default:
            return STR_ENCODING_ANSI;
    }
    ///as ascii
    return STR_ENCODING_ANSI;
}

size_t STR_evalCharCount(const char *rawString, size_t byteLength, STR_ENCODING_ENUM encoding){

    STR_StringIterator iter;
    memset(&iter, 0, sizeof(iter));

    STR_ENCODING_ENUM encodingBOM = STR_evalEncodingFromBOM(rawString, byteLength);

    if(encodingBOM == STR_ENCODING_UNKOWN || encodingBOM == STR_ENCODING_ANSI){
        if(encoding != STR_ENCODING_UNKOWN && encoding != STR_ENCODING_ANSI){
            encodingBOM = encoding;
        }
    } else {///TODO FIXME
        switch(encodingBOM){
            case(STR_ENCODING_ANSI):
                break;
            case(STR_ENCODING_UTF8):
                rawString+=STR_BOM_UTF8_SIZE;
                break;
            case(STR_ENCODING_UTF16_BE):
                rawString+=STR_BOM_UTF16_BE_SIZE;
                iter.endianess = STR_ENDIAN_BIG;
                break;
            case(STR_ENCODING_UTF16_LE):
                rawString+=STR_BOM_UTF16_LE_SIZE;
                iter.endianess = STR_ENDIAN_LITTLE;
                break;
            case(STR_ENCODING_UTF32_BE):
                rawString+=STR_BOM_UTF32_BE_SIZE;
                iter.endianess = STR_ENDIAN_BIG;
                break;
            case(STR_ENCODING_UTF32_LE):
                rawString+=STR_BOM_UTF32_LE_SIZE;
                iter.endianess = STR_ENDIAN_LITTLE;
                break;
            default:
                return 0;
        }
    }

    iter.encoding = encodingBOM;
    iter.error = STR_CHAR_ERROR_NONE;
    iter.iterator = 0;
    iter.maxLen = byteLength;
    iter.charPos = 0;
    iter.character = 0;
    iter.str = rawString;

    size_t chars = 0;
    switch(iter.encoding){
        case(STR_ENCODING_ANSI):
            while(STR_StringIterNextCheck(&iter)){
                STR_StringIterNext(&iter);
                chars++;
            }
            break;
        case(STR_ENCODING_UTF8):
            while(STR_StringIterNextCheckUTF8(&iter)){
                STR_StringIterNextUTF8(&iter);
                chars++;
            }
            break;
        case(STR_ENCODING_UTF16_BE):
        case(STR_ENCODING_UTF16_LE):
            while(STR_StringIterNextCheckUTF16(&iter)){
                STR_StringIterNextUTF16(&iter);
                chars++;
            }
            break;
        case(STR_ENCODING_UTF32_BE):
        case(STR_ENCODING_UTF32_LE):
            while(STR_StringIterNextCheckUTF32(&iter)){
                STR_StringIterNextUTF32(&iter);
                chars++;
            }
            break;
        default:
            return 0;
    }

    return chars;
}

/*************************************
Initialization
*************************************/

STR_String *STR_newString(char* ascii_or_BOM_at_front, size_t byteLength){

    STR_ENCODING_ENUM bom = STR_evalEncodingFromBOM(ascii_or_BOM_at_front, byteLength);
    if(bom == STR_ENCODING_UNKOWN){
        return NULL;
    }

    STR_String *str = (STR_String *)calloc(1, sizeof(STR_String));
    if(str == NULL){
        return NULL;
    }

    if(STR_initStringComplex(str, ascii_or_BOM_at_front, byteLength, bom) == STR_ERROR_FAILURE){
        free(str);
        return NULL;
    }

    return str;
}


STR_ERROR_ENUM STR_initString(STR_String *string, char* ascii_or_BOM_at_front, size_t byteLength){
    STR_ENCODING_ENUM bom = STR_evalEncodingFromBOM(ascii_or_BOM_at_front, byteLength);
    if(bom == STR_ENCODING_UNKOWN){
        return STR_ERROR_FAILURE;
    }

    return STR_initStringComplex(string, ascii_or_BOM_at_front, byteLength, bom);
}

STR_ERROR_ENUM STR_initStringComplex(STR_String *string, char* rawString, size_t byteLength, STR_ENCODING_ENUM noBomEncoding){

    string->sso[STR_SSO_CONTROL_ADDR] = 0;

    if(byteLength < STR_SSO_CONTROL_ADDR){
        return STR_X_initStringSmallComplex(string, rawString, byteLength, noBomEncoding);
    }

    string->sso[STR_SSO_CONTROL_ADDR] = ~0;
    string->parts.byteLength = byteLength;
    string->parts.str = rawString;
    string->parts.length = 0;
    string->parts.encoding = noBomEncoding;

    return STR_ERROR_SUCCESS;
}


STR_Char *STR_newChar(size_t codePoint, STR_ENCODING_ENUM encoding){

    STR_Char *chr = malloc(sizeof(chr));
    if(chr == NULL){
        return NULL;
    }
    if(STR_initChar(chr, codePoint, encoding) == STR_ERROR_FAILURE || chr->byteCount == 0){
        free(chr);
        return NULL;
    }
    return chr;
}

STR_ERROR_ENUM STR_initChar(STR_Char *chr, STR_codepoint codePoint, STR_ENCODING_ENUM encoding){

    switch(encoding){
        case(STR_ENCODING_ANSI):
            STR_initCharANSI(chr, codePoint);
        case(STR_ENCODING_UTF8):
            return STR_initCharUTF8(chr, codePoint);
        case(STR_ENCODING_UTF16_BE):
            return STR_initCharUTF16(chr, codePoint, STR_ENDIAN_BIG);
        case(STR_ENCODING_UTF16_LE):
            return STR_initCharUTF16(chr, codePoint, STR_ENDIAN_LITTLE);
        case(STR_ENCODING_UTF32_BE):
            return STR_initCharUTF32(chr, codePoint, STR_ENDIAN_BIG);
        case(STR_ENCODING_UTF32_LE):
            return STR_initCharUTF32(chr, codePoint, STR_ENDIAN_LITTLE);
        default:
            chr->byteCount = 0;
            return STR_ERROR_FAILURE;
    }
}

size_t STR_CharSize(STR_codepoint codePoint, STR_ENCODING_ENUM encoding){

    switch(encoding){
        case(STR_ENCODING_ANSI):
            return STR_CharSizeANSI(codePoint);
        case(STR_ENCODING_UTF8):
            return STR_CharSizeUTF8(codePoint);
        case(STR_ENCODING_UTF16_BE):
            return STR_CharSizeUTF16(codePoint);
        case(STR_ENCODING_UTF16_LE):
            return STR_CharSizeUTF16(codePoint);
        case(STR_ENCODING_UTF32_BE):
            return STR_CharSizeUTF32(codePoint);
        case(STR_ENCODING_UTF32_LE):
            return STR_CharSizeUTF32(codePoint);
        default:
            return 0;
    }
}

/*************************************
Deallocation
*************************************/

void STR_freeString(STR_String *string){
    STR_freeStringData(string);
    free(string);
}
void STR_freeStringData(STR_String *string){
    if(STR_StringIsSmall(string) != STR_TRUE){
        free(string->parts.str);
    }
}
void STR_freeStringNotData(STR_String *string){
    free(string);
}


/*************************************
Data Accessors
*************************************/

const char const* STR_StringReadDataPointer(const STR_String *string){
    if(STR_StringIsSmall(string)){
        return string->sso;
    }else{
        return string->parts.str;
    }
}

char* STR_StringGetDataPointer(STR_String *string){
    if(STR_StringIsSmall(string)){
        return string->sso;
    }else{
        return string->parts.str;
    }
}

STR_ENCODING_ENUM STR_StringGetEncoding(const STR_String *string){

    STR_ENCODING_ENUM encoding = STR_StringGetExplicitEncoding(string);
    STR_ENCODING_ENUM encodingBOM = STR_StringGetImplicitEncoding(string);
    if(encodingBOM == STR_ENCODING_UNKOWN || encodingBOM == STR_ENCODING_ANSI){
        if(encoding != STR_ENCODING_UNKOWN){
            encodingBOM = encoding;
        }
    }else if(encodingBOM != encoding){
        encoding = encodingBOM;
    }

    encoding = encodingBOM;

    return encoding;
}

STR_ENCODING_ENUM STR_StringGetExplicitEncoding(const STR_String *string){
    STR_ENCODING_ENUM encoding;
    if(STR_StringIsSmall(string)){
        encoding = STR_X_StringSmallGetEncoding(string);
    } else {
        encoding = string->parts.encoding;
    }
    return encoding;
}

STR_ENCODING_ENUM STR_StringGetImplicitEncoding(const STR_String *string){
    STR_ENCODING_ENUM encoding;
    const char* const raw = STR_StringGetDataPointer(string);
    encoding = STR_evalEncodingFromBOM(raw, STR_StringGetByteLength(string));
    return encoding;
}

size_t STR_StringGetCharLength(const STR_String *string){
    size_t size;
    if(STR_StringIsSmall(string)){
        return STR_X_StringSmallGetCharLength(string);
    } else {
        size = string->parts.length;
    }
    return size;
}

size_t STR_StringGetByteLength(const STR_String *string){
    if(STR_StringIsSmall(string)){
        return STR_X_StringSmallGetByteLength(string);
    } else {
        return string->parts.byteLength;
    }
}

STR_bool STR_StringIsSmall(const STR_String *string){
    return ((string->sso[STR_SSO_CONTROL_ADDR]&STR_SSO_LONG_BIT) ==0);
}

/*************************************
Iteration
*************************************/

STR_ERROR_ENUM STR_initStringIteratorRaw(STR_StringIterator *iter, const char *rawData, size_t dataByteLength, STR_ENCODING_ENUM explicitEncoding){

    memset(iter, ~((unsigned int)0), sizeof(*iter));

    iter->iterator = 0;

    STR_ENCODING_ENUM encodingBOM = STR_evalEncodingFromBOM(rawData, dataByteLength);
    if(encodingBOM == STR_ENCODING_UNKOWN || encodingBOM == STR_ENCODING_ANSI){
        if(explicitEncoding == STR_ENCODING_UNKOWN){
            encodingBOM = STR_ENCODING_ENUM_SIZE;
            iter->error = STR_CHAR_ERROR_UNSUPPORTED_ENC;
            return STR_ERROR_FAILURE;
        }
        encodingBOM = explicitEncoding;
    }else{
        switch(encodingBOM){
            case(STR_ENCODING_ANSI):
                iter->iterator = STR_BOM_ANSI_SIZE;
                goto ENDIAN_SELECTED;
            case(STR_ENCODING_UTF8):
                iter->iterator = STR_BOM_UTF8_SIZE;
                goto ENDIAN_SELECTED;
            case(STR_ENCODING_UTF16_BE):
                iter->iterator = STR_BOM_UTF16_BE_SIZE;
                iter->endianess = STR_ENDIAN_BIG;
                goto ENDIAN_SELECTED;
            case(STR_ENCODING_UTF16_LE):
                iter->iterator = STR_BOM_UTF16_LE_SIZE;
                iter->endianess = STR_ENDIAN_LITTLE;
                goto ENDIAN_SELECTED;
            case(STR_ENCODING_UTF32_BE):
                iter->iterator = STR_BOM_UTF32_BE_SIZE;
                iter->endianess = STR_ENDIAN_BIG;
                goto ENDIAN_SELECTED;
            case(STR_ENCODING_UTF32_LE):
                iter->iterator = STR_BOM_UTF32_LE_SIZE;
                iter->endianess = STR_ENDIAN_LITTLE;
                goto ENDIAN_SELECTED;
            default:
                iter->error = STR_CHAR_ERROR_UNSUPPORTED_ENC;
                return STR_ERROR_FAILURE;
        }
    }

    switch(encodingBOM){
        case(STR_ENCODING_UTF16_BE):
            iter->endianess = STR_ENDIAN_BIG;
            break;
        case(STR_ENCODING_UTF16_LE):
            iter->endianess = STR_ENDIAN_LITTLE;
            break;
        case(STR_ENCODING_UTF32_BE):
            iter->endianess = STR_ENDIAN_BIG;
            break;
        case(STR_ENCODING_UTF32_LE):
            iter->endianess = STR_ENDIAN_LITTLE;
            break;
        default:
            iter->error = STR_CHAR_ERROR_UNSUPPORTED_ENC;
            return STR_ERROR_FAILURE;
    }
ENDIAN_SELECTED:

    iter->encoding = encodingBOM;

    iter->charPos = 0;
    iter->error = STR_CHAR_ERROR_NONE;
    iter->character = 0;
    iter->str = rawData;
    iter->maxLen = dataByteLength;

    return STR_ERROR_SUCCESS;
}

STR_ERROR_ENUM STR_StringIterStartRaw(const STR_String *string, STR_StringIterator *iter){

    STR_ENCODING_ENUM encoding = STR_StringGetEncoding(string);

    switch(encoding){
        case(STR_ENCODING_ANSI):
            return STR_StringIterStartANSI(string, iter);
        case(STR_ENCODING_UTF8):
            return STR_StringIterStartUTF8(string, iter);
        case(STR_ENCODING_UTF16_BE):
            return STR_StringIterStartUTF16(string, iter);
        case(STR_ENCODING_UTF16_LE):
            return STR_StringIterStartUTF16(string, iter);
        case(STR_ENCODING_UTF32_BE):
            return STR_StringIterStartUTF32(string, iter);
        case(STR_ENCODING_UTF32_LE):
            return STR_StringIterStartUTF32(string, iter);
        default:
            iter->charPos = ~((size_t)0);
            iter->character = ~((size_t)0);
            iter->iterator = ~((size_t)0);
            iter->error = STR_CHAR_ERROR_UNSUPPORTED_ENC;
            return STR_ERROR_FAILURE;
    }

}

STR_bool STR_StringIterNextCheck(STR_StringIterator *iter){

    switch(iter->encoding){
        case(STR_ENCODING_ANSI):
            return STR_StringIterNextCheck(iter);
        case(STR_ENCODING_UTF8):
            return STR_StringIterNextCheckUTF8(iter);
        case(STR_ENCODING_UTF16_BE):
            return STR_StringIterNextCheckUTF16(iter);
        case(STR_ENCODING_UTF16_LE):
            return STR_StringIterNextCheckUTF16(iter);
        case(STR_ENCODING_UTF32_BE):
            return STR_StringIterNextCheckUTF32(iter);
        case(STR_ENCODING_UTF32_LE):
            return STR_StringIterNextCheckUTF32(iter);
        default:
            iter->charPos = ~((size_t)0);
            iter->character = ~((size_t)0);
            iter->iterator = ~((size_t)0);
            iter->error = STR_CHAR_ERROR_UNSUPPORTED_ENC;
            return STR_FALSE;
    }

}

void STR_StringIterNext(STR_StringIterator *iter){

    switch(iter->encoding){
        case(STR_ENCODING_ANSI):
            STR_StringIterNext(iter);
            break;
        case(STR_ENCODING_UTF8):
            STR_StringIterNextUTF8(iter);
            break;
        case(STR_ENCODING_UTF16_BE):
            STR_StringIterNextUTF16(iter);
            break;
        case(STR_ENCODING_UTF16_LE):
            STR_StringIterNextUTF16(iter);
            break;
        case(STR_ENCODING_UTF32_BE):
            STR_StringIterNextUTF32(iter);
            break;
        case(STR_ENCODING_UTF32_LE):
            STR_StringIterNextUTF32(iter);
            break;
        default:
            iter->charPos = ~((size_t)0);
            iter->character = ~((size_t)0);
            iter->iterator = ~((size_t)0);
            iter->error = STR_CHAR_ERROR_UNSUPPORTED_ENC;
            return;
    }

}



/*************************************
Small Strings
*************************************/

STR_ERROR_ENUM STR_X_initStringSmallComplex(STR_String *string, char *data, unsigned char byteLength, STR_ENCODING_ENUM encoding){

    memcpy(string->sso, data, byteLength*sizeof(char));
    string->sso[STR_SSO_CONTROL_ADDR] = 0;

    if(encoding != STR_ENCODING_ANSI){

        unsigned char encodeTranslation;
        switch(encoding){
            case(STR_ENCODING_UTF8):
                encodeTranslation = 0;
            case(STR_ENCODING_UTF16_BE):
                encodeTranslation = 1;
            case(STR_ENCODING_UTF16_LE):
                encodeTranslation = 2;
            case(STR_ENCODING_UTF32_BE):
                encodeTranslation = 3;
            case(STR_ENCODING_UTF32_LE):
                encodeTranslation = 4;
            default:
                return STR_ERROR_FAILURE;
        }
        string->sso[STR_SSO_CONTROL_ADDR] |= STR_SSO_ENCODE_BIT;
        string->sso[STR_SSO_CONTROL_ADDR-1] = (encodeTranslation&STR_SSO_ENCODE_VAL_MASK) << STR_SSO_ENCODE_SHIFT;
        string->sso[STR_SSO_CONTROL_ADDR-1] |= (unsigned char)(STR_evalCharCount(data, byteLength, encoding)&STR_SSO_CHAR_LENGTH_MASK);
    }

    return STR_ERROR_SUCCESS;
}

unsigned char STR_X_StringSmallGetByteLength(const STR_String *string){
    return ((STR_SSO_SIZE-1) - (string->sso[STR_SSO_CONTROL_ADDR] & STR_SSO_BYTE_LENGTH_MASK) );
}

STR_ENCODING_ENUM STR_X_StringSmallGetEncoding(const STR_String *string){
    if((string->sso[STR_SSO_CONTROL_ADDR] & STR_SSO_ENCODE_BIT) !=0){
        unsigned char encoding = string->sso[STR_SSO_CONTROL_ADDR-1]&STR_SSO_ENCODE_VAL_MASK;
        switch(encoding){
            case(0):
                return STR_ENCODING_UTF8;
            case(1):
                return STR_ENCODING_UTF16_BE;
            case(2):
                return STR_ENCODING_UTF16_LE;
            case(3):
                return STR_ENCODING_UTF32_BE;
            case(4):
                return STR_ENCODING_UTF32_LE;
            default:
                return STR_ENCODING_UNKOWN;
        }
    }else{
        return STR_ENCODING_ANSI;
    }
}

unsigned char STR_X_StringSmallGetCharLength(const STR_String *string){
    if(STR_X_StringSmallGetEncoding(string) == STR_ENCODING_ANSI){
        return STR_X_StringSmallGetByteLength(string);
    }
    return (string->sso[STR_SSO_CONTROL_ADDR-1] & STR_SSO_CHAR_LENGTH_MASK);
}


size_t STR_X_StringCalcChars(const STR_String *string){

    return STR_evalCharCount(STR_StringGetDataPointer(string),
                      STR_StringGetByteLength(string),
                      STR_StringGetEncoding(string));
}









const char* STR_EncodingToString(STR_ENCODING_ENUM encoding){
    if(encoding >= STR_ENCODING_ENUM_SIZE){
        return NULL;
    }
    return STR_ENCODING_ENUM_STRINGS[encoding];
}

const char* STR_CharErrorToString(STR_CHAR_ERROR_ENUM encoding){
    if(encoding >= STR_CHAR_ERROR_ENUM_SIZE){
        return NULL;
    }
    return STR_CHAR_ERROR_ENUM_STRINGS[encoding];
}

const char *const STR_BOM_ANSI = "";
const char *const STR_BOM_UTF8 = "\xEF\xBB\xBF";
const char *const STR_BOM_UTF16_BE = "\xFE\xFF";
const char *const STR_BOM_UTF16_LE = "\xFF\xFE";
const char *const STR_BOM_UTF32_BE = "\x00\x00\xFE\xFF";
const char *const STR_BOM_UTF32_LE = "\xFF\xFE\x00\x00";
const char *const STR_BOM_UTF8;
///TODO later support guaranteed
const char *const STR_BOM_UTF_EBCDIC = "\xDD\x73\x66\x73";

///TODO not_support but maybe later
const char *const STR_BOM_UTF1 = "\xF7\x64\x4C";
const char *const STR_BOM_SCSU = "\x0E\xFE\xFF";
const char *const STR_BOM_BOCU1 = "\xFB\xEE\x28";
const char *const STR_BOM_GB18030 = "\x84\x31\x95\x33";

///TODO potentially but not likely ever since superseded
const char *const STR_BOM_UTF7__0 = "\x2B\x2F\x76\x38";
const char *const STR_BOM_UTF7__1 = "\x2B\x2F\x76\x39";
const char *const STR_BOM_UTF7__2 = "\x2B\x2F\x76\x2B";
const char *const STR_BOM_UTF7__3 = "\x2B\x2F\x76\x2F";
const char *const STR_BOM_UTF7__4 = "\x2B\x2F\x76\x38\x2D";

const char *const STR_ILLEGAL_BOM = "UNKNOWN BOM";
const char *const STR_UNSUPPORTED_BOM = "UNSUPPORTED FORMAT";

const char *const STR_ENCODING_ENUM_STRINGS[STR_ENCODING_ENUM_SIZE] = {
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_UNKOWN),
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_ANSI),
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_UTF1),
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_UTF7_0),
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_UTF7_1),
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_UTF7_2),
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_UTF7_3),
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_UTF7_4),
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_UTF8),
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_UTF16_BE),
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_UTF16_LE),
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_UTF32_BE),
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_UTF32_LE),
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_SCSU),
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_GB18030),
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_UTF_EBCDIC),
    STR_X_STRINGIFY_TOKEN(STR_ENCODING_BOCU1)
};

const char *const STR_CHAR_ERROR_ENUM_STRINGS[STR_CHAR_ERROR_ENUM_SIZE] = {
    STR_X_STRINGIFY_TOKEN(STR_CHAR_ERROR_NONE),
    STR_X_STRINGIFY_TOKEN(STR_CHAR_ERROR_DONE),
    STR_X_STRINGIFY_TOKEN(STR_CHAR_ERROR_ENCODE_MISMATCH),
    STR_X_STRINGIFY_TOKEN(STR_CHAR_ERROR_ILLEGAL_VALUE),
    STR_X_STRINGIFY_TOKEN(STR_CHAR_ERROR_MISSING_DATA),
    STR_X_STRINGIFY_TOKEN(STR_CHAR_ERROR_ASCII_8BIT),
    STR_X_STRINGIFY_TOKEN(STR_CHAR_ERROR_OVERFLOW),
    STR_X_STRINGIFY_TOKEN(STR_CHAR_ERROR_TRAILING_DATA),
    STR_X_STRINGIFY_TOKEN(STR_CHAR_ERROR_UNPAIRED_DATA),
    STR_X_STRINGIFY_TOKEN(STR_CHAR_ERROR_ILLEGAL_FORMAT),
    STR_X_STRINGIFY_TOKEN(STR_CHAR_ERROR_UNSUPPORTED_ENC),
};
