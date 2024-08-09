# Games101

I am a React developer, and I am learning computer graphics. This is a repository for the course Games101.

_It's a learning log_.

## setup

Install Dependencies

### cmake
a cpp builder
```bash
brew install cmake
```

### eigen
cpp vector and matrix library
https://eigen.tuxfamily.org/index.php?title=Main_Page




### opencv
to show images

Because I use homebrew to install opencv, the path is different from the default path, so I need to set the path in CMakeLists.txt. Be care for the version number.
```cmake
set(OpenCV_DIR /opt/homebrew/Cellar/opencv/4.10.0_2/lib/cmake/opencv4)
```

## Homework 1
  