#ifndef DRM_H
#define DRM_H

#include "candock/candockexport.hpp"


class CANDOCK_EXPORT drm {
    
public:
    static bool check_drm(const std::string& key_location);
    
};

#endif