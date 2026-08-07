#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"

class triangle : public hittable {
    public:
        // Triangle defined by three vertices
        triangle(const point3& v0, const point3& v1, const point3& v2, shared_ptr<material> mat)
            : v0(v0), v1(v1), v2(v2), mat(mat)
        {
            edge1 = v1 - v0;
            edge2 = v2 - v0;
            normal = unit_vector(cross(edge1, edge2));
            set_bounding_box();
        }

        // Triangle with explicit vertex normals (for smooth shading)
        triangle(const point3& v0, const point3& v1, const point3& v2,
                 const vec3& n0, const vec3& n1, const vec3& n2,
                 shared_ptr<material> mat)
            : v0(v0), v1(v1), v2(v2), n0(n0), n1(n1), n2(n2), mat(mat), has_vertex_normals(true)
        {
            edge1 = v1 - v0;
            edge2 = v2 - v0;
            normal = unit_vector(cross(edge1, edge2));
            set_bounding_box();
        }

        // Triangle with UVs
        triangle(const point3& v0, const point3& v1, const point3& v2,
                 float u0, float v0_uv, float u1, float v1_uv, float u2, float v2_uv,
                 shared_ptr<material> mat)
            : v0(v0), v1(v1), v2(v2),
              uv0{u0, v0_uv}, uv1{u1, v1_uv}, uv2{u2, v2_uv},
              mat(mat), has_uvs(true)
        {
            edge1 = v1 - v0;
            edge2 = v2 - v0;
            normal = unit_vector(cross(edge1, edge2));
            set_bounding_box();
        }

        void set_bounding_box() {
            auto min_x = std::fmin(std::fmin(v0.x(), v1.x()), v2.x());
            auto min_y = std::fmin(std::fmin(v0.y(), v1.y()), v2.y());
            auto min_z = std::fmin(std::fmin(v0.z(), v1.z()), v2.z());
            auto max_x = std::fmax(std::fmax(v0.x(), v1.x()), v2.x());
            auto max_y = std::fmax(std::fmax(v0.y(), v1.y()), v2.y());
            auto max_z = std::fmax(std::fmax(v0.z(), v1.z()), v2.z());

            bbox = aabb(point3(min_x, min_y, min_z), point3(max_x, max_y, max_z));
        }

        aabb bounding_box() const override { return bbox; }

        // Möller–Trumbore intersection algorithm
        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            const float epsilon = 1e-8;

            vec3 h = cross(r.direction(), edge2);
            float a = dot(edge1, h);

            // Ray is parallel to triangle
            if (std::fabs(a) < epsilon)
                return false;

            float f = 1.0 / a;
            vec3 s = r.origin() - v0;
            float u = f * dot(s, h);

            // Intersection outside triangle
            if (u < 0.0 || u > 1.0)
                return false;

            vec3 q = cross(s, edge1);
            float v = f * dot(r.direction(), q);

            // Intersection outside triangle
            if (v < 0.0 || u + v > 1.0)
                return false;

            float t = f * dot(edge2, q);

            if (!ray_t.contains(t))
                return false;

            rec.t = t;
            rec.p = r.at(t);
            rec.mat = mat;

            // Interpolate normal if we have vertex normals (smooth shading)
            vec3 outward_normal;
            if (has_vertex_normals) {
                float w = 1.0 - u - v;
                outward_normal = unit_vector(w * n0 + u * n1 + v * n2);
            } else {
                outward_normal = normal;
            }
            rec.set_face_normal(r, outward_normal);

            // Set UV coordinates
            if (has_uvs) {
                float w = 1.0 - u - v;
                rec.u = w * uv0[0] + u * uv1[0] + v * uv2[0];
                rec.v = w * uv0[1] + u * uv1[1] + v * uv2[1];
            } else {
                // Use barycentric coordinates as UVs
                rec.u = u;
                rec.v = v;
            }

            return true;
        }

    private:
        point3 v0, v1, v2;          // Vertices
        vec3 edge1, edge2;          // Precomputed edges
        vec3 normal;                // Face normal
        vec3 n0, n1, n2;            // Vertex normals (optional)
        float uv0[2], uv1[2], uv2[2];  // UV coordinates (optional)
        shared_ptr<material> mat;
        aabb bbox;
        bool has_vertex_normals = false;
        bool has_uvs = false;
};

// Helper to create a triangle mesh from vertex/index data
inline shared_ptr<hittable_list> triangle_mesh(
    const std::vector<point3>& vertices,
    const std::vector<int>& indices,
    shared_ptr<material> mat)
{
    auto mesh = make_shared<hittable_list>();

    for (size_t i = 0; i < indices.size(); i += 3) {
        mesh->add(make_shared<triangle>(
            vertices[indices[i]],
            vertices[indices[i + 1]],
            vertices[indices[i + 2]],
            mat
        ));
    }

    return mesh;
}

#endif
