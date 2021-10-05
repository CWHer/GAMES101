//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture
{
private:
    cv::Mat image_data;

public:
    Texture(const std::string &name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getOriginalColor(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

    Eigen::Vector3f getColor(float u, float v)
    {
        auto toVec3f = [](const auto &v)
        {
            return Eigen::Vector3f(v[0], v[1], v[2]);
        };
        // I HATE AUTO
        auto lerp = [](float k, const auto &v0, const auto &v1)
        {
            return v0 + k * (v1 - v0);
        };
        // if u_img > a.5, then use (u_img, u_img + 1)
        //  else use (u_img - 1, u_img)
        int u_img = round(u * width);
        int v_img = round((1 - v) * height);
        // NOTE: bilinear would out of bounds
        if (u_img <= 0 || v_img <= 0 ||
            u_img >= width || v_img >= height)
            return getOriginalColor(u, v);

        // std::cout << "bilinear" << std::endl;

        float s = u * width - (u_img - 0.5);
        float t = (1 - v) * height - (v_img - 0.5);
        // std::cout << s << ',' << t << std::endl;
        assert(s >= 0 && s <= 1);
        assert(t >= 0 && t <= 1);

        // std::cout << width << ',' << height << std::endl;
        // std::cout << u_img << ',' << v_img << std::endl;
        auto u11 = image_data.at<cv::Vec3b>(v_img, u_img);
        auto u01 = image_data.at<cv::Vec3b>(v_img, u_img - 1);
        auto u10 = image_data.at<cv::Vec3b>(v_img - 1, u_img);
        auto u00 = image_data.at<cv::Vec3b>(v_img - 1, u_img - 1);
        // std::cout << "get u_ij" << std::endl;

        Eigen::Vector3f u0 = lerp(s, toVec3f(u00), toVec3f(u10));
        Eigen::Vector3f u1 = lerp(s, toVec3f(u01), toVec3f(u11));
        Eigen::Vector3f color = lerp(t, u0, u1);

        // std::cout << width << ',' << height << std::endl;
        // std::cout << u_img << ',' << v_img << std::endl;
        // std::cout << s << ',' << t << std::endl;
        // std::cout << u01 << ',' << u11 << std::endl;
        // std::cout << u00 << ',' << u10 << std::endl;
        // std::cout << u0 << ',' << u1 << std::endl;
        // std::cout << color << std::endl;
        // std::cout << getOriginalColor(u, v) << std::endl;
        // exit(0);

        return color;
    }
};
#endif //RASTERIZER_TEXTURE_H
