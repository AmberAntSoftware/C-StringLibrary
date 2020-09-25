#include "str_string.h"
#include "str_string_ansi.h"

size_t STR_CharSizeANSI(size_t codePoint){
    return 8/CHAR_BIT + ((8-((8/CHAR_BIT)*CHAR_BIT))!=0);
}

STR_Char *STR_newCharANSI(size_t codePoint){
    STR_Char *chr = malloc(sizeof(chr));
    if(chr == NULL){
        return NULL;
    }
    if(STR_initCharANSI(chr, codePoint) == STR_ERROR_FAILURE || chr->byteCount == 0){
        free(chr);
        return NULL;
    }
    return chr;
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

STR_ERROR_ENUM STR_StringIterStartANSI(const STR_String *string, STR_StringIterator *iter){

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
        return STR_ERROR_FAILURE;
    }

    iter->charPos = 0;
    iter->error = STR_CHAR_ERROR_NONE;
    iter->character = 0;

    iter->str = STR_StringReadDataPointer(string);
    iter->maxLen = STR_StringGetByteLength(string);

    STR_StringIterNext(iter);
    return STR_ERROR_SUCCESS;
}

STR_bool STR_StringIterNextCheckANSI(STR_StringIterator *iter){
    if(iter->iterator >= iter->maxLen){
        if(iter->error != STR_CHAR_ERROR_ENCODE_MISMATCH){
            iter->error = STR_CHAR_ERROR_DONE;
        }
        return STR_FALSE;
    }
    return STR_TRUE;
}

void STR_StringIterNextANSI(STR_StringIterator *iter){
    iter->character = (unsigned char)iter->str[iter->iterator];
    iter->charPos++;
    iter->iterator++;
}
