#ifndef STR_STRING_UTF8_H_INCLUDED
#define STR_STRING_UTF8_H_INCLUDED

size_t STR_CharSizeUTF8(size_t codePoint);

STR_Char* STR_newCharUTF8(size_t codePoint);
STR_ERROR_ENUM STR_initCharUTF8(STR_Char *chr, size_t codePoint);

STR_ERROR_ENUM STR_StringIterStartUTF8(const STR_String *string, STR_StringIterator *iter);
STR_bool STR_StringIterNextCheckUTF8(STR_StringIterator *iter);
void STR_StringIterNextUTF8(STR_StringIterator *iter);

unsigned char STR_X_UTF8_CalcMetaSize(unsigned char charByte);


#endif // STR_STRING_UTF8_H_INCLUDED
