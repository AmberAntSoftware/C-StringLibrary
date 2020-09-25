#include "str_string.h"
#include "str_string_utf16.h"

size_t STR_CharSizeUTF16(size_t codePoint){
    const size_t ENCODE_LIMIT = 0xFFFF;
    const size_t MAX_LIMIT = (((size_t)1)<<20)-1;

    if(codePoint > MAX_LIMIT){
        return 0;
    }

    if(codePoint > ENCODE_LIMIT){
        return 32/CHAR_BIT + ((32-((32/CHAR_BIT)*CHAR_BIT))!=0);
    }

    return 16/CHAR_BIT + ((16-((16/CHAR_BIT)*CHAR_BIT))!=0);
}

STR_Char *STR_newCharUTF16(size_t codePoint, STR_ENDIAN_ENUM endianess){

    STR_Char *chr = malloc(sizeof(chr));
    if(chr == NULL){
        return NULL;
    }

    STR_ERROR_ENUM ret;

    if(endianess == STR_ENDIAN_BIG){
        ret = STR_initCharUTF16(chr, codePoint, STR_ENDIAN_BIG);
    }else if(endianess == STR_ENDIAN_LITTLE){
        ret = STR_initCharUTF16(chr, codePoint, STR_ENDIAN_LITTLE);
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

STR_ERROR_ENUM STR_initCharUTF16(STR_Char *chr, size_t codePoint, STR_ENDIAN_ENUM endianess){

    const size_t MAX_LIMIT = (((size_t)1)<<20)-1;
    const size_t ENCODE_LIMIT = 0xFFFF;
    const unsigned char HIGH_BYTE = 0xD8;
    const unsigned char LOW_BYTE = 0xDC;
    const unsigned char SURROGATE_DATA_MASK = 3;
    const unsigned char SURROGATE_DATA_MASK_BITS = 2;
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

STR_ERROR_ENUM STR_StringIterStartUTF16(const STR_String *string, STR_StringIterator *iter){

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
            return STR_ERROR_FAILURE;
    }

    iter->charPos = 0;
    iter->error = STR_CHAR_ERROR_NONE;
    iter->character = 0;
    iter->str = STR_StringReadDataPointer(string);
    iter->maxLen = STR_StringGetByteLength(string);

    STR_StringIterNextUTF16(iter);
    return STR_ERROR_SUCCESS;
}

STR_bool STR_StringIterNextCheckUTF16(STR_StringIterator *iter){
    return iter->error != STR_CHAR_ERROR_DONE && iter->error != STR_CHAR_ERROR_TRAILING_DATA && iter->error != STR_CHAR_ERROR_ENCODE_MISMATCH;
}

void STR_StringIterNextUTF16(STR_StringIterator *iter){

    const size_t BYTES_SIZE_CHUNK = 2;

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
                iter->character >>= (STR_CHAR_BITS*BYTES_SIZE_CHUNK);
                iter->character &= 0xFFFF;
                iter->error = STR_CHAR_ERROR_UNPAIRED_DATA;
            }else{
                iter->character =
                    (((iter->character>>(STR_CHAR_BITS*BYTES_SIZE_CHUNK))&SURROGATE_PART_MASK) << SURROGATE_PART_SHIFT)
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
