#ifndef STR_STRING_ANSI_H_INCLUDED
#define STR_STRING_ANSI_H_INCLUDED

size_t STR_CharSizeANSI(size_t codePoint);

STR_Char* STR_newCharANSI(size_t codePoint);
STR_ERROR_ENUM STR_initCharANSI(STR_Char *chr, size_t codePoint);


STR_ERROR_ENUM STR_StringIterStartANSI(const STR_String *string, STR_StringIterator *iter);
STR_bool STR_StringIterNextCheckANSI(STR_StringIterator *iter);
void STR_StringIterNextANSI(STR_StringIterator *iter);

#endif // STR_STRING_ANSI_H_INCLUDED
