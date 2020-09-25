#ifndef STR_STRING_UTF32_H_INCLUDED
#define STR_STRING_UTF32_H_INCLUDED

size_t STR_CharSizeUTF32(size_t codePoint);

STR_Char* STR_newCharUTF32(size_t codePoint, STR_ENDIAN_ENUM endianess);
STR_ERROR_ENUM STR_initCharUTF32(STR_Char *chr, size_t codePoint, STR_ENDIAN_ENUM endianess);

///STR_StringIterStartUTF32
STR_ERROR_ENUM STR_StringIterStartUTF32(const STR_String *string, STR_StringIterator *iter);
STR_bool STR_StringIterNextCheckUTF32(STR_StringIterator *iter);
void STR_StringIterNextUTF32(STR_StringIterator *iter);

#endif // STR_STRING_UTF32_H_INCLUDED
