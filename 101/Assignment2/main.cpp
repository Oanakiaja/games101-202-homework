// clang-format off
#include <iostream>
#include <opencv2/opencv.hpp>
#include "rasterizer.hpp"
#include "global.hpp"
#include "Triangle.hpp"

constexpr double MY_PI = 3.1415926;

double degreesToRadians(double degrees)
{
    return degrees * M_PI / 180.0f;
}

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1,0,0,-eye_pos[0],
                 0,1,0,-eye_pos[1],
                 0,0,1,-eye_pos[2],
                 0,0,0,1;

    view = translate*view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar)
{
  // Students will implement this function
    Eigen::Matrix4f ortho_projection = Eigen::Matrix4f::Identity();
    // 正交投影保持平行不变性，思考将长方形的锥体变换为一个 中心在 0 点的  -1 ,1 的 立方体
    // 相当于先平移到 0,0,0 ，然后放缩到 -1,1
    // 计算坐标
    float t, b, r, l, n, f;

    t = -zNear * std::tan(eye_fov / 2);
    b = -t;

    r = t * aspect_ratio;
    l = -r;

    // 计算 平移矩阵
    Eigen::Matrix4f ortho_trans{
        {1, 0, 0, -(r + l) / 2},
        {0, 1, 0, -(t + b) / 2},
        {0, 0, 1, -(zNear + zFar) / 2},
        {0, 0, 0, 1}};

    // 计算缩放矩阵
    Eigen::Matrix4f ortho_scale{
        {2 / (r - l), 0, 0, 0},
        {0, 2 / (t - b), 0, 0},
        {0, 0, 2 / (zNear - zFar), 0},
        {0, 0, 0, 1}};
    // translate multy scale
    // Eigen::Matrix4f ortho_projection{{{2 / (r - l), 0, 0, -(r + l) / (r - l)},
    //                                   {0, 2 / (t - b), 0, -(t + b) / (t - b)},
    //                                   {0, 0, -2 / (zf - zn), -(zFar + zNear) / (zf - zn)},
    //                                   {0, 0, 0, 1}}};

    // 透视投影有深度信息的缩放，所以将正交投影变成透视投影
    // 透视变换矩阵 , 相似三角形原理， 我们要把相机 zNear 和 zFar 中间的物体映射到 zNear上，所以有个相似三角形的变换
    Eigen::Matrix4f persp_to_ortho{
        {zNear, 0, 0, 0},
        {0, zNear, 0, 0},
        {0, 0, zNear + zFar, -zNear * zFar},
        {0, 0, 1, 0}};

    Eigen::Matrix4f project = Eigen::Matrix4f::Identity();

    return ortho_scale * ortho_trans * persp_to_ortho * project;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc == 2)
    {
        command_line = true;
        filename = std::string(argv[1]);
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0,0,5};


    std::vector<Eigen::Vector3f> pos
            {
                    {2, 0, -2},
                    {0, 2, -2},
                    {-2, 0, -2},
                    {3.5, -1, -5},
                    {2.5, 1.5, -5},
                    {-1, 0.5, -5}
            };

    std::vector<Eigen::Vector3i> ind
            {
                    {0, 1, 2},
                    {3, 4, 5}
            };

    std::vector<Eigen::Vector3f> cols
            {
                    {217.0, 238.0, 185.0},
                    {217.0, 238.0, 185.0},
                    {217.0, 238.0, 185.0},
                    {185.0, 217.0, 238.0},
                    {185.0, 217.0, 238.0},
                    {185.0, 217.0, 238.0}
            };

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);
    auto col_id = r.load_colors(cols);

    int key = 0;
    int frame_count = 0;

    if (command_line)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

        cv::imwrite(filename, image);

        return 0;
    }

    while(key != 27)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';
    }

    return 0;
}
// clang-format on