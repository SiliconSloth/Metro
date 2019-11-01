#include "gitwrapper/strarray.h"

StrArray::StrArray(git_strarray array) {
    this->array = array;
}

StrArray::~StrArray() {
    git_strarray_free(&array);
}

char **StrArray::strings() {
    return array.strings;
}

size_t StrArray::count() {
    return array.count;
}