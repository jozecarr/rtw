#ifndef HITTABLE_H
#define HITTABLE_H

#include "commons.h"
#include "aabb.h"

class material;

class hit_record {
    public: 
    point3 p;
    vec3 normal;
    shared_ptr<material> mat;
    float t;
    float u;
    float v;
    bool front_face;

    void set_face_normal(const ray& r, const vec3& outward_normal) {
        // Sets the hit record normal vector
        // assumes outward_normal has unit length

        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable {
    public:
        virtual ~hittable() = default;

        virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;

        virtual aabb bounding_box() const = 0;
};

class translate : public hittable {
    public:
        translate(shared_ptr<hittable> object, const vec3& offset) : object(object), offset(offset) {
            bbox = object->bounding_box() + offset;
        }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            // move ray backwards by offset
            ray offset_r(r.origin() - offset, r.direction(), r.time());

            // determine if and where intersection along offset ray
            if(!object->hit(offset_r, ray_t, rec)) return false;

            // move intersection point forwards by offset
            rec.p += offset;

            return true;
        }

        aabb bounding_box() const override { return bbox; }

    private:
        shared_ptr<hittable> object;
        vec3 offset;
        aabb bbox;
};

class rotate_y : public hittable {
    public:
        rotate_y(shared_ptr<hittable> object, float angle) : object(object) {
            auto radians = degrees_to_radians(angle);
            sin_theta = std::sin(radians);
            cos_theta = std::cos(radians);
            bbox = object->bounding_box();

            point3 min( infinity,  infinity,  infinity);
            point3 max(-infinity, -infinity, -infinity);

            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    for (int k = 0; k < 2; k++) {
                        auto x = i*bbox.x.max + (1-i)*bbox.x.min;
                        auto y = j*bbox.y.max + (1-j)*bbox.y.min;
                        auto z = k*bbox.z.max + (1-k)*bbox.z.min;

                        auto newx =  cos_theta*x + sin_theta*z;
                        auto newz = -sin_theta*x + cos_theta*z;

                        vec3 tester(newx, y, newz);

                        for (int c = 0; c < 3; c++) {
                            min[c] = std::fmin(min[c], tester[c]);
                            max[c] = std::fmax(max[c], tester[c]);
                        }
                    }
                }  
            }
            
            bbox = aabb(min, max);
        }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            // transform ray from world to object space

            auto origin = point3(
                (cos_theta * r.origin().x()) - (sin_theta * r.origin().z()),
                r.origin().y(),
                (sin_theta * r.origin().x()) + (cos_theta * r.origin().z())
            );

            auto direction = vec3(
                (cos_theta * r.direction().x()) - (sin_theta * r.direction().z()),
                r.direction().y(),
                (sin_theta * r.direction().x()) + (cos_theta * r.direction().z())
            );

            ray rotated_r(origin, direction, r.time());

            // check if and where intersection exists in object space 

            if(!object->hit(rotated_r, ray_t, rec)) return false;

            // transform intersection from object to world space

            rec.p = point3(
                (cos_theta * rec.p.x()) + (sin_theta * rec.p.z()),
                rec.p.y(),
                (-sin_theta * rec.p.x()) + (cos_theta * rec.p.z())
            );

            rec.normal = vec3(
                (cos_theta * rec.normal.x()) + (sin_theta * rec.normal.z()),
                rec.normal.y(),
                (-sin_theta * rec.normal.x()) + (cos_theta * rec.normal.z())
            );

            return true;
        }

        aabb bounding_box() const override { return bbox; }

    private:
        shared_ptr<hittable> object;
        float sin_theta;
        float cos_theta;
        aabb bbox;
};

class rotate_x : public hittable {
    public:
        rotate_x(shared_ptr<hittable> object, float angle) : object(object) {
            auto radians = degrees_to_radians(angle);
            sin_theta = std::sin(radians);
            cos_theta = std::cos(radians);
            bbox = object->bounding_box();

            point3 min( infinity,  infinity,  infinity);
            point3 max(-infinity, -infinity, -infinity);

            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    for (int k = 0; k < 2; k++) {
                        auto x = i*bbox.x.max + (1-i)*bbox.x.min;
                        auto y = j*bbox.y.max + (1-j)*bbox.y.min;
                        auto z = k*bbox.z.max + (1-k)*bbox.z.min;

                        auto newy =  cos_theta*y - sin_theta*z;
                        auto newz =  sin_theta*y + cos_theta*z;

                        vec3 tester(x, newy, newz);

                        for (int c = 0; c < 3; c++) {
                            min[c] = std::fmin(min[c], tester[c]);
                            max[c] = std::fmax(max[c], tester[c]);
                        }
                    }
                }
            }

            bbox = aabb(min, max);
        }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            // transform ray from world to object space (rotate backwards around X)
            auto origin = point3(
                r.origin().x(),
                (cos_theta * r.origin().y()) + (sin_theta * r.origin().z()),
                (-sin_theta * r.origin().y()) + (cos_theta * r.origin().z())
            );

            auto direction = vec3(
                r.direction().x(),
                (cos_theta * r.direction().y()) + (sin_theta * r.direction().z()),
                (-sin_theta * r.direction().y()) + (cos_theta * r.direction().z())
            );

            ray rotated_r(origin, direction, r.time());

            if(!object->hit(rotated_r, ray_t, rec)) return false;

            // transform intersection from object to world space
            rec.p = point3(
                rec.p.x(),
                (cos_theta * rec.p.y()) - (sin_theta * rec.p.z()),
                (sin_theta * rec.p.y()) + (cos_theta * rec.p.z())
            );

            rec.normal = vec3(
                rec.normal.x(),
                (cos_theta * rec.normal.y()) - (sin_theta * rec.normal.z()),
                (sin_theta * rec.normal.y()) + (cos_theta * rec.normal.z())
            );

            return true;
        }

        aabb bounding_box() const override { return bbox; }

    private:
        shared_ptr<hittable> object;
        float sin_theta;
        float cos_theta;
        aabb bbox;
};

class rotate_z : public hittable {
    public:
        rotate_z(shared_ptr<hittable> object, float angle) : object(object) {
            auto radians = degrees_to_radians(angle);
            sin_theta = std::sin(radians);
            cos_theta = std::cos(radians);
            bbox = object->bounding_box();

            point3 min( infinity,  infinity,  infinity);
            point3 max(-infinity, -infinity, -infinity);

            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    for (int k = 0; k < 2; k++) {
                        auto x = i*bbox.x.max + (1-i)*bbox.x.min;
                        auto y = j*bbox.y.max + (1-j)*bbox.y.min;
                        auto z = k*bbox.z.max + (1-k)*bbox.z.min;

                        auto newx =  cos_theta*x - sin_theta*y;
                        auto newy =  sin_theta*x + cos_theta*y;

                        vec3 tester(newx, newy, z);

                        for (int c = 0; c < 3; c++) {
                            min[c] = std::fmin(min[c], tester[c]);
                            max[c] = std::fmax(max[c], tester[c]);
                        }
                    }
                }
            }

            bbox = aabb(min, max);
        }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            // transform ray from world to object space (rotate backwards around Z)
            auto origin = point3(
                (cos_theta * r.origin().x()) + (sin_theta * r.origin().y()),
                (-sin_theta * r.origin().x()) + (cos_theta * r.origin().y()),
                r.origin().z()
            );

            auto direction = vec3(
                (cos_theta * r.direction().x()) + (sin_theta * r.direction().y()),
                (-sin_theta * r.direction().x()) + (cos_theta * r.direction().y()),
                r.direction().z()
            );

            ray rotated_r(origin, direction, r.time());

            if(!object->hit(rotated_r, ray_t, rec)) return false;

            // transform intersection from object to world space
            rec.p = point3(
                (cos_theta * rec.p.x()) - (sin_theta * rec.p.y()),
                (sin_theta * rec.p.x()) + (cos_theta * rec.p.y()),
                rec.p.z()
            );

            rec.normal = vec3(
                (cos_theta * rec.normal.x()) - (sin_theta * rec.normal.y()),
                (sin_theta * rec.normal.x()) + (cos_theta * rec.normal.y()),
                rec.normal.z()
            );

            return true;
        }

        aabb bounding_box() const override { return bbox; }

    private:
        shared_ptr<hittable> object;
        float sin_theta;
        float cos_theta;
        aabb bbox;
};

