//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_INTERSECTION_H
#define RAYTRACING_INTERSECTION_H
#include "Vector.hpp"
#include "Material.hpp"
class Object;
class Sphere;

struct Intersection
{
    static bool SHOW_INTERSECTION;

    Intersection()
    {
        happened = false;
        coords = Vector3f();
        normal = Vector3f();
        distance = std::numeric_limits<double>::max();
        obj = nullptr;
        m = nullptr;
    }

    friend std::ostream &operator<<(std::ostream &out,
                                    const Intersection &inter)
    {
        std::cout << "happened: " << inter.happened << '\n';
        std::cout << "coords: " << inter.coords << '\n';
        std::cout << "normal: " << inter.normal << '\n';
        std::cout << "emit: " << inter.emit << '\n';
        std::cout << "distance: " << inter.distance << '\n';
        std::cout << "object ptr: " << (void *)inter.obj << '\n';
        std::cout << "material ptr: " << (void *)inter.m;
        return out;
    }
    bool happened;
    Vector3f coords;
    Vector3f tcoords;
    Vector3f normal;
    Vector3f emit;
    double distance;
    Object *obj;
    Material *m;
};

// NOTE: the idea comes from ic()
#define IC(expr) ic(#expr, expr)

struct IceCream
{
    bool ICECREAM;
    IceCream() : ICECREAM(false) {}

    void enable() { ICECREAM = true; }
    void disable() { ICECREAM = false; }
    bool enabled() { return ICECREAM; }

    template <class T>
    T operator()(char *name, T t)
    {
        if (ICECREAM)
            std::cout << name << ": "
                      << t << std::endl;
        return t;
    }
};

#endif //RAYTRACING_INTERSECTION_H
