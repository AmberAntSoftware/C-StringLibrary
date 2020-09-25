#ifndef STR_STRING_UTF16_H_INCLUDED
#define STR_STRING_UTF16_H_INCLUDED

size_t STR_CharSizeUTF16(size_t codePoint);

STR_Char* STR_newCharUTF16(size_t codePoint, STR_ENDIAN_ENUM endianess);
STR_ERROR_ENUM STR_initCharUTF16(STR_Char *chr, size_t codePoint, STR_ENDIAN_ENUM endianess);

STR_ERROR_ENUM STR_StringIterStartUTF16(const STR_String *string, STR_StringIterator *iter);
STR_bool STR_StringIterNextCheckUTF16(STR_StringIterator *iter);
void STR_StringIterNextUTF16(STR_StringIterator *iter);

#endif // STR_STRING_UTF16_H_INCLUDED
