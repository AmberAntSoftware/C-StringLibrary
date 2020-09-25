#ifndef STR_STRING_CONVERT_H_INCLUDED
#define STR_STRING_CONVERT_H_INCLUDED

size_t STR_StringConvertByteSize(const STR_String* toConvert, STR_ENCODING_ENUM newFormat);
size_t STR_StringConvertCharCount(const STR_String* toConvert, STR_ENCODING_ENUM newFormat);

STR_String* STR_StringConvertNew(const STR_String* toConvert, STR_ENCODING_ENUM newFormat);
STR_ERROR_ENUM STR_StringConvertInit(STR_String *initThis, const STR_String* toConvert, STR_ENCODING_ENUM newFormat);
STR_ERROR_ENUM STR_StringConvertToLocation(void* rawLocation, const STR_String* toConvert, STR_ENCODING_ENUM newFormat);

#endif // STR_STRING_CONVERT_H_INCLUDED
