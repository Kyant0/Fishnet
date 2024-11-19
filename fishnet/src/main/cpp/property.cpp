#include "property.h"

#include <bits/sysconf.h>
#include <cstring>
#include <sys/system_properties.h>

std::string get_property(const char *key, const char *default_value) {
    static char value[PROP_VALUE_MAX];
    if (__system_property_get(key, value) > 0) {
        return value;
    }
    return default_value;
}

bool get_bool_property(const char *key, bool default_value) {
    return strcmp(get_property(key, default_value ? "1" : "0").c_str(), "1") == 0;
}

size_t get_page_size() {
#if defined(PAGE_SIZE)
    return PAGE_SIZE;
#else
    return sysconf(_SC_PAGESIZE);
#endif
}
