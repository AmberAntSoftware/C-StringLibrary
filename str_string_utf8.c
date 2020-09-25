#include "str_string.h"
#include "str_string_utf8.h"

size_t STR_CharSizeUTF8(size_t codePoint){
    const unsigned char MAX_BITS = 21;
    const unsigned char LEN_4_THRESHOLD = 16;
    const unsigned char LEN_3_THRESHOLD = 11;
    const unsigned char LEN_2_THRESHOLD = 7;

    if(codePoint > (((size_t)1)<<MAX_BITS)-1){
        return 0;
    }

    if(codePoint > (((size_t)1)<<LEN_4_THRESHOLD)-1 ){
        return 32/CHAR_BIT + ((32-((32/CHAR_BIT)*CHAR_BIT))!=0);
    }

    if(codePoint > (((size_t)1)<<LEN_3_THRESHOLD)-1 ){
        return 24/CHAR_BIT + ((24-((24/CHAR_BIT)*CHAR_BIT))!=0);
    }

    if(codePoint > (((size_t)1)<<LEN_2_THRESHOLD)-1 ){
        return 16/CHAR_BIT + ((16-((16/CHAR_BIT)*CHAR_BIT))!=0);
    }

    return 8/CHAR_BIT + ((8-((8/CHAR_BIT)*CHAR_BIT))!=0);
}

STR_Char *STR_newCharUTF8(size_t codePoint){
    STR_Char *chr = malloc(sizeof(chr));
    if(chr == NULL){
        return NULL;
    }
    if(STR_initCharUTF8(chr, codePoint) == STR_ERROR_FAILURE || chr->byteCount == 0){
        free(chr);
        return NULL;
    }
    return chr;
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

STR_ERROR_ENUM STR_StringIterStartUTF8(const STR_String *string, STR_StringIterator *iter){

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
        return STR_ERROR_FAILURE;
    }

    iter->charPos = 0;
    iter->error = STR_CHAR_ERROR_NONE;
    iter->character = 0;

    iter->str = STR_StringReadDataPointer(string);
    iter->maxLen = STR_StringGetByteLength(string);

    STR_StringIterNextUTF8(iter);
    return STR_ERROR_SUCCESS;
}

STR_bool STR_StringIterNextCheckUTF8(STR_StringIterator *iter){
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

void STR_StringIterNextUTF8(STR_StringIterator *iter){

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
    const unsigned int ILLEGAL_INV_4 = 0x80808000;
    const unsigned int ILLEGAL_INV_3 = 0x00808000;
    const unsigned int ILLEGAL_INV_2 = 0x00008000;
    #endif

    iter->charPos++;

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
                goto ASCII_8BIT;
            }
            break;
        case(2):
            if( ((val&ILLEGAL_MASK_2)^ILLEGAL_INV_2) != 0 ){
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
    iter->charPos++;
    iter->iterator = (maxIter-bytes)+1;
    iter->character = ((size_t)0) ^ ((unsigned char)iter->str[iter->iterator-1]);
    iter->error = STR_CHAR_ERROR_ASCII_8BIT;

}

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