// Arbitrary axis rotation using Rodrigues' rotation formula
class rotate : public hittable {
    public:
        rotate(shared_ptr<hittable> object, const vec3& axis, float angle) : object(object) {
            this->axis = unit_vector(axis);
            auto radians = degrees_to_radians(angle);
            sin_theta = std::sin(radians);
            cos_theta = std::cos(radians);
            one_minus_cos = 1.0 - cos_theta;

            // Precompute rotation matrix elements
            float x = this->axis.x(), y = this->axis.y(), z = this->axis.z();

            // Row 1
            m[0][0] = cos_theta + x*x*one_minus_cos;
            m[0][1] = x*y*one_minus_cos - z*sin_theta;
            m[0][2] = x*z*one_minus_cos + y*sin_theta;

            // Row 2
            m[1][0] = y*x*one_minus_cos + z*sin_theta;
            m[1][1] = cos_theta + y*y*one_minus_cos;
            m[1][2] = y*z*one_minus_cos - x*sin_theta;

            // Row 3
            m[2][0] = z*x*one_minus_cos - y*sin_theta;
            m[2][1] = z*y*one_minus_cos + x*sin_theta;
            m[2][2] = cos_theta + z*z*one_minus_cos;

            // Inverse rotation matrix (transpose for orthogonal matrix)
            m_inv[0][0] = m[0][0]; m_inv[0][1] = m[1][0]; m_inv[0][2] = m[2][0];
            m_inv[1][0] = m[0][1]; m_inv[1][1] = m[1][1]; m_inv[1][2] = m[2][1];
            m_inv[2][0] = m[0][2]; m_inv[2][1] = m[1][2]; m_inv[2][2] = m[2][2];

            // Compute rotated bounding box
            bbox = object->bounding_box();
            point3 min( infinity,  infinity,  infinity);
            point3 max(-infinity, -infinity, -infinity);

            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    for (int k = 0; k < 2; k++) {
                        auto corner = vec3(
                            i*bbox.x.max + (1-i)*bbox.x.min,
                            j*bbox.y.max + (1-j)*bbox.y.min,
                            k*bbox.z.max + (1-k)*bbox.z.min
                        );

                        vec3 rotated = rotate_vec(corner);

                        for (int c = 0; c < 3; c++) {
                            min[c] = std::fmin(min[c], rotated[c]);
                            max[c] = std::fmax(max[c], rotated[c]);
                        }
                    }
                }
            }

            bbox = aabb(min, max);
        }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            // Transform ray from world to object space (inverse rotation)
            auto origin = rotate_vec_inv(r.origin());
            auto direction = rotate_vec_inv(r.direction());
            ray rotated_r(origin, direction, r.time());

            if(!object->hit(rotated_r, ray_t, rec)) return false;

            // Transform intersection from object to world space
            rec.p = rotate_vec(rec.p);
            rec.normal = rotate_vec(rec.normal);

            return true;
        }

        aabb bounding_box() const override { return bbox; }

    private:
        shared_ptr<hittable> object;
        vec3 axis;
        float sin_theta;
        float cos_theta;
        float one_minus_cos;
        float m[3][3];      // Rotation matrix
        float m_inv[3][3];  // Inverse rotation matrix
        aabb bbox;

        vec3 rotate_vec(const vec3& v) const {
            return vec3(
                m[0][0]*v.x() + m[0][1]*v.y() + m[0][2]*v.z(),
                m[1][0]*v.x() + m[1][1]*v.y() + m[1][2]*v.z(),
                m[2][0]*v.x() + m[2][1]*v.y() + m[2][2]*v.z()
            );
        }

        vec3 rotate_vec_inv(const vec3& v) const {
            return vec3(
                m_inv[0][0]*v.x() + m_inv[0][1]*v.y() + m_inv[0][2]*v.z(),
                m_inv[1][0]*v.x() + m_inv[1][1]*v.y() + m_inv[1][2]*v.z(),
                m_inv[2][0]*v.x() + m_inv[2][1]*v.y() + m_inv[2][2]*v.z()
            );
        }
};

#endif