#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr float MY_PI = 3.1415926;

double degreesToRadians(double degrees)
{
    return degrees * M_PI / 180.0f;
}

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate{{{1, 0, 0, -eye_pos[0]},
                               {0, 1, 0, -eye_pos[1]},
                               {
                                   0,
                                   0,
                                   1,
                                   -eye_pos[2],
                               },
                               {0, 0, 0, 1}}};

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_rotation(Eigen::Vector3f axis, float angle)
{
    float rotation_rad = degreesToRadians(angle);
    Eigen::Matrix3f N{
        {0, -axis[2], axis[1]},
        {axis[2], 0, -axis[0]},
        {-axis[1], axis[0], 0},
    };
    Eigen::Matrix3f rotation3f;
    Eigen::Matrix3f I = Eigen::Matrix3f::Identity();
    rotation3f = I * std::cos(rotation_rad) + (1 - std::cos(rotation_rad)) * axis * axis.transpose() + std::sin(rotation_rad) * N;
    Eigen::Matrix4f rotation4f{
        {rotation3f(0, 0), rotation3f(0, 1), rotation3f(0, 2), 0},
        {rotation3f(1, 0), rotation3f(1, 1), rotation3f(1, 2), 0},
        {rotation3f(2, 0), rotation3f(2, 1), rotation3f(2, 2), 0},
        {0, 0, 0, 1}};

    return rotation4f;
};

Eigen::Matrix4f get_model_matrix(Eigen::Vector3f axis, float rotation_angle)
{
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    // basic rotation z axis matrix
    //     float cos_a = std::cos(degreesToRadians(rotation_angle));
    // float sin_a = std::sin(degreesToRadians(rotation_angle));
    // Eigen::Matrix4f rotate{{cos_a, -sin_a, 0, 0},
    //                        {sin_a, cos_a, 0, 0},
    //                        {0, 0, 1, 0},
    //                        {0, 0, 0, 1}};

    Eigen::Matrix4f rotate = get_rotation(axis, rotation_angle);

    return rotate;
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

int main(int argc, const char **argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3)
    {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4)
        {
            filename = std::string(argv[3]);
        }
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};
    Eigen::Vector3f rotate_axis = {0, 1, 1};

    std::vector<Eigen::Vector3f>
        pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(rotate_axis, angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(rotate_axis, angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, -0.1, -50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a')
        {
            std::cout << "a pressed" << angle << '\n';
            angle += 10;
        }
        else if (key == 'd')
        {
            std::cout << "d pressed" << angle << '\n';
            angle -= 10;
        }
    }

    return 0;
}
