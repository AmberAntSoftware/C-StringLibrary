#include "str_string.h"


///TODO replace with functions per initial byte lookup
STR_ENCODING_ENUM STR_getEncodingFromBOM(const char* rawString, size_t rawLength){

    if(rawLength < 1){
        return STR_ENCODING_UNKOWN;
    }

    switch(rawString[0]){
        case('\x00'):
            if(rawLength > 2 && rawString[1] == '\x00' && rawString[2] == '\xFE'
               && rawString[3] == '\xFF'){
                return STR_ENCODING_UTF32_BE;
            }
            return STR_ENCODING_UNKOWN;
        case('\x0E'):
            if(rawLength > 2 && rawString[1]=='\xFE' && rawString[2]=='\xFF'){
                return STR_ENCODING_SCSU;
            }
            return STR_ENCODING_UNKOWN;
        case('\x2B'):
            if(rawLength < 4 || rawString[1] != '\x2F' || rawString[2] != '\x76'){
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
            if(rawLength < 4 || rawString[1] != '\x31' || rawString[2] != '\x95' ||
               rawString[3] != '\x33'){
                return STR_ENCODING_UNKOWN;
            }
            return STR_ENCODING_GB18030;
        case('\xDD'):
            if(rawLength < 4 || rawString[1] != '\x73' || rawString[2] != '\x66' ||
               rawString[3] != '\x73'){
                return STR_ENCODING_UNKOWN;
            }
            return STR_ENCODING_UTF_EBCDIC;
        case('\xEF'):
            if(rawLength < 3 || rawString[1] != '\xBB' || rawString[2] != '\xBF'){
                return STR_ENCODING_UNKOWN;
            }
            return STR_ENCODING_UTF8;
        case('\xF7'):
            if(rawLength < 3 || rawString[1] != '\x64' || rawString[2] != '\x4C'){
                return STR_ENCODING_UNKOWN;
            }
            return STR_ENCODING_UTF1;
        case('\xFB'):
            if(rawLength < 3 || rawString[1] != '\xEE' || rawString[2] != '\x28'){
                return STR_ENCODING_UNKOWN;
            }
            return STR_ENCODING_BOCU1;
        case('\xFE'):
            if(rawLength < 2 || rawString[1] != '\xFF'){
                return STR_ENCODING_UNKOWN;
            }
            return STR_ENCODING_UTF16_BE;
        case('\xFF'):
            if(rawLength < 2 || rawString[1] != '\xFE'){
                return STR_ENCODING_UNKOWN;
            }
            if(rawLength > 3 && rawString[2] == '\00' && rawString[3] == '\00'){
                return STR_ENCODING_UTF32_LE;
            }
            return STR_ENCODING_UTF16_LE;
        default:
            return STR_ENCODING_ANSI;
    }
    ///as ascii
    return STR_ENCODING_ANSI;
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







void STR_initStringComplex(STR_String *string, char* rawData, size_t byteLength, STR_ENCODING_ENUM encoding){

    string->sso[STR_SSO_CONTROL_ADDR] = 0;

    if(encoding == STR_ENCODING_ANSI && byteLength < STR_SSO_SIZE ){
        string->sso[STR_SSO_CONTROL_ADDR] = 0;
        string->sso[STR_SSO_CONTROL_ADDR] ^= byteLength & STR_SSO_LOW_BIT_MASK;
        return;
    }

    if(byteLength < STR_SSO_CONTROL_ADDR ){
        ///TODO FIXME add in non ANSI SSO
        puts("SSO for non ANSI is not made yet");
        exit(-7);
        return;
    }

    ///NON SSO string
    string->sso[STR_SSO_CONTROL_ADDR] = ~0;

    string->parts.byteLength = byteLength;
    string->parts.str = rawData;
    string->parts.length = 0;
    string->parts.encoding = encoding;


}







const char const* STR_StringGetDataPointer(const STR_String *string){
    if(STR_StringIsSmall(string)){
        return string->sso;
    }else{
        return string->parts.str;
    }
}

size_t STR_StringGetLength(const STR_String *string){
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
    encoding = STR_getEncodingFromBOM(raw, STR_StringGetByteLength(string));
    return encoding;
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

STR_ERROR_ENUM STR_initChar(STR_Char *chr, size_t codePoint, STR_ENCODING_ENUM encoding){

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

STR_Char *STR_newCharANSI(size_t codePoint){
    return STR_newChar(codePoint, STR_ENCODING_ANSI);
}

STR_Char *STR_newCharUTF8(size_t codePoint){
    return STR_newChar(codePoint, STR_ENCODING_UTF8);
}

STR_Char *STR_newCharUTF16(size_t codePoint, STR_ENDIAN_ENUM endianess){

    if(endianess == STR_ENDIAN_BIG){
        return STR_newChar(codePoint, STR_ENCODING_UTF16_BE);

    }else if(endianess == STR_ENDIAN_LITTLE){
        return STR_newChar(codePoint, STR_ENCODING_UTF16_LE);

    }else{
        return NULL;
    }
}

STR_Char *STR_newCharUTF32(size_t codePoint, STR_ENDIAN_ENUM endianess){

    if(endianess == STR_ENDIAN_BIG){
        return STR_newChar(codePoint, STR_ENCODING_UTF32_BE);

    }else if(endianess == STR_ENDIAN_LITTLE){
        return STR_newChar(codePoint, STR_ENCODING_UTF32_LE);

    }else{
        return NULL;
    }
}

STR_ERROR_ENUM STR_initCharANSI(STR_Char *chr, size_t codePoint){

    if(codePoint > 127){
        chr->byteCount = 0;
        return STR_ERROR_FAILURE;
    }
    chr->byteCount = 1;
    chr->bytes[0] = (unsigned char)codePoint;
    return STR_ERROR_SUCCESS;
}

STR_ERROR_ENUM STR_initCharUTF8(STR_Char *chr, size_t codePoint){
    const unsigned char MAX_BITS = 21;
    const unsigned char LEN_4_THRESHOLD = 16;
    const unsigned char LEN_3_THRESHOLD = 11;
    const unsigned char LEN_2_THRESHOLD = 7;

    const unsigned char LEN_4_HEAD_MASK = 0x7;
    const unsigned char LEN_3_HEAD_MASK = 0xF;
    const unsigned char LEN_2_HEAD_MASK = 0x1F;

    const unsigned char LEN_4_HEAD_KEY = 0xF0;
    const unsigned char LEN_3_HEAD_KEY = 0xE0;
    const unsigned char LEN_2_HEAD_KEY = 0xC0;

    const unsigned char PARTS_KEY = 0x80;
    const unsigned char PARTS_MASK = 0x3F;
    const unsigned char PARTS_BITS = 6;

    if(codePoint > (((size_t)1)<<MAX_BITS)-1){
        chr->byteCount = 0;
        return STR_ERROR_FAILURE;
    }

    if(codePoint > (((size_t)1)<<LEN_4_THRESHOLD)-1 ){
        chr->byteCount = 4;
        chr->bytes[0] = LEN_4_HEAD_KEY | ((codePoint>>(PARTS_BITS*3)) & LEN_4_HEAD_MASK);
        chr->bytes[1] = PARTS_KEY | ((codePoint>>(PARTS_BITS*2)) & PARTS_MASK);
        chr->bytes[2] = PARTS_KEY | ((codePoint>>(PARTS_BITS*1)) & PARTS_MASK);
        chr->bytes[3] = PARTS_KEY | (codePoint & PARTS_MASK);
        return STR_ERROR_SUCCESS;
    }

    if(codePoint > (((size_t)1)<<LEN_3_THRESHOLD)-1 ){
        chr->byteCount = 3;
        chr->bytes[0] = LEN_3_HEAD_KEY | ((codePoint>>(PARTS_BITS*2)) & LEN_3_HEAD_MASK);
        chr->bytes[1] = PARTS_KEY | ((codePoint>>(PARTS_BITS*1)) & PARTS_MASK);
        chr->bytes[2] = PARTS_KEY | (codePoint & PARTS_MASK);
        return STR_ERROR_SUCCESS;
    }

    if(codePoint > (((size_t)1)<<LEN_2_THRESHOLD)-1 ){
        chr->byteCount = 2;
        chr->bytes[0] = LEN_2_HEAD_KEY | ((codePoint>>(PARTS_BITS)) & LEN_2_HEAD_MASK);
        chr->bytes[1] = PARTS_KEY | (codePoint & PARTS_MASK);
        return STR_ERROR_SUCCESS;
    }

    chr->byteCount = 1;
    chr->bytes[0] = (unsigned char)(codePoint&0xFF);

    return STR_ERROR_SUCCESS;
}

STR_ERROR_ENUM STR_initCharUTF16(STR_Char *chr, size_t codePoint, STR_ENDIAN_ENUM endianess){
    const size_t MAX_LIMIT = (((size_t)1)<<20)-1;
    const size_t ENCODE_LIMIT = 0xFFFF;
    const unsigned char HIGH_BYTE = 0xD8;
    const unsigned char LOW_BYTE = 0xDC;
    //const unsigned char SURROGATE_MASK = 0xFC;
    //const unsigned char SURROGATE_MASK_BITS = 6;
    const unsigned char SURROGATE_DATA_MASK = 3;
    const unsigned char SURROGATE_DATA_MASK_BITS = 2;
    //const size_t FULL_SURROGATE_DATA_MASK = 0x103FF;
    const unsigned char FULL_SURROGATE_DATA_MASK_BITS = 10;

    if(codePoint > MAX_LIMIT){
        chr->byteCount = 0;
        return STR_ERROR_FAILURE;
    }

    endianess = (endianess != 0);

    if(codePoint > ENCODE_LIMIT){
        chr->byteCount = 4;
        codePoint-=0x10000;

        chr->bytes[endianess] = HIGH_BYTE ^ (unsigned char)((codePoint>>(FULL_SURROGATE_DATA_MASK_BITS*2-SURROGATE_DATA_MASK_BITS)) & SURROGATE_DATA_MASK);
        chr->bytes[endianess^1] = ((codePoint>>((FULL_SURROGATE_DATA_MASK_BITS*2-SURROGATE_DATA_MASK_BITS)-STR_CHAR_BITS)) & ((((unsigned int)1)<<STR_CHAR_BITS)-1))&0xFF;
        chr->bytes[endianess+2] = LOW_BYTE ^ ((codePoint>>(FULL_SURROGATE_DATA_MASK_BITS-SURROGATE_DATA_MASK_BITS)) & SURROGATE_DATA_MASK);
        chr->bytes[(endianess^1)+2] = codePoint & ((((unsigned int)1)<<STR_CHAR_BITS)-1);

        return STR_ERROR_SUCCESS;
    }

    chr->byteCount = 2;
    chr->bytes[endianess^1] = (codePoint >> 8)&0xFF;
    chr->bytes[endianess] = codePoint&0xFF;

    return STR_ERROR_SUCCESS;
}

STR_ERROR_ENUM STR_initCharUTF32(STR_Char *chr, size_t codePoint, STR_ENDIAN_ENUM endianess){

    const size_t MAX_LIMIT = (((size_t)1)<<31)+((((size_t)1)<<31)-1);

    if(codePoint > MAX_LIMIT){
        chr->byteCount = 0;
        return STR_ERROR_FAILURE;
    }

    endianess = (endianess != 0);

    chr->byteCount = 4;

    const unsigned char posM = (endianess << 1)|(endianess);
    chr->bytes[0] = (codePoint>>(posM*STR_CHAR_BITS))&0xFF;
    chr->bytes[1] = (codePoint>>((1 + endianess)*STR_CHAR_BITS))&0xFF;
    chr->bytes[2] = (codePoint>>((2 - endianess)*STR_CHAR_BITS))&0xFF;
    chr->bytes[3] = (codePoint>>((posM^3)*STR_CHAR_BITS))&0xFF;

    return STR_ERROR_SUCCESS;

}



/****************************************************************/

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

STR_bool STR_StringIsSmall(const STR_String *string){
    return ((string->sso[STR_SSO_CONTROL_ADDR]&STR_SSO_LONG_BIT) ==0);
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

/****************************************************************/























size_t STR_X_StringCalcChars(const STR_String *string){
    size_t chars = 0;
    STR_StringIterator iter;
    STR_StringforEach(string, &iter){
        chars++;
    }
    return chars;
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
//regular ebcdic

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



unsigned char STR_X_UTF8_CalcMetaSize(unsigned char charByte){

    unsigned char byteCount = 0;
    unsigned char val = 1;

    val = ((charByte&0x80)>>7)&val;
    byteCount+=val;
    val = ((charByte&0x40)>>6)&val;
    byteCount+=val;
    val = ((charByte&0x20)>>5)&val;
    byteCount+=val;
    val = ((charByte&0x10)>>4)&val;
    byteCount+=val;

    val = ((charByte&0x08)>>3)&val;
    byteCount+=val;
    val = ((charByte&0x04)>>2)&val;
    byteCount+=val;
    val = ((charByte&0x02)>>1)&val;
    byteCount+=val;
    val = ((charByte&0x01)>>0)&val;
    byteCount+=val;

    return byteCount;

}

size_t STR_evalCharCount(const char *rawString, size_t byteLength, STR_ENCODING_ENUM encoding){

    STR_StringIterator iter;
    memset(&iter, 0, sizeof(iter));

    STR_ENCODING_ENUM encodingBOM = STR_getEncodingFromBOM(rawString, byteLength);

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
            while(STR_X_ANSI_checkRunover(&iter)){
                STR_X_ANSI_nextChar(&iter);
                chars++;
            }
            break;
        case(STR_ENCODING_UTF8):
            while(STR_X_UTF8_checkRunover(&iter)){
                STR_X_UTF8_nextChar(&iter);
                chars++;
            }
            break;
        case(STR_ENCODING_UTF16_BE):
        case(STR_ENCODING_UTF16_LE):
            while(STR_X_UTF16_checkRunover(&iter)){
                STR_X_UTF16_nextChar(&iter);
                chars++;
            }
            break;
        case(STR_ENCODING_UTF32_BE):
        case(STR_ENCODING_UTF32_LE):
            while(STR_X_UTF32_checkRunover(&iter)){
                STR_X_UTF32_nextChar(&iter);
                chars++;
            }
            break;
        default:
            return 0;
    }

    return chars;
}









STR_ERROR_ENUM STR_initStringIterator(STR_StringIterator *iter, const char *rawData, size_t dataByteLength, STR_ENCODING_ENUM explicitEncoding){

    memset(iter, ~((unsigned int)0), sizeof(*iter));

    iter->iterator = 0;

    STR_ENCODING_ENUM encodingBOM = STR_getEncodingFromBOM(rawData, dataByteLength);
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
















void STR_initCharIter(const STR_String *string, STR_StringIterator *iter){

    STR_ENCODING_ENUM encoding = STR_StringGetEncoding(string);

    switch(encoding){
        case(STR_ENCODING_ANSI):
            STR_X_ANSI_initChar(string, iter);
            return;
        case(STR_ENCODING_UTF8):
            STR_X_UTF8_initChar(string, iter);
            return;
        case(STR_ENCODING_UTF16_BE):
        case(STR_ENCODING_UTF16_LE):
            STR_X_UTF16_initChar(string, iter);
            return;
        case(STR_ENCODING_UTF32_BE):
        case(STR_ENCODING_UTF32_LE):
            STR_X_UTF32_initChar(string, iter);
            return;
        default:
            iter->charPos = ~((size_t)0);
            iter->character = ~((size_t)0);
            iter->iterator = ~((size_t)0);
            iter->error = STR_CHAR_ERROR_UNSUPPORTED_ENC;
            return;
    }

}

STR_bool STR_X_checkRunover(STR_StringIterator *iter){

    switch(iter->encoding){
        case(STR_ENCODING_ANSI):
            return STR_X_ANSI_checkRunover(iter);
        case(STR_ENCODING_UTF8):
            return STR_X_UTF8_checkRunover(iter);
        case(STR_ENCODING_UTF16_BE):
            return STR_X_UTF16_checkRunover(iter);
        case(STR_ENCODING_UTF16_LE):
            return STR_X_UTF16_checkRunover(iter);
        case(STR_ENCODING_UTF32_BE):
            return STR_X_UTF32_checkRunover(iter);
        case(STR_ENCODING_UTF32_LE):
            return STR_X_UTF32_checkRunover(iter);
        default:
            iter->charPos = ~((size_t)0);
            iter->character = ~((size_t)0);
            iter->iterator = ~((size_t)0);
            iter->error = STR_CHAR_ERROR_UNSUPPORTED_ENC;
            return STR_FALSE;
    }

}

void STR_X_nextChar(STR_StringIterator *iter){

    switch(iter->encoding){
        case(STR_ENCODING_ANSI):
            STR_X_ANSI_nextChar(iter);
            break;
        case(STR_ENCODING_UTF8):
            STR_X_UTF8_nextChar(iter);
            break;
        case(STR_ENCODING_UTF16_BE):
            STR_X_UTF16_nextChar(iter);
            break;
        case(STR_ENCODING_UTF16_LE):
            STR_X_UTF16_nextChar(iter);
            break;
        case(STR_ENCODING_UTF32_BE):
            STR_X_UTF32_nextChar(iter);
            break;
        case(STR_ENCODING_UTF32_LE):
            STR_X_UTF32_nextChar(iter);
            break;
        default:
            iter->charPos = ~((size_t)0);
            iter->character = ~((size_t)0);
            iter->iterator = ~((size_t)0);
            iter->error = STR_CHAR_ERROR_UNSUPPORTED_ENC;
            return;
    }

}

void STR_X_ANSI_initChar(const STR_String *string, STR_StringIterator *iter){

    ///TODO FIXME ALL ENCODINGS
    ///check and offset if BOM exists
    ///if BOM does not match specified, error and cancel

    iter->iterator = 0;

    STR_ENCODING_ENUM encoding = STR_StringGetExplicitEncoding(string);
    STR_ENCODING_ENUM encodingBOM = STR_StringGetImplicitEncoding(string);

    if(encodingBOM == STR_ENCODING_UNKOWN){
        if(encoding == STR_ENCODING_ANSI){
            encodingBOM = encoding;
        }
    }

    iter->encoding = encodingBOM;

    if(encodingBOM != STR_ENCODING_ANSI){
        iter->charPos = ~((size_t)0);
        iter->character = ~((size_t)0);
        iter->iterator = ~((size_t)0);
        iter->error = STR_CHAR_ERROR_ENCODE_MISMATCH;
        return;
    }

    iter->charPos = 0;
    iter->error = STR_CHAR_ERROR_NONE;
    iter->character = 0;

    iter->str = STR_StringGetDataPointer(string);
    iter->maxLen = STR_StringGetByteLength(string);

    STR_X_ANSI_nextChar(iter);
}

STR_bool STR_X_ANSI_checkRunover(STR_StringIterator *iter){
    if(iter->iterator >= iter->maxLen){
        if(iter->error != STR_CHAR_ERROR_ENCODE_MISMATCH){
            iter->error = STR_CHAR_ERROR_DONE;
        }
        return STR_FALSE;
    }
    return STR_TRUE;
}

void STR_X_ANSI_nextChar(STR_StringIterator *iter){
    iter->character = (unsigned char)iter->str[iter->iterator];
    iter->charPos++;
    iter->iterator++;
}

void STR_X_UTF8_initChar(const STR_String *string, STR_StringIterator *iter){

    iter->iterator = 0;

    STR_ENCODING_ENUM encoding = STR_StringGetExplicitEncoding(string);
    STR_ENCODING_ENUM encodingBOM = STR_StringGetImplicitEncoding(string);

    if(encodingBOM == STR_ENCODING_UNKOWN || encodingBOM == STR_ENCODING_ANSI){
        if(encoding == STR_ENCODING_UTF8){
            encodingBOM = encoding;
        }
    }else if(encodingBOM == STR_ENCODING_UTF8){
        iter->iterator = STR_BOM_UTF8_SIZE;
    }

    iter->encoding = encodingBOM;

    if(encodingBOM != STR_ENCODING_UTF8){
        iter->charPos = ~((size_t)0);
        iter->character = ~((size_t)0);
        iter->iterator = ~((size_t)0);
        iter->error = STR_CHAR_ERROR_ENCODE_MISMATCH;
        return;
    }

    iter->charPos = 0;
    iter->error = STR_CHAR_ERROR_NONE;
    iter->character = 0;

    iter->str = STR_StringGetDataPointer(string);
    iter->maxLen = STR_StringGetByteLength(string);

    STR_X_UTF8_nextChar(iter);
}

STR_bool STR_X_UTF8_checkRunover(STR_StringIterator *iter){
    if(iter->iterator >= iter->maxLen){
        if(iter->error != STR_CHAR_ERROR_ENCODE_MISMATCH){
            iter->error = STR_CHAR_ERROR_DONE;
        }
        iter->iterator = ~((size_t)0);
        goto END;
    }
    if(iter->iterator + STR_X_UTF8_CalcMetaSize(iter->str[iter->iterator]) >= iter->maxLen){
        iter->error = STR_CHAR_ERROR_TRAILING_DATA;
        iter->iterator = ~((size_t)0);
    }
END:
    return iter->iterator < iter->maxLen;
}

void STR_X_UTF8_nextChar(STR_StringIterator *iter){

    //const unsigned char MAX_BYTES = 4;
    //const unsigned char ERR_CHECK = 1;
    const unsigned char PART_BITS = 6;
    const unsigned char PART_MASK = (1<<PART_BITS)-1;

    #if STR_LITTLE_ENDIAN == 0
    const unsigned int ILLEGAL_MASK_4 = 0x00C0C0C0;
    const unsigned int ILLEGAL_MASK_3 = 0x00C0C000;
    const unsigned int ILLEGAL_MASK_2 = 0x00C00000;
    const unsigned int ILLEGAL_INV_4 = 0x00808080;
    const unsigned int ILLEGAL_INV_3 = 0x00808000;
    const unsigned int ILLEGAL_INV_2 = 0x00800000;
    #else
    const unsigned int ILLEGAL_MASK_4 = 0xC0C0C000;
    const unsigned int ILLEGAL_MASK_3 = 0x00C0C000;
    const unsigned int ILLEGAL_MASK_2 = 0x0000C000;
    //const unsigned int ILLEGAL_MASK_1 = 0x00000000;
    const unsigned int ILLEGAL_INV_4 = 0x80808000;
    const unsigned int ILLEGAL_INV_3 = 0x00808000;
    const unsigned int ILLEGAL_INV_2 = 0x00008000;
    //const unsigned int ILLEGAL_INV_1 = 0x00000000;
    #endif

    iter->charPos++;

//*
    const unsigned char bytes = STR_X_UTF8_CalcMetaSize((unsigned char)iter->str[iter->iterator]);
    const size_t maxIter = iter->iterator+bytes;

    unsigned int val = *((unsigned int*)(iter->str+iter->iterator));
    switch(bytes){
        case(4):
            if( ((val&ILLEGAL_MASK_4)^ILLEGAL_INV_4) != 0 ){
                goto ASCII_8BIT;
            }
            break;
        case(3):
            if( ((val&ILLEGAL_MASK_3)^ILLEGAL_INV_3) != 0 ){
                //printf("%x :: %x :::: %x\n%x %x %x\n", val, (val&ILLEGAL_MASK_3), (val&ILLEGAL_MASK_3)^ILLEGAL_INV_3, (unsigned int)iter->str[iter->iterator], (unsigned int)iter->str[iter->iterator+1], (unsigned int)iter->str[iter->iterator+2]);
                goto ASCII_8BIT;
            }
            break;
        case(2):
            if( ((val&ILLEGAL_MASK_2)^ILLEGAL_INV_2) != 0 ){
            //if( STR_X_UTF8_CalcMetaSize(iter->str[iter->iterator+1]) != 1 ){
                goto ASCII_8BIT;
            }
            break;
        return;
        case(1):
            goto ASCII_8BIT;
        return;
        case(0):
            iter->character = ((size_t)0)^ ((unsigned char)iter->str[iter->iterator]);
            iter->iterator++;
            return;
        default:
            goto ASCII_8BIT;
    }

    iter->error = STR_CHAR_ERROR_NONE;
    iter->character = ((unsigned char)iter->str[iter->iterator]) & ((1<<(7-bytes))-1);
    iter->iterator++;

    for(;iter->iterator < maxIter; iter->iterator++){
        iter->character <<= PART_BITS;
        iter->character ^= (unsigned char)iter->str[iter->iterator] & PART_MASK;
    }

    return;

ASCII_8BIT:
    puts("8BIT ASCII");
    iter->charPos++;
    iter->iterator = (maxIter-bytes)+1;
    iter->character = ((size_t)0) ^ ((unsigned char)iter->str[iter->iterator-1]);
    iter->error = STR_CHAR_ERROR_ASCII_8BIT;

}

void STR_X_UTF16_initChar(const STR_String *string, STR_StringIterator *iter){

    iter->iterator = 0;

    STR_ENCODING_ENUM encoding = STR_StringGetExplicitEncoding(string);
    STR_ENCODING_ENUM encodingBOM = STR_StringGetImplicitEncoding(string);

    if(encodingBOM == STR_ENCODING_UNKOWN || encodingBOM == STR_ENCODING_ANSI){
        if(encoding == STR_ENCODING_UTF16_BE || encoding == STR_ENCODING_UTF16_LE){
            encodingBOM = encoding;
        }else if(encoding == STR_ENCODING_UNKOWN){
            encodingBOM = STR_ENCODING_ENUM_SIZE;
        }
    }else if(encodingBOM == STR_ENCODING_UTF16_BE || encodingBOM == STR_ENCODING_UTF16_LE){
        if(encodingBOM == STR_ENCODING_UTF16_BE){
            iter->iterator = STR_BOM_UTF16_BE_SIZE;
        }else{
            iter->iterator = STR_BOM_UTF16_LE_SIZE;
        }
    }

    iter->encoding = encodingBOM;

    switch(encodingBOM){
        case(STR_ENCODING_UTF16_BE):
            iter->endianess = 0;
            break;
        case(STR_ENCODING_UTF16_LE):
            iter->endianess = 1;
            break;
        default:
            iter->charPos = ~((size_t)0);
            iter->character = ~((size_t)0);
            iter->iterator = ~((size_t)0);
            iter->error = STR_CHAR_ERROR_ENCODE_MISMATCH;
            return;
    }

    iter->charPos = 0;
    iter->error = STR_CHAR_ERROR_NONE;
    iter->character = 0;
    iter->str = STR_StringGetDataPointer(string);
    iter->maxLen = STR_StringGetByteLength(string);

    STR_X_UTF16_nextChar(iter);
}

STR_bool STR_X_UTF16_checkRunover(STR_StringIterator *iter){
    return iter->error != STR_CHAR_ERROR_DONE && iter->error != STR_CHAR_ERROR_TRAILING_DATA && iter->error != STR_CHAR_ERROR_ENCODE_MISMATCH;
}

void STR_X_UTF16_nextChar(STR_StringIterator *iter){

    const size_t BYTES_SIZE_CHUNK = 2;

    //const unsigned char HIGH_BYTE = 0xD8;
    //const unsigned char LOW_BYTE = 0xDC00;
    //const unsigned char SURROGATE_MASK = 0xFC00;
    //const size_t FULL_SURROGATE_DATA_MASK = 0x103FF;
    //const unsigned char FULL_SURROGATE_DATA_MASK_BITS = 10;
    const unsigned char SURROGATE_SHIFT = 10;
    const unsigned char SURROGATE_MASK_X = 0x3F;
    const unsigned char SURROGATE_HIGH = 0x36;
    const unsigned char SURROGATE_LOW = 0x37;
    const size_t SURROGATE_PART_MASK = 0x3FF;
    const unsigned char SURROGATE_PART_SHIFT = 10;

    if(iter->iterator >= iter->maxLen){
        if(iter->error != STR_CHAR_ERROR_ENCODE_MISMATCH){
            iter->error = STR_CHAR_ERROR_DONE;
        }
        iter->iterator = ~((size_t)0);
        return;
    }

    if(iter->iterator + BYTES_SIZE_CHUNK >= iter->maxLen){
        iter->charPos = ~((size_t)0);
        iter->character = ~((size_t)0);
        iter->iterator = ~((size_t)0);
        iter->error = STR_CHAR_ERROR_TRAILING_DATA;
        return;
    }

    iter->error = STR_CHAR_ERROR_NONE;

    iter->character ^= iter->character;
    iter->character = (unsigned char)iter->str[iter->iterator + iter->endianess];
    iter->character <<= STR_CHAR_BITS;
    iter->character ^= (unsigned char)iter->str[iter->iterator + (iter->endianess ^ 1)];
    iter->iterator += BYTES_SIZE_CHUNK;

    if(((iter->character>>SURROGATE_SHIFT) & SURROGATE_MASK_X) == SURROGATE_HIGH){
        if(iter->iterator >= iter->maxLen){
            iter->iterator = ~((size_t)0);
            iter->error = STR_CHAR_ERROR_TRAILING_DATA;
        }else{

            iter->character <<= STR_CHAR_BITS;
            iter->character ^= (unsigned char)iter->str[iter->iterator + iter->endianess];
            iter->character <<= STR_CHAR_BITS;
            iter->character ^= (unsigned char)iter->str[iter->iterator + (iter->endianess ^ 1)];

            if(((iter->character>>SURROGATE_SHIFT) & SURROGATE_MASK_X) != SURROGATE_LOW){
                iter->iterator -= BYTES_SIZE_CHUNK;
                iter->character >>= (STR_CHAR_BITS*2);
                iter->character &= 0xFFFF;
                iter->error = STR_CHAR_ERROR_UNPAIRED_DATA;
            }else{
                iter->character =
                    (((iter->character>>(STR_CHAR_BITS*2))&SURROGATE_PART_MASK) << SURROGATE_PART_SHIFT)
                    | (iter->character&SURROGATE_PART_MASK);
                iter->character+= 0x10000;
                iter->iterator += BYTES_SIZE_CHUNK;
            }
        }
    }else if(((iter->character>>SURROGATE_SHIFT) & SURROGATE_MASK_X) == SURROGATE_LOW){
        iter->error = STR_CHAR_ERROR_UNPAIRED_DATA;
    }

    iter->charPos++;

}

void STR_X_UTF32_initChar(const STR_String *string, STR_StringIterator *iter){

    ///TODO FIXME ALL ENCODINGS
    ///check and offset if BOM exists
    ///if BOM does not match specified, error and cancel

    iter->iterator = 0;

    STR_ENCODING_ENUM encoding = STR_StringGetExplicitEncoding(string);
    STR_ENCODING_ENUM encodingBOM = STR_StringGetImplicitEncoding(string);
    if(encodingBOM == STR_ENCODING_UNKOWN || encodingBOM == STR_ENCODING_ANSI){
        if(encoding == STR_ENCODING_UTF32_BE || encoding == STR_ENCODING_UTF32_LE){
            encodingBOM = encoding;
        }else if(encoding == STR_ENCODING_UNKOWN){
            encodingBOM = STR_ENCODING_ENUM_SIZE;
        }
    }else if(encodingBOM == STR_ENCODING_UTF32_BE || encodingBOM == STR_ENCODING_UTF32_LE){
        if(encodingBOM == STR_ENCODING_UTF32_BE){
            iter->iterator = STR_BOM_UTF32_BE_SIZE;
        }else{
            iter->iterator = STR_BOM_UTF32_LE_SIZE;
        }
    }

    iter->encoding = encodingBOM;

    switch(encodingBOM){
        case(STR_ENCODING_UTF32_BE):
            iter->endianess = 0;
            break;
        case(STR_ENCODING_UTF32_LE):
            iter->endianess = 1;
            break;
        default:
            iter->charPos = ~((size_t)0);
            iter->character = ~((size_t)0);
            iter->iterator = ~((size_t)0);
            iter->error = STR_CHAR_ERROR_ENCODE_MISMATCH;
            return;
    }

    iter->charPos = 0;
    iter->error = STR_CHAR_ERROR_NONE;
    iter->character = 0;
    iter->str = STR_StringGetDataPointer(string);
    iter->maxLen = STR_StringGetByteLength(string);

    STR_X_UTF32_nextChar(iter);
}

STR_bool STR_X_UTF32_checkRunover(STR_StringIterator *iter){
    if(iter->iterator < iter->maxLen){
        return 1;
    }
    iter->error = STR_CHAR_ERROR_DONE;
    return 0;
}

void STR_X_UTF32_nextChar(STR_StringIterator *iter){

    const size_t BYTES_SIZE_CHUNK = 4;

    iter->character ^= iter->character;

    const unsigned char posM = (iter->endianess << 1)|(iter->endianess);
    iter->character ^= (unsigned char)iter->str[iter->iterator + posM];
    iter->character<<=STR_CHAR_BITS;
    iter->character ^= (unsigned char)iter->str[iter->iterator + 1 + iter->endianess];
    iter->character<<=STR_CHAR_BITS;
    iter->character ^= (unsigned char)iter->str[iter->iterator + 2 - iter->endianess];
    iter->character<<=STR_CHAR_BITS;
    iter->character ^= (unsigned char)iter->str[iter->iterator + (posM^3)];

    iter->iterator += BYTES_SIZE_CHUNK;
    iter->charPos++;
}
