#include "types.hpp"
#include "debug/log.hpp"
#include "math/geometry_func.hpp"
#include "math/math_func.hpp"

#include <math.h>
#include <array>
#include <cmath>
#include <numeric>
#include <vector>
namespace emp {
AABB AABB::CreateFromCircle(const Circle &c)
{
    return AABB::CreateMinMax(c.pos - vec2f(c.radius, c.radius), c.pos + vec2f(c.radius, c.radius));
}
AABB AABB::CreateFromPolygon(const ConvexPolygon &p)
{
    return AABB::CreateFromVerticies(p.getVertecies());
}
AABB AABB::TransformedAABB(const TransformMatrix &transform, const AABB &model)
{
    std::array<vec2f, 4U> verts = { model.bl(), model.tl(), model.tr(), model.br() };
    for(auto &v : verts) {
        v = transformPoint(transform, v);
    }
    return AABB::CreateFromVerticies(verts);
}
AABB AABB::CreateMinMax(vec2f min, vec2f max)
{
    AABB a;
    a.min = min;
    a.max = max;
    return a;
}
AABB AABB::Expandable()
{
    static const vec2f extreme = { 0xffffff, 0xffffff };
    return AABB::CreateMinMax(extreme, -extreme);
}
AABB AABB::CreateCenterSize(vec2f c, vec2f s)
{
    auto half = s / 2.f;
    return {
        { c.x - half.x, c.y - half.y },
        { c.x + half.x, c.y + half.y }
    };
}
AABB AABB::CreateMinSize(vec2f min, vec2f size)
{
    AABB a;
    a.min = min;
    a.max = a.min + size;
    return a;
}
Ray Ray::CreatePoints(vec2f a, vec2f b)
{
    Ray r;
    r.pos = a;
    r.dir = b - a;
    return r;
}
Ray Ray::CreatePositionDirection(vec2f p, vec2f d)
{
    Ray r;
    r.pos = p;
    r.dir = d;
    return r;
}
ConvexPolygon ConvexPolygon::CreateRegular(vec2f pos, float rot, size_t count, float dist)
{
    std::vector<vec2f> model;
    for(size_t i = 0; i < count; i++) {
        model.push_back(vec2f(sinf(3.141f * 2.f * ((float)i / (float)count)), cosf(3.141f * 2.f * ((float)i / (float)count))) *
                        dist);
    }
    return ConvexPolygon(pos, rot, model);
}
ConvexPolygon ConvexPolygon::CreateFromPoints(std::vector<vec2f> verticies)
{
    vec2f avg = std::reduce(verticies.begin(), verticies.end()) / (float)verticies.size();
    for(auto &v : verticies) {
        v -= avg;
    }
    return ConvexPolygon(avg, 0.f, verticies);
}
ConvexPolygon ConvexPolygon::CreateFromAABB(const AABB &aabb)
{
    std::vector<vec2f> points = { aabb.min, vec2f(aabb.min.x, aabb.max.y), aabb.max, vec2f(aabb.max.x, aabb.min.y) };
    return ConvexPolygon::CreateFromPoints(points);
}

ConcavePolygon::ConcavePolygon(std::vector<ConvexPolygon> polygons)
{
    vec2f avg_pos;
    float area_sum = 0.f;
    for(const auto &p : polygons) {
        auto a = area(p.getModelVertecies());
        area_sum += a;
        avg_pos += p.getPos() * a;
    }
    avg_pos = avg_pos / area_sum;
    for(auto &p : polygons) {
        p.setPos(p.getPos() - avg_pos);
    }
    pos = avg_pos;
    m_model = polygons;
    m_polygons = m_model;
    m_updatePolys();
}
vec2f operator*(vec2f a, vec2f b)
{
    return vec2f(a.x * b.x, a.y * b.y);
}

}  //  namespace emp
