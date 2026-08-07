#include "commons.h"

#include "bvh.h"
#include "camera.h"
#include "constant_medium.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "quad.h"
#include "triangle.h"

#include <cstdlib>
#include <cstring>

//global settings
int scene = 5;
int thread_count = 0; // (0=automatic thread count)
bool do_threading = true;
int image_width = 0;       // (0=scene default)
int samples_per_pixel = 0; // (0=scene default)
int max_depth = 0;         // (0=scene default)

void print_usage(const char* exe) {
    std::cout
        << "Usage: " << (exe ? exe : "t") << " [scene] [threads] [threading] [width] [samples] [depth]\n\n"
        << "Arguments:\n"
        << "  scene      Scene id (default: 5)\n"
        << "  threads    Thread count (0 = automatic, default: 0)\n"
        << "  threading  Enable threading (1/0, default: 1)\n"
        << "  width      Image width in pixels (0 = scene default, default: 0)\n"
        << "  samples    Samples per pixel (0 = scene default, default: 0)\n"
        << "  depth      Max ray bounces (0 = scene default, default: 0)\n\n"
        << "Scenes:\n"
        << "  0  health_check\n"
        << "  1  lotso_spheres\n"
        << "  2  checkered_spheres\n"
        << "  3  earth\n"
        << "  4  perlin_spheres\n"
        << "  5  quads\n"
        << "  6  simple_light\n"
        << "  7  cornell_box\n"
        << "  8  cornell_smoke\n"
        << "  9  big_scene\n"
        << " 10  triangles_demo\n"
        << " 11  cyberpunk\n";
}

void health_check() { //よ
    hittable_list world;

    auto red   = make_shared<lambertian>(colour(0.65, 0.05, 0.05));
    
    world.add(make_shared<sphere>(point3(0,0,-1), 0.5, red ));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = image_width > 0 ? image_width : 1000;
    cam.samples_per_pixel = samples_per_pixel > 0 ? samples_per_pixel : 100;
    cam.max_depth         = max_depth > 0 ? max_depth : 50;
    cam.background        = colour(0.70, 0.80, 1.00);
    cam.vfov     = 20;
    cam.lookfrom = point3(3,3,2);
    cam.lookat   = point3(0,0,-1);
    cam.vup      = vec3(0,1,0);
    cam.defocus_angle = 0;

    if (thread_count > 0) {
        cam.render(world, thread_count);
    } else if (do_threading) {
        cam.render(world);
    } else {
        cam.render(world, 1);
    }
}

