#ifndef FISHNET_PROPERTY_H
#define FISHNET_PROPERTY_H

#include <string>
#include <sys/auxv.h>

std::string get_property(const char *key, const char *default_value);

bool get_bool_property(const char *key, bool default_value);

inline size_t get_page_size() {
#if defined(PAGE_SIZE)
    return PAGE_SIZE;
#else
    return (size_t) getauxval(AT_PAGESZ);
#endif
}

#endif //FISHNET_PROPERTY_H
