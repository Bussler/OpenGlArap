#ifndef EIGEN_CONTAINERS_INCLUDED
#define EIGEN_CONTAINERS_INCLUDED

// Eigen types don't play well with standard containers and need special treatment
// See https://eigen.tuxfamily.org/dox/group__TopicStlContainers.html

#include <Eigen/Core>
#include <Eigen/StdVector>

typedef std::vector<Eigen::Vector2f, Eigen::aligned_allocator<Eigen::Vector2f>> vector_Vector2f;
typedef std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f>> vector_Vector3f;
typedef std::vector<Eigen::Vector4f, Eigen::aligned_allocator<Eigen::Vector4f>> vector_Vector4f;

typedef std::vector<Eigen::Matrix3f, Eigen::aligned_allocator<Eigen::Matrix3f>> vector_Matrix3f;
typedef std::vector<Eigen::Matrix4f, Eigen::aligned_allocator<Eigen::Matrix4f>> vector_Matrix4f;

#endif
