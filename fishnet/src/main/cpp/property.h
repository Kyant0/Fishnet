#ifndef FISHNET_PROPERTY_H
#define FISHNET_PROPERTY_H

#include <string>

std::string get_property(const char *key, const char *default_value);

bool get_bool_property(const char *key, bool default_value);

#endif //FISHNET_PROPERTY_H
