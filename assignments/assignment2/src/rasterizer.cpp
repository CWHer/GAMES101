//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>

rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f &v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

static bool insideTriangle(double x, double y,
                           const Eigen::Vector3f *_v)
{
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]

    // NOTE: _v stores 3 points of triangle
    // NOTE: _v.size() == 3
    Eigen::Vector2f point(x, y);
    std::vector<int> ret;
    for (int i = 0; i < 3; ++i)
    {
        Eigen::Vector2f p = point - _v[i].head(2);
        Eigen::Vector2f e = (_v[(i + 1) % 3] - _v[i]).head(2);
        ret.emplace_back(e[0] * p[1] - e[1] * p[0]);
    }
    return ret[0] > 0 == ret[1] > 0 &&
           ret[1] > 0 == ret[2] > 0;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f *v)
{
    float c1 = (x * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * y + v[1].x() * v[2].y() - v[2].x() * v[1].y()) / (v[0].x() * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * v[0].y() + v[1].x() * v[2].y() - v[2].x() * v[1].y());
    float c2 = (x * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * y + v[2].x() * v[0].y() - v[0].x() * v[2].y()) / (v[1].x() * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * v[1].y() + v[2].x() * v[0].y() - v[0].x() * v[2].y());
    float c3 = (x * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * y + v[0].x() * v[1].y() - v[1].x() * v[0].y()) / (v[2].x() * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * v[2].y() + v[0].x() * v[1].y() - v[1].x() * v[0].y());
    return {c1, c2, c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto &buf = pos_buf[pos_buffer.pos_id];
    auto &ind = ind_buf[ind_buffer.ind_id];
    auto &col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto &i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
            mvp * to_vec4(buf[i[0]], 1.0f),
            mvp * to_vec4(buf[i[1]], 1.0f),
            mvp * to_vec4(buf[i[2]], 1.0f)};
        //Homogeneous division
        for (auto &vec : v)
        {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto &vert : v)
        {
            vert.x() = 0.5 * width * (vert.x() + 1.0);
            vert.y() = 0.5 * height * (vert.y() + 1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }

    std::cout << "load sample buf" << std::endl;
    load_sample_buf();
}

void rst::rasterizer::load_sample_buf()
{
    for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y)
        {
            auto idx = get_index(x, y);
            Eigen::Vector3f frame = {0, 0, 0};
            for (const auto &sample_frame : sample_frame_buf[idx])
                frame += sample_frame;
            frame_buf[idx] = frame / (MSAA_N * MSAA_N);
        }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle &t)
{
    auto v = t.toVector4();
    // std::cout << t.v[1] << std::endl;
    // 350, 108.579, 0.714285
    int min_x = std::floor(std::min(v[0][0], std::min(v[1][0], v[2][0])));
    int max_x = std::ceil(std::max(v[0][0], std::max(v[1][0], v[2][0])));
    int min_y = std::floor(std::min(v[0][1], std::min(v[1][1], v[2][1])));
    int max_y = std::ceil(std::max(v[0][1], std::max(v[1][1], v[2][1])));

    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle

    for (int x = min_x; x < max_x; ++x)
        for (int y = min_y; y < max_y; ++y)
            for (int i = 0; i < offsets.size(); ++i)
            {
                const auto offset = offsets[i];
                if (insideTriangle(x + offset.first,
                                   y + offset.second,
                                   t.v))
                {
                    // If so, use the following code to get the interpolated z value.
                    float alpha, beta, gamma;
                    std::tie(alpha, beta, gamma) = computeBarycentric2D(
                        x + offset.first, y + offset.second, t.v);
                    float w_reciprocal = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                    z_interpolated *= w_reciprocal;
                    // std::cout << "z_interpolated: " << z_interpolated << std::endl;

                    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
                    // std::vector<Eigen::Vector3f> frame_buf;
                    // std::vector<float> depth_buf;

                    if (z_interpolated < sample_dep_buf[get_index(x, y)][i])
                    {
                        // std::cout << "draw" << std::endl;
                        sample_dep_buf[get_index(x, y)][i] = z_interpolated;
                        sample_frame_buf[get_index(x, y)][i] = t.getColor();
                        // frame_buf[get_index(x, y)] = t.getColor();
                    }
                }
            }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f &m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f &v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f &p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
        for (auto &frame_buf : sample_frame_buf)
            std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        for (auto &dep_buf : sample_dep_buf)
            std::fill(dep_buf.begin(), dep_buf.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    sample_dep_buf.resize(w * h);
    for (auto &buf : sample_dep_buf)
        buf.resize(MSAA_N * MSAA_N);
    sample_frame_buf.resize(w * h);
    for (auto &buf : sample_frame_buf)
        buf.resize(MSAA_N * MSAA_N);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height - 1 - y) * width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f &point, const Eigen::Vector3f &color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height - 1 - point.y()) * width + point.x();
    frame_buf[ind] = color;
}

// clang-format on