#ifndef TEXTURE_H
#define TEXTURE_H

#include "raw_stb_image.h"
#include "perlin.h"

class texture {
    public:
        virtual ~texture() = default;

        virtual colour value(double u, double v, const point3& p) const = 0;
};

class solid_colour : public texture {
    public:
        solid_colour(const colour& albedo) : albedo(albedo) {}

        solid_colour(double red, double green, double blue) : solid_colour(colour(red, green, blue)) {}

        colour value(double u, double v, const point3& p) const override { return albedo; }

    private: 
        colour albedo;
};

class checker_texture : public texture {
    public:
        checker_texture(double scale, shared_ptr<texture> even, shared_ptr<texture> odd) : inv_scale(1.0 / scale), even(even), odd(odd) {}

        checker_texture(double scale, const colour& c1, const colour& c2) : checker_texture(scale, make_shared<solid_colour>(c1), make_shared<solid_colour>(c2)) {}

        colour value(double u, double v, const point3& p) const override {
            auto xInteger = int(std::floor(inv_scale * p.x()));
            auto yInteger = int(std::floor(inv_scale * p.y()));
            auto zInteger = int(std::floor(inv_scale * p.z()));
            
            bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

            return isEven ? even->value(u,v,p) : odd->value(u,v,p);
        }

    private:
        double inv_scale;
        shared_ptr<texture> even;
        shared_ptr<texture> odd;
};

class image_texture : public texture {
    public:
        image_texture(const char* filename) : image(filename) {}

        colour value(double u, double v, const point3& p) const override {
            if (image.height() <= 0) return colour(0, 1, 1);

            u = interval(0,1).clamp(u);
            v = 1.0 - interval(0,1).clamp(v);

            auto i = int(u * image.width());
            auto j = int(v * image.height());
            auto pixel = image.pixel_data(i,j);

            auto colour_scale = 1.0 / 255.0;
            return colour(colour_scale*pixel[0], colour_scale*pixel[1], colour_scale*pixel[2]);
        }
    private:
        rtw_image image;
};

class noise_texture : public texture {
    public:
        noise_texture(double scale) : scale(scale) {}

        colour value(double u, double v, const point3& p) const override {
            return colour(.5,.5,.5) * (1 + std::sin(scale * p.z() + 10 * noise.turb(p,7)));
        }

    private:
        perlin noise;
        double scale;
};

#endif
