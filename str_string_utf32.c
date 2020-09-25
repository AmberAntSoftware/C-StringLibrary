#include "str_string.h"
#include "str_string_utf32.h"

size_t STR_CharSizeUTF32(size_t codePoint){
    return 32/CHAR_BIT + ((32-((32/CHAR_BIT)*CHAR_BIT))!=0);
}

STR_Char *STR_newCharUTF32(size_t codePoint, STR_ENDIAN_ENUM endianess){

    STR_Char *chr = malloc(sizeof(chr));
    if(chr == NULL){
        return NULL;
    }

    STR_ERROR_ENUM ret;

    if(endianess == STR_ENDIAN_BIG){
        ret = STR_initCharUTF32(chr, codePoint, STR_ENDIAN_BIG);
    }else if(endianess == STR_ENDIAN_LITTLE){
        ret = STR_initCharUTF32(chr, codePoint, STR_ENDIAN_LITTLE);
    }else{
        free(chr);
        return NULL;
    }

    if(ret == STR_ERROR_FAILURE || chr->byteCount == 0){
        free(chr);
        return NULL;
    }

    return chr;
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

STR_ERROR_ENUM STR_StringIterStartUTF32(const STR_String *string, STR_StringIterator *iter){

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
            return STR_ERROR_FAILURE;
    }

    iter->charPos = 0;
    iter->error = STR_CHAR_ERROR_NONE;
    iter->character = 0;
    iter->str = STR_StringReadDataPointer(string);
    iter->maxLen = STR_StringGetByteLength(string);

    STR_StringIterNextUTF32(iter);
    return STR_ERROR_SUCCESS;
}

STR_bool STR_StringIterNextCheckUTF32(STR_StringIterator *iter){
    if(iter->iterator < iter->maxLen){
        return 1;
    }
    iter->error = STR_CHAR_ERROR_DONE;
    return 0;
}

void STR_StringIterNextUTF32(STR_StringIterator *iter){

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