void lotso_spheres() {
    hittable_list world;

    auto checker = make_shared<checker_texture>(0.32, colour(.2, .3, .1), colour(.9, .9, .9));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(checker)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_float();
            point3 center(a + 0.9*random_float(), 0.2, b + 0.9*random_float());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = colour::random() * colour::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    auto center2 = center; //+ vec3(0, random_float(0, 0.5), 0);
                    world.add(make_shared<sphere>(center, center2, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = colour::random(0.5, 1);
                    auto fuzz = random_float(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(colour(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(colour(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    world = hittable_list(make_shared<bvh_node>(world));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = image_width > 0 ? image_width : 800;
    cam.samples_per_pixel = samples_per_pixel > 0 ? samples_per_pixel : 250;
    cam.max_depth         = max_depth > 0 ? max_depth : 50;
    cam.background        = colour(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    if (thread_count > 0) {
        cam.render(world, thread_count);
    } else if (do_threading) {
        cam.render(world);
    } else {
        cam.render(world, 1);
    }
}

void checkered_spheres() {
    hittable_list world;

    auto checker = make_shared<checker_texture>(0.32, colour(.2, .3, .1), colour(.9, .9, .9));

    world.add(make_shared<sphere>(point3(0,-10, 0), 10, make_shared<lambertian>(checker)));
    world.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = image_width > 0 ? image_width : 400;
    cam.samples_per_pixel = samples_per_pixel > 0 ? samples_per_pixel : 100;
    cam.max_depth         = max_depth > 0 ? max_depth : 50;
    cam.background        = colour(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    if (thread_count > 0) {
        cam.render(world, thread_count);
    } else if (do_threading) {
        cam.render(world);
    } else {
        cam.render(world, 1);
    }
}

void earth() {
    auto earth_texture = make_shared<image_texture>("images/earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(point3(0,0,0), 2, earth_surface);

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = image_width > 0 ? image_width : 400;
    cam.samples_per_pixel = samples_per_pixel > 0 ? samples_per_pixel : 100;
    cam.max_depth = max_depth > 0 ? max_depth : 50;
    cam.background        = colour(0.70, 0.80, 1.00);

    cam.vfov = 20;
    cam.lookfrom = point3(0,0,12);
    cam.lookat = point3(0,0,0);
    cam.vup = vec3(0,1,0);

    cam.defocus_angle = 0;

    if (thread_count > 0) {
        cam.render(hittable_list(globe), thread_count);
    } else if (do_threading) {
        cam.render(hittable_list(globe));
    } else {
        cam.render(hittable_list(globe), 1);
    }
}

void perlin_spheres() {
    hittable_list world;

    auto pertext = make_shared<noise_texture>(4);
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
    world.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = image_width > 0 ? image_width : 2560;
    cam.samples_per_pixel = samples_per_pixel > 0 ? samples_per_pixel : 100;
    cam.max_depth         = max_depth > 0 ? max_depth : 50;
    cam.background        = colour(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    if (thread_count > 0) {
        cam.render(world, thread_count);
    } else if (do_threading) {
        cam.render(world);
    } else {
        cam.render(world, 1);
    }
}

void quads() {
    hittable_list world;

    auto left_red     = make_shared<lambertian>(colour(1.0, 0.2, 0.2));
    auto back_green   = make_shared<lambertian>(colour(0.2, 1.0, 0.2));
    auto right_blue   = make_shared<lambertian>(colour(0.2, 0.2, 1.0));
    auto upper_orange = make_shared<lambertian>(colour(1.0, 0.5, 0.0));
    auto lower_teal   = make_shared<lambertian>(colour(0.2, 0.8, 0.8));

    world.add(make_shared<quad>(point3(-3,-2, 5), vec3(0, 0,-4), vec3(0, 4, 0), left_red));
    world.add(make_shared<quad>(point3(-2,-2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));
    world.add(make_shared<quad>(point3( 3,-2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
    world.add(make_shared<quad>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));
    world.add(make_shared<quad>(point3(-2,-3, 5), vec3(4, 0, 0), vec3(0, 0,-4), lower_teal));

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = image_width > 0 ? image_width : 400;
    cam.samples_per_pixel = samples_per_pixel > 0 ? samples_per_pixel : 100;
    cam.max_depth         = max_depth > 0 ? max_depth : 50;
    cam.background        = colour(0.70, 0.80, 1.00);

    cam.vfov     = 80;
    cam.lookfrom = point3(0,0,9);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    if (thread_count > 0) {
        cam.render(world, thread_count);
    } else if (do_threading) {
        cam.render(world);
    } else {
        cam.render(world, 1);
    }
}

void simple_light() {
    hittable_list world;

    auto pertext = make_shared<noise_texture>(4);
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
    world.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));

    auto difflight = make_shared<diffuse_light>(colour(4,4,4));
    world.add(make_shared<sphere>(point3(0,7,0), 2, difflight));
    world.add(make_shared<quad>(point3(3,1,-2), vec3(2,0,0), vec3(0,2,0), difflight));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = image_width > 0 ? image_width : 2000;
    cam.samples_per_pixel = samples_per_pixel > 0 ? samples_per_pixel : 100;
    cam.max_depth         = max_depth > 0 ? max_depth : 100;
    cam.background        = colour(0,0,0);

    cam.vfov     = 20;
    cam.lookfrom = point3(26,3,6);
    cam.lookat   = point3(0,2,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    if (thread_count > 0) {
        cam.render(world, thread_count);
    } else if (do_threading) {
        cam.render(world);
    } else {
        cam.render(world, 1);
    }
}

void cornell_box() {
    hittable_list world;

    auto red   = make_shared<lambertian>(colour(.65, .05, .05));
    auto white = make_shared<lambertian>(colour(.73, .73, .73));
    auto green = make_shared<lambertian>(colour(.12, .45, .15));
    auto light = make_shared<diffuse_light>(colour(15, 15, 15));

    world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(make_shared<quad>(point3(343, 554, 332), vec3(-130,0,0), vec3(0,0,-105), light));
    world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), white));
    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    shared_ptr<hittable> box1 = box(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));
    world.add(box1);

    shared_ptr<hittable> box2 = box(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));
    world.add(box2);

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = image_width > 0 ? image_width : 600;
    cam.samples_per_pixel = samples_per_pixel > 0 ? samples_per_pixel : 200;
    cam.max_depth         = max_depth > 0 ? max_depth : 50;
    cam.background        = colour(0,0,0);

    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    if (thread_count > 0) {
        cam.render(world, thread_count);
    } else if (do_threading) {
        cam.render(world);
    } else {
        cam.render(world, 1);
    }
}

void cornell_smoke() {
    hittable_list world;

    auto red   = make_shared<lambertian>(colour(.65, .05, .05));
    auto white = make_shared<lambertian>(colour(.73, .73, .73));
    auto green = make_shared<lambertian>(colour(.12, .45, .15));
    auto light = make_shared<diffuse_light>(colour(7, 7, 7));

    world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(make_shared<quad>(point3(113,554,127), vec3(330,0,0), vec3(0,0,305), light));
    world.add(make_shared<quad>(point3(0,555,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    shared_ptr<hittable> box1 = box(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));

    shared_ptr<hittable> box2 = box(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));

    world.add(make_shared<constant_medium>(box1, 0.01, colour(0,0,0)));
    world.add(make_shared<constant_medium>(box2, 0.01, colour(1,1,1)));

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = image_width > 0 ? image_width : 600;
    cam.samples_per_pixel = samples_per_pixel > 0 ? samples_per_pixel : 200;
    cam.max_depth         = max_depth > 0 ? max_depth : 50;
    cam.background        = colour(0,0,0);

    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    if (thread_count > 0) {
        cam.render(world, thread_count);
    } else if (do_threading) {
        cam.render(world);
    } else {
        cam.render(world, 1);
    }
}

void big_scene() {
    hittable_list boxes1;
    auto ground = make_shared<lambertian>(colour(0.48, 0.83, 0.53));

    int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i*w;
            auto z0 = -1000.0 + j*w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_float(1,101);
            auto z1 = z0 + w;

            boxes1.add(box(point3(x0,y0,z0), point3(x1,y1,z1), ground));
        }
    }

    hittable_list world;

    world.add(make_shared<bvh_node>(boxes1));

    auto light = make_shared<diffuse_light>(colour(7, 7, 7));
    world.add(make_shared<quad>(point3(123,554,147), vec3(300,0,0), vec3(0,0,265), light));

    auto center1 = point3(400, 400, 200);
    auto center2 = center1 + vec3(30,0,0);
    auto sphere_material = make_shared<lambertian>(colour(0.7, 0.3, 0.1));
    world.add(make_shared<sphere>(center1, center2, 50, sphere_material));

    world.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
    world.add(make_shared<sphere>(
        point3(0, 150, 145), 50, make_shared<metal>(colour(0.8, 0.8, 0.9), 1.0)
    ));

    auto boundary = make_shared<sphere>(point3(360,150,145), 70, make_shared<dielectric>(1.5));
    world.add(boundary);
    world.add(make_shared<constant_medium>(boundary, 0.2, colour(0.2, 0.4, 0.9)));
    boundary = make_shared<sphere>(point3(0,0,0), 5000, make_shared<dielectric>(1.5));
    world.add(make_shared<constant_medium>(boundary, .0001, colour(1,1,1)));

    auto emat = make_shared<lambertian>(make_shared<image_texture>("images/earthmap.jpg"));
    world.add(make_shared<sphere>(point3(400,200,400), 100, emat));
    auto pertext = make_shared<noise_texture>(0.2);
    world.add(make_shared<sphere>(point3(220,280,300), 80, make_shared<lambertian>(pertext)));

    hittable_list boxes2;
    auto white = make_shared<lambertian>(colour(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxes2.add(make_shared<sphere>(point3::random(0,165), 10, make_shared<lambertian>(colour(random_float(0,1),random_float(0,1),random_float(0,1)))));
    }

    world.add(make_shared<translate>(
        make_shared<rotate_y>(
            make_shared<bvh_node>(boxes2), 15),
            vec3(-100,270,395)
        )
    );

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = image_width > 0 ? image_width : 2560;
    cam.samples_per_pixel = samples_per_pixel > 0 ? samples_per_pixel : 1000;
    cam.max_depth         = max_depth > 0 ? max_depth : 100;
    cam.background        = colour(0,0,0);

    cam.vfov     = 40;
    cam.lookfrom = point3(478, 278, -600);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    if (thread_count > 0) {
        cam.render(world, thread_count);
    } else if (do_threading) {
        cam.render(world);
    } else {
        cam.render(world, 1);
    }
}

void triangles_demo() {
    hittable_list world;

    // Ground plane using two triangles
    auto ground = make_shared<lambertian>(colour(0.5, 0.5, 0.5));
    world.add(make_shared<triangle>(
        point3(-10, 0, -10), point3(10, 0, -10), point3(10, 0, 10), ground));
    world.add(make_shared<triangle>(
        point3(-10, 0, -10), point3(10, 0, 10), point3(-10, 0, 10), ground));

    // Pyramid made of triangles
    auto gold = make_shared<metal>(colour(0.8, 0.6, 0.2), 0.1);
    point3 apex(0, 2, 0);
    point3 bl(-1, 0, -1), br(1, 0, -1), fr(1, 0, 1), fl(-1, 0, 1);

    world.add(make_shared<triangle>(apex, bl, br, gold));  // back
    world.add(make_shared<triangle>(apex, br, fr, gold));  // right
    world.add(make_shared<triangle>(apex, fr, fl, gold));  // front
    world.add(make_shared<triangle>(apex, fl, bl, gold));  // left

    // Tetrahedron with rotate_x (tilted forward)
    auto glass = make_shared<dielectric>(1.5);
    auto tetra = make_shared<hittable_list>();
    float h = std::sqrt(2.0/3.0);
    point3 t0(0, h, 0);
    point3 t1(-0.5, 0, -std::sqrt(3.0)/6.0);
    point3 t2(0.5, 0, -std::sqrt(3.0)/6.0);
    point3 t3(0, 0, std::sqrt(3.0)/3.0);

    tetra->add(make_shared<triangle>(t0, t1, t2, glass));
    tetra->add(make_shared<triangle>(t0, t2, t3, glass));
    tetra->add(make_shared<triangle>(t0, t3, t1, glass));
    tetra->add(make_shared<triangle>(t1, t3, t2, glass));

    // Apply rotate_x (tilt 30 degrees) and translate
    shared_ptr<hittable> tilted_tetra = make_shared<rotate_x>(tetra, 30);
    tilted_tetra = make_shared<translate>(tilted_tetra, vec3(-2.5, 0.5, 0));
    world.add(tilted_tetra);

    // Octahedron with rotate_z (leaning sideways)
    auto red_mat = make_shared<lambertian>(colour(0.8, 0.1, 0.1));
    auto octa = make_shared<hittable_list>();
    point3 top(0, 1, 0), bot(0, -1, 0);
    point3 o1(1, 0, 0), o2(0, 0, 1), o3(-1, 0, 0), o4(0, 0, -1);

    // Top half
    octa->add(make_shared<triangle>(top, o1, o2, red_mat));
    octa->add(make_shared<triangle>(top, o2, o3, red_mat));
    octa->add(make_shared<triangle>(top, o3, o4, red_mat));
    octa->add(make_shared<triangle>(top, o4, o1, red_mat));
    // Bottom half
    octa->add(make_shared<triangle>(bot, o2, o1, red_mat));
    octa->add(make_shared<triangle>(bot, o3, o2, red_mat));
    octa->add(make_shared<triangle>(bot, o4, o3, red_mat));
    octa->add(make_shared<triangle>(bot, o1, o4, red_mat));

    // Apply rotate_z (lean 25 degrees) and translate
    shared_ptr<hittable> leaning_octa = make_shared<rotate_z>(octa, 25);
    leaning_octa = make_shared<translate>(leaning_octa, vec3(2.5, 1, 0));
    world.add(leaning_octa);

    // Diamond shape with arbitrary axis rotation
    auto blue_mat = make_shared<lambertian>(colour(0.2, 0.3, 0.8));
    auto diamond = make_shared<hittable_list>();
    point3 dtop(0, 1.5, 0), dbot(0, -0.5, 0);
    point3 d1(0.7, 0.5, 0), d2(0, 0.5, 0.7), d3(-0.7, 0.5, 0), d4(0, 0.5, -0.7);

    diamond->add(make_shared<triangle>(dtop, d1, d2, blue_mat));
    diamond->add(make_shared<triangle>(dtop, d2, d3, blue_mat));
    diamond->add(make_shared<triangle>(dtop, d3, d4, blue_mat));
    diamond->add(make_shared<triangle>(dtop, d4, d1, blue_mat));
    diamond->add(make_shared<triangle>(dbot, d2, d1, blue_mat));
    diamond->add(make_shared<triangle>(dbot, d3, d2, blue_mat));
    diamond->add(make_shared<triangle>(dbot, d4, d3, blue_mat));
    diamond->add(make_shared<triangle>(dbot, d1, d4, blue_mat));

    // Arbitrary axis rotation (diagonal axis)
    shared_ptr<hittable> spinning_diamond = make_shared<rotate>(diamond, vec3(1, 1, 0), 45);
    spinning_diamond = make_shared<translate>(spinning_diamond, vec3(0, 1.5, -2.5));
    world.add(spinning_diamond);

    // Light source
    auto light = make_shared<diffuse_light>(colour(4, 4, 4));
    world.add(make_shared<quad>(point3(-2, 5, -2), vec3(4, 0, 0), vec3(0, 0, 4), light));

    world = hittable_list(make_shared<bvh_node>(world));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = image_width > 0 ? image_width : 800;
    cam.samples_per_pixel = samples_per_pixel > 0 ? samples_per_pixel : 200;
    cam.max_depth         = max_depth > 0 ? max_depth : 50;
    cam.background        = colour(0.1, 0.1, 0.15);

    cam.vfov     = 40;
    cam.lookfrom = point3(0, 4, 8);
    cam.lookat   = point3(0, 1, 0);
    cam.vup      = vec3(0, 1, 0);

    cam.defocus_angle = 0;

    if (thread_count > 0) {
        cam.render(world, thread_count);
    } else if (do_threading) {
        cam.render(world);
    } else {
        cam.render(world, 1);
    }
}

void cyberpunk() {
    hittable_list world;

    // rain-slicked street
    auto asphalt = make_shared<metal>(colour(0.05, 0.05, 0.08), 0.04);
    world.add(make_shared<quad>(point3(-1000, 0, -1000), vec3(2000, 0, 0), vec3(0, 0, 3000), asphalt));

    colour neon_palette[5] = {
        colour(1.00, 0.10, 0.60),  // magenta
        colour(0.10, 0.90, 1.00),  // cyan
        colour(0.55, 0.20, 1.00),  // violet
        colour(1.00, 0.45, 0.10),  // amber
        colour(0.15, 1.00, 0.50),  // green
    };

    // tower blocks lining both sides of the street, neon signage on the street-facing walls
    hittable_list city;
    auto tower = make_shared<lambertian>(colour(0.08, 0.09, 0.12));
    auto glass_facade = make_shared<metal>(colour(0.20, 0.25, 0.35), 0.15);
    for (int side = 0; side < 2; side++) {
        float dir = (side == 0) ? -1.0 : 1.0;
        for (int row = 0; row < 2; row++) {
            for (int j = 0; j < 14; j++) {
                float w = 90;
                float x_in = dir * (110 + row*110);
                float z0 = -550 + j*110;
                float h = random_float(120, 420) + row*random_float(0, 200);

                shared_ptr<material> facade = tower;
                if (random_float() < 0.35) facade = glass_facade;
                city.add(box(point3(x_in, 0, z0), point3(x_in + dir*w, h, z0 + w), facade));

                int signs = random_int(1, 3);
                for (int s = 0; s < signs; s++) {
                    auto neon = make_shared<diffuse_light>(3.5 * neon_palette[random_int(0, 4)]);
                    float sw = random_float(15, 55);
                    float sh = random_float(6, 14);
                    float sy = random_float(15, h - 25);
                    float sz = z0 + random_float(5, w - sw - 5);
                    city.add(make_shared<quad>(point3(x_in - dir*0.5, sy, sz), vec3(0, 0, sw), vec3(0, sh, 0), neon));
                }
            }
        }
    }
    world.add(make_shared<bvh_node>(city));

    // kerb strip lights
    auto strip_cyan = make_shared<diffuse_light>(colour(0.4, 3.0, 3.6));
    auto strip_pink = make_shared<diffuse_light>(colour(3.6, 0.4, 2.4));
    for (int j = 0; j < 14; j++) {
        float z0 = -550 + j*110;
        world.add(make_shared<quad>(point3(-105, 0.5, z0), vec3(4, 0, 0), vec3(0, 0, 90), strip_cyan));
        world.add(make_shared<quad>(point3(101, 0.5, z0), vec3(4, 0, 0), vec3(0, 0, 90), strip_pink));
    }

    // synthwave sun at the end of the street
    world.add(make_shared<sphere>(point3(0, 250, 2500), 450, make_shared<diffuse_light>(colour(2.5, 0.6, 1.2))));

    // motion-blurred hover traffic
    world.add(make_shared<sphere>(point3(-45, 130, -120), point3(-45, 130, 40), 6, make_shared<diffuse_light>(colour(7, 6.5, 5))));
    world.add(make_shared<sphere>(point3(50, 180, 350), point3(50, 180, 190), 6, make_shared<diffuse_light>(colour(7, 1.0, 1.4))));

    // chrome and glass street props
    world.add(make_shared<sphere>(point3(-60, 45, 130), 45, make_shared<metal>(colour(0.85, 0.9, 0.95), 0.02)));
    world.add(make_shared<sphere>(point3(60, 35, -40), 35, make_shared<dielectric>(1.5)));

    // plasma core: glowing gas in a glass shell
    auto core = make_shared<sphere>(point3(0, 55, 430), 55, make_shared<dielectric>(1.5));
    world.add(core);
    world.add(make_shared<constant_medium>(core, 0.15, colour(0.2, 0.9, 1.0)));

    // holographic particle cloud drifting over the street
    hittable_list holo;
    for (int j = 0; j < 1000; j++) {
        auto glow = make_shared<diffuse_light>(1.8 * neon_palette[random_int(0, 4)]);
        holo.add(make_shared<sphere>(point3::random(0, 120), 3, glow));
    }
    world.add(make_shared<translate>(
        make_shared<rotate_y>(make_shared<bvh_node>(holo), 25),
        vec3(-75, 230, 260)));

    // low-hanging smog
    auto smog = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
    world.add(make_shared<constant_medium>(smog, 0.0002, colour(0.55, 0.65, 0.8)));

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = image_width > 0 ? image_width : 2560;
    cam.samples_per_pixel = samples_per_pixel > 0 ? samples_per_pixel : 1000;
    cam.max_depth         = max_depth > 0 ? max_depth : 100;
    cam.background        = colour(0.01, 0.01, 0.025);

    cam.vfov     = 55;
    cam.lookfrom = point3(-35, 110, -680);
    cam.lookat   = point3(5, 170, 200);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    if (thread_count > 0) {
        cam.render(world, thread_count);
    } else if (do_threading) {
        cam.render(world);
    } else {
        cam.render(world, 1);
    }
}

int main(int argc, char** argv) {
    using clock = std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;

    auto parse_int = [](const char* s, int fallback) {
        if (!s) return fallback;
        char* end = nullptr;
        long v = std::strtol(s, &end, 10);
        if (end == s) return fallback;
        return static_cast<int>(v);
    };

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }

    if (argc > 1) scene             =  parse_int(argv[1], scene);
    if (argc > 2) thread_count      =  parse_int(argv[2], thread_count);
    if (argc > 3) do_threading      = (parse_int(argv[3], 1) != 0);
    if (argc > 4) image_width       =  parse_int(argv[4], image_width);
    if (argc > 5) samples_per_pixel =  parse_int(argv[5], samples_per_pixel);
    if (argc > 6) max_depth         =  parse_int(argv[6], max_depth);

    auto start1 = clock::now();
    switch (scene) {
        case 0:  health_check();                break;
        case 1:  lotso_spheres();               break;
        case 2:  checkered_spheres();           break;
        case 3:  earth();                       break;
        case 4:  perlin_spheres();              break;
        case 5:  quads();                       break;
        case 6:  simple_light();                break;
        case 7:  cornell_box();                 break;
        case 8:  cornell_smoke();               break;
        case 9:  big_scene();                   break;
        case 10: triangles_demo();              break;
        case 11: cyberpunk();                   break;
        default: health_check();                break;
    }
    auto end1 = clock::now();

    std::clog << "time: " << duration_cast<milliseconds>(end1 - start1).count() << " ms" << std::endl;
}
