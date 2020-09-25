#include "str_string.h"
#include "str_string_convert.h"

size_t STR_StringConvertByteSize(const STR_String* toConvert, STR_ENCODING_ENUM newFormat){

    STR_ENCODING_ENUM curFormat = STR_StringGetEncoding(toConvert);
    if(curFormat == newFormat){
        return 0;
    }

    switch(curFormat){
        case(STR_ENCODING_ANSI):
            break;
        case(STR_ENCODING_UTF8):
            break;
        case(STR_ENCODING_UTF16_BE):
            break;
        case(STR_ENCODING_UTF16_LE):
            break;
        case(STR_ENCODING_UTF32_BE):
            break;
        case(STR_ENCODING_UTF32_LE):
            break;
        default:
            return 0;
    }

    size_t count = 0;

    if(newFormat == STR_ENCODING_ANSI){
        ///fixed width encoding, memoized total length for chars is possible to be constant time
        return (STR_StringGetCharLength(toConvert) * STR_CharSizeANSI(0));
    }

    if(newFormat == STR_ENCODING_UTF32_BE || newFormat == STR_ENCODING_UTF32_LE){
        ///fixed width encoding, memoized total length for chars is possible to be constant time
        return (STR_StringGetCharLength(toConvert) * STR_CharSizeUTF32(0));
    }

    STR_StringIterator iter;
    switch(curFormat){
        case(STR_ENCODING_ANSI):
            STR_StringForEachANSI(toConvert, &iter){
                count += STR_CharSize(iter.character, newFormat);
            }
            return count;
        case(STR_ENCODING_UTF8):
            STR_StringForEachUTF8(toConvert, &iter){
                count += STR_CharSize(iter.character, newFormat);
            }
            return count;
        case(STR_ENCODING_UTF16_BE):
        case(STR_ENCODING_UTF16_LE):
            STR_StringForEachUTF16(toConvert, &iter){
                count += STR_CharSize(iter.character, newFormat);
            }
            return count;
        case(STR_ENCODING_UTF32_BE):
        case(STR_ENCODING_UTF32_LE):
            STR_StringForEachUTF32(toConvert, &iter){
                count += STR_CharSize(iter.character, newFormat);
            }
            return count;
        default:
            return 0;
    }
}

STR_String* STR_StringConvertNew(const STR_String* toConvert, STR_ENCODING_ENUM newFormat){

    STR_String *str;
    if(STR_StringGetByteLength(toConvert) == 0){
        return NULL;
    }
    str = (STR_String *)calloc(1, sizeof(STR_String));
    if(str == NULL){
        return NULL;
    }

    if(STR_StringConvertInit(str, toConvert, newFormat) == STR_ERROR_FAILURE){
        free(str);
        return NULL;
    }

    return str;
}

STR_ERROR_ENUM STR_StringConvertInit(STR_String *initThis, const STR_String* toConvert, STR_ENCODING_ENUM newFormat){
    size_t size = STR_StringConvertByteSize(toConvert, newFormat);
    if(size == 0){
        return STR_ERROR_FAILURE;
    }

    if(size <= STR_SSO_CONTROL_ADDR){
        ///TODO SMALL
        if(STR_StringConvertToLocation(&initThis->sso, toConvert, newFormat) == STR_ERROR_FAILURE){
            return STR_ERROR_FAILURE;
        }
        if(STR_initStringComplex(initThis, initThis->sso, STR_StringGetByteLength(toConvert), newFormat) == STR_ERROR_FAILURE){
            return STR_ERROR_FAILURE;
        }
        return STR_ERROR_SUCCESS;
    }

    unsigned char *conv = (unsigned char *)malloc(sizeof(unsigned char)*size);

    if(conv == NULL){
        return STR_ERROR_FAILURE;
    }

    if(STR_StringConvertToLocation(conv, toConvert, newFormat) == STR_ERROR_FAILURE){
        free(conv);
        return STR_ERROR_FAILURE;
    }

    if(STR_initStringComplex(initThis, (char*)conv, STR_StringGetByteLength(toConvert), newFormat) == STR_ERROR_FAILURE){
        free(conv);
        return STR_ERROR_FAILURE;
    }

    return STR_ERROR_SUCCESS;
}

STR_ERROR_ENUM STR_StringConvertToLocation(void* rawLocation, const STR_String* toConvert, STR_ENCODING_ENUM newFormat){
    STR_StringIterator iter;
    STR_Char chr;
    size_t i,j;
    i = 0;
    switch(STR_StringGetEncoding(toConvert)){
        case(STR_ENCODING_ANSI):
            STR_StringForEachANSI(toConvert, &iter){
                STR_initChar(&chr, iter.character, newFormat);
                for(j = 0; j < chr.byteCount; j++){
                    ((unsigned char*)rawLocation)[i] = chr.bytes[j];
                    i++;
                }
            }
            return STR_ERROR_SUCCESS;
        case(STR_ENCODING_UTF8):
            STR_StringForEachUTF8(toConvert, &iter){
                STR_initChar(&chr, iter.character, newFormat);
                for(j = 0; j < chr.byteCount; j++){
                    ((unsigned char*)rawLocation)[i] = chr.bytes[j];
                    i++;
                }
            }
            return STR_ERROR_SUCCESS;
        case(STR_ENCODING_UTF16_BE):
        case(STR_ENCODING_UTF16_LE):
            STR_StringForEachUTF16(toConvert, &iter){
                STR_initChar(&chr, iter.character, newFormat);
                for(j = 0; j < chr.byteCount; j++){
                    ((unsigned char*)rawLocation)[i] = chr.bytes[j];
                    i++;
                }
            }
            return STR_ERROR_SUCCESS;
        case(STR_ENCODING_UTF32_BE):
        case(STR_ENCODING_UTF32_LE):
            STR_StringForEachUTF32(toConvert, &iter){
                STR_initChar(&chr, iter.character, newFormat);
                for(j = 0; j < chr.byteCount; j++){
                    ((unsigned char*)rawLocation)[i] = chr.bytes[j];
                    i++;
                }
            }
            return STR_ERROR_SUCCESS;
        default:
            //iter->error = STR_CHAR_ERROR_UNSUPPORTED_ENC;
            return STR_ERROR_FAILURE;
    }

    return STR_ERROR_FAILURE;
}
