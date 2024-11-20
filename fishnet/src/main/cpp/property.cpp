#include "property.h"

#include <sys/system_properties.h>

std::string get_property(const char *key, const char *default_value) {
    char value[PROP_VALUE_MAX];
    if (__system_property_get(key, value) > 0) {
        return value;
    }
    return default_value;
}

bool get_bool_property(const char *key, bool default_value) {
    return strcmp(get_property(key, default_value ? "1" : "0").c_str(), "1") == 0;
}
