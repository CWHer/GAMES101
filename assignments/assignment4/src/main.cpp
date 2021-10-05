#include <chrono>
#include <limits>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

const int width = 700;
const int height = 800;

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata)
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4)
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
                  << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window)
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001)
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                     3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t)
{
    // TODO: Implement de Casteljau's algorithm
    if (control_points.size() == 1)
        return control_points.front();
    auto lerp = [t](const auto &v0, const auto &v1)
    {
        return t * v0 + (1 - t) * v1;
    };
    std::vector<cv::Point2f> new_ctrl_pts;
    new_ctrl_pts.resize(control_points.size() - 1);
    for (int i = 0; i < new_ctrl_pts.size(); ++i)
        new_ctrl_pts[i] = lerp(control_points[i], control_points[i + 1]);
    return recursive_bezier(new_ctrl_pts, t);
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window)
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's
    // recursive Bezier algorithm.

    std::vector<cv::Point2f> points;
    for (double t = 0.0; t <= 1.0; t += 0.001)
        points.emplace_back(recursive_bezier(control_points, t));

    // naive AA
    auto dist = [](const auto &a, const auto &b)
    {
        auto sqr = [](float x)
        { return x * x; };
        return sqrt(sqr(a.x - b.x) + sqr(a.y - b.y));
    };

    const float lim = 8;

    // O(pwh)
    // std::cout << "naive AA" << std::endl;
    // for (int x = 0; x < height; ++x)
    //     for (int y = 0; y < width; ++y)
    //     {
    //         float d = std::numeric_limits<float>::max();
    //         cv::Point2f loc(x + 0.5, y + 0.5);
    //         for (const auto &point : points)
    //             d = std::min(d, dist(point, loc));
    //         window.at<cv::Vec3b>(y, x)[1] = 255. * std::max(0.0f, (lim - d)) / lim;
    //     }
    // std::cout << "Done" << std::endl;

    // O(p)
    std::cout << "fast AA" << std::endl;
    auto resizeVec = [](auto &vec, int n)
    {
        for (auto &v : vec)
            v.resize(n);
    };
    std::vector<std::vector<float>> d(width);
    resizeVec(d, height);
    for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y)
            d[x][y] = std::numeric_limits<float>::max();

    const int dx = lim + 1;
    for (const auto &point : points)
    {
        // int cnt = 0;
        int x = round(point.x), y = round(point.y);
        for (int i = -dx; i <= dx; ++i)
            for (int j = -dx; j <= dx; ++j)
                if (y + i >= 0 && y + i <= width &&
                    x + j >= 0 && x + j <= height)
                {
                    // std::cout << "cnt: " << ++cnt << std::endl;
                    cv::Point2f loc(x + j + 0.5, y + i + 0.5);
                    d[y + i][x + j] = std::min(dist(point, loc), d[y + i][x + j]);
                }
    }
    std::cout << "load result" << std::endl;
    for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y)
            window.at<cv::Vec3b>(x, y)[1] = 255. * std::max(0.0f, (lim - d[x][y])) / lim;

    std::cout << "Done" << std::endl;
}
int main()
{
    cv::Mat window = cv::Mat(width, height, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27)
    {
        for (auto &point : control_points)
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4)
        {
            // naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

    return 0;
}
