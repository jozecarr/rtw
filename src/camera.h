#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"
#include <thread>
#include <vector>
#include <mutex>
#include <sstream>
#include <atomic>

class camera {
    public:
    double aspect_ratio      = 1.0;  // ratio of image width over height
    int    image_width       = 100;  // rendered image width in pixel count
    int    samples_per_pixel = 10;   // count of random samples for each pixel
    int    max_depth         = 10;   // maximum number of ray bounces into scene
    colour background;               // scene background colour  

    double vfov = 90; // vertical view angle (field of view)
    point3 lookfrom = point3(0,0,0); // point cam is looking from
    point3 lookat   = point3(0,0,-1); // point cam is looking at
    vec3   vup      = vec3(0,1,0); // cam relative up direction

    double defocus_angle = 0; // variation angle of rays through each pixel
    double focus_dist = 10; // distance from camera lookfrom point to plane of perfect focus

    void render(const hittable& world, int num_threads = 0) {
        initialize(); 
        
        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        int threads_to_use = num_threads > 0 ? num_threads : std::thread::hardware_concurrency();
        std::vector<std::thread> threads;
        std::vector<std::string> scanline_buffers(image_height);
        std::atomic<int> rows_done{0};
        std::mutex log_mutex;
        std::vector<int> thread_totals(threads_to_use, 0);
        std::vector<std::atomic<int>> thread_rows_done(threads_to_use);

        auto log_progress = [&](int done) {
            std::lock_guard<std::mutex> lock(log_mutex);
            std::ostringstream ss;
            ss << "\rTotal " << done << '/' << image_height << " | ";
            for (int idx = 0; idx < threads_to_use; idx++) {
                ss << 'T' << idx << ':' << thread_rows_done[idx].load() << '/' << thread_totals[idx];
                if (idx + 1 < threads_to_use) ss << ' ';
            }
            std::clog << ss.str() << std::flush;
        };

        int rows_per_thread = image_height / threads_to_use;
        for (int t = 0; t < threads_to_use; t++) {
            int start = t * rows_per_thread;
            int end = (t == threads_to_use - 1) ? image_height : start + rows_per_thread;
            thread_totals[t] = end - start;
            thread_rows_done[t] = 0;

            threads.emplace_back([this, &world, &scanline_buffers, &rows_done, &log_progress, &thread_rows_done, &thread_totals, t, start, end]() {
                for (int j = start; j < end; j++) {
                    std::ostringstream scanline_stream;
                    for (int i = 0; i < image_width; i++) {
                        colour pixel_colour(0,0,0);
                        for (int sample = 0; sample < samples_per_pixel; sample++) {
                            ray r = get_ray(i, j);
                            pixel_colour += ray_colour(r, max_depth, world);
                        }
                        write_colour(scanline_stream, pixel_samples_scale * pixel_colour);
                    }
                    scanline_buffers[j] = scanline_stream.str();

                    int done = ++rows_done;
                    int thread_done = ++thread_rows_done[t];
                    if (thread_done % 10 == 0 || thread_done == thread_totals[t]) {
                        log_progress(done);
                    }
                }
            });
        }

        for (auto& th : threads) th.join();
        for (int j = 0; j < image_height; j++) std::cout << scanline_buffers[j];

        log_progress(rows_done.load());
        std::clog << '\n' << "Done." << std::endl;
    };

    private:
        int    image_height;         // Rendered image height
        double pixel_samples_scale;  // Colour scale factor for a sum of pixel samples
        point3 center;               // Camera center
        point3 pixel00_loc;          // Location of pixel 0, 0
        vec3   pixel_delta_u;        // Offset to pixel to the right
        vec3   pixel_delta_v;        // Offset to pixel below
        vec3 u, v, w;
        vec3 defocus_disk_u;
        vec3 defocus_disk_v;

        void initialize() {
            image_height = int(image_width / aspect_ratio);
            image_height = (image_height < 1) ? 1 : image_height;

            pixel_samples_scale = 1.0 / samples_per_pixel;

            center = lookfrom;

            // Determine viewport dimensions.
            auto theta = degrees_to_radians(vfov);
            auto h = std::tan(theta/2);
            auto viewport_height = 2 * h * focus_dist;
            auto viewport_width = viewport_height * (double(image_width)/image_height);

            // calculate u,v,w uit basis vectors for the cams coord frame
            w = unit_vector(lookfrom - lookat);
            u = unit_vector(cross(vup, w));
            v = cross(w, u);

            // Calculate the vectors across the horizontal and down the vertical viewport edges.
            auto viewport_u = viewport_width * u;
            auto viewport_v = viewport_height * -v;

            // Calculate the horizontal and vertical delta vectors from pixel to pixel.
            pixel_delta_u = viewport_u / image_width;
            pixel_delta_v = viewport_v / image_height;

            // Calculate the location of the upper left pixel.
            auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
            pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

            // calculate camera defocus disk basis vectors
            auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle/2));
            defocus_disk_u = u * defocus_radius;
            defocus_disk_v = v * defocus_radius;
        }

        ray get_ray(int i, int j) const {
            // Construct a camera ray originating from the defocus disk and directed at a randomly
            // sampled point around the pixel location i, j.

            auto offset = sample_square();
            auto pixel_sample = pixel00_loc
                            + ((i + offset.x()) * pixel_delta_u)
                            + ((j + offset.y()) * pixel_delta_v);

            auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
            auto ray_direction = pixel_sample - ray_origin;
            auto ray_time = random_double();

            return ray(ray_origin, ray_direction, ray_time);
        }

        vec3 sample_square() const {
            // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
            return vec3(random_double() - 0.5, random_double() - 0.5, 0);
        }

        point3 defocus_disk_sample() const {
            // Returns a random point in the camera defocus disk.
            auto p = random_in_unit_disk();
            return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
        }
        
        colour ray_colour(const ray& r, int depth, const hittable& world) const {
            if (depth <= 0) return colour(0,0,0);

            hit_record rec;

            if (!world.hit(r, interval(0.001, infinity), rec)) return background;

            ray scattered;
            colour attenuation;
            colour colour_from_emission = rec.mat->emitted(rec.u, rec.v, rec.p);

            if (!rec.mat->scatter(r, rec, attenuation, scattered)) return colour_from_emission;

            colour colour_from_scatter = attenuation * ray_colour(scattered, depth-1, world);

            return colour_from_emission + colour_from_scatter;
        }
};

#endif
