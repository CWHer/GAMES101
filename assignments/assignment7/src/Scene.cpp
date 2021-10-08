//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"

void Scene::buildBVH()
{
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum)
            {
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
    const Ray &ray,
    const std::vector<Object *> &objects,
    float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear)
        {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }

    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    // std::cout << "path tracing" << std::endl;
    IceCream ic;
    // ic.enable();
    ic.disable();

    const double eps = 1e-2;
    Intersection cur_inter = Scene::intersect(ray);
    if (!cur_inter.happened)
        return Vector3f(0, 0, 0);
    if (cur_inter.obj->hasEmit())
        return cur_inter.m->getEmission();
    Vector3f normal = cur_inter.normal.normalized();
    Material *m = cur_inter.m;
    // NOTE: I use "-" here
    Vector3f wo = -ray.direction.normalized();
    Vector3f hit_point = cur_inter.coords;
    // std::cout << "get current info" << std::endl;

    // ---------> path tracing
    Vector3f l_dir, l_indir;

    // ----> sample light
    Intersection light_inter;
    float pdf_light;
    sampleLight(light_inter, pdf_light);
    auto light_path = light_inter.coords - hit_point;
    auto ws = light_path.normalized();
    Intersection actual_inter = intersect(Ray(hit_point, ws));
    if (actual_inter.happened &&
        (light_inter.coords - actual_inter.coords).norm() < eps)
    {
        // light is not blocked
        IC(dotProduct(wo, normal));
        IC(dotProduct(ws, normal));
        l_dir = light_inter.emit *
                m->eval(wo, ws, normal) *
                dotProduct(ws, normal) *
                dotProduct(-ws, actual_inter.normal) /
                actual_inter.distance / pdf_light;
        // std::cout << l_dir << std::endl;
        // std::cout << "sample light" << std::endl;
        // std::cout << std::endl;
        // exit(0);
    }
    // <---- sample light

    // ----> sample indirect light
    // RussianRoulette
    if (get_random_float() < RussianRoulette)
    {
        Vector3f wi;
        wi = m->sample(wo, normal).normalized();
        auto indir_light_inter = intersect(Ray(hit_point, wi));
        IC(dotProduct(wo, normal));
        IC(dotProduct(wi, normal));
        if (indir_light_inter.happened &&
            !indir_light_inter.obj->hasEmit())
        {
            l_indir = castRay(Ray(hit_point, wi), depth + 1) *
                      m->eval(wo, wi, normal) *
                      dotProduct(wi, normal) /
                      m->pdf(wo, wi, normal) / RussianRoulette;
        }
        // std::cout << l_indir << std::endl;
        // std::cout << "sample indirect light" << std::endl;
        // std::cout << std::endl;
        // exit(0);
    }
    // <---- sample indirect light

    // <--------- path tracing
    // NOTE: this deals with NaN
    //  However, it results in some dark pixels....
    if (std::isnan(l_dir.x))
        l_dir = Vector3f(0, 0, 0);
    if (std::isnan(l_indir.x))
        l_indir = Vector3f(0, 0, 0);

    return l_dir + l_indir;
}