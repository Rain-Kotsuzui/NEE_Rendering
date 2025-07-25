#ifndef HIT_H
#define HIT_H

#include <vecmath.h>
#include "material.hpp"

class Hit {
public:

    // constructors
    Hit() {
        material = nullptr;
        t = 1e38;
    }

    Hit(float _t, Material *m, const Vector3f &n) {
        t = _t;
        material = m;
        normal = n;
    }
    Hit(const Vector3f &n) {
        t = 1e38;
        material = nullptr;
        normal = n;
    }

    Hit(const Hit &h) {
        t = h.t;
        material = h.material;
        normal = h.normal;
    }
    
    // destructor
    ~Hit() = default;
    
    void resetRemainMaterial_Normal() {
        t = 1e38;
    }
    float getT() const {
        return t;
    }
    
    Material *getMaterial() const {
        return material;
    }

    const Vector3f &getNormal() const {
        return normal;
    }

    void set(float _t, Material *m, const Vector3f &n) {
        t = _t;
        material = m;
        normal = n;
    }
    void setMaterial(Material *m) {
        material = m;
    }

private:
    float t;
    Material *material;
    Vector3f normal;
};

inline std::ostream &operator<<(std::ostream &os, const Hit &h) {
    //os << "Hit <" << h.getT() << ", " << h.getNormal() << ">";
    return os;
}

#endif // HIT_H
