#ifndef FISHNET_H
#define FISHNET_H

#define FISHNET_EXPORT __attribute__((visibility("default")))

#ifdef __cplusplus
extern "C" {
#endif

FISHNET_EXPORT void Fishnet_init(bool enabled);

#ifdef __cplusplus
}
#endif

#endif //FISHNET_H
