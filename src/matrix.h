#pragma once

#if LINUX
#include <math.h>
#endif

#include "vector.h"

namespace math
{
#pragma warning(disable: 4201)
    template<typename T>
    class matrix
    {
    public:
        union {
            struct {
                T _11, _12, _13, _14;
                T _21, _22, _23, _24;
                T _31, _32, _33, _34;
                T _41, _42, _43, _44;
            };
            T m[4][4];
        };

    public:
        matrix()
        {
            Identity();
        }

        matrix(T m11, T m12, T m13, T m14,
            T m21, T m22, T m23, T m24,
            T m31, T m32, T m33, T m34,
            T m41, T m42, T m43, T m44)
        {
            _11 = m11; _12 = m12; _13 = m13; _14 = m14;
            _21 = m21; _22 = m22; _23 = m23; _24 = m24;
            _31 = m31; _32 = m32; _33 = m33; _34 = m34;
            _41 = m41; _42 = m42; _43 = m43; _44 = m44;
        }

        matrix operator* (const matrix& other)
        {
            matrix result;

            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    result.m[i][j] = m[i][0] * other.m[0][j] + m[i][1] * other.m[1][j] + m[i][2] * other.m[2][j] + m[i][3] * other.m[3][j];
                }
            }

            return result;
        }

        void Identity()
        {
            _11 = 1.0f;	_12 = 0.0f;	_13 = 0.0f; _14 = 0.0f;
            _21 = 0.0f;	_22 = 1.0f;	_23 = 0.0f; _14 = 0.0f;
            _31 = 0.0f;	_32 = 0.0f;	_33 = 1.0f; _14 = 0.0f;
            _41 = 0.0f;	_42 = 0.0f;	_43 = 0.0f; _14 = 1.0f;
        }

        math::Vector3f Right() { return math::Vector3f(_11, _12, _13); }
        math::Vector3f Forward() { return math::Vector3f(_21, _22, _23); }
        math::Vector3f Up() { return math::Vector3f(_31, _32, _33); }
        math::Vector3f Position() { return math::Vector3f(_41, _42, _43); }

        void SetRight(const math::Vector3f &vec)
        {
            _11 = vec.x();
            _12 = vec.y();
            _13 = vec.z();
            _14 = 0.0f;
        }

        void SetForward(const math::Vector3f &vec)
        {
            _21 = vec.x();
            _22 = vec.y();
            _23 = vec.z();
            _24 = 0.0f;
        }

        void SetUp(const math::Vector3f &vec)
        {
            _31 = vec.x();
            _32 = vec.y();
            _33 = vec.z();
            _34 = 0.0f;
        }

        void SetPosition(const math::Vector3f &vec)
        {
            _41 = vec.x();
            _42 = vec.y();
            _43 = vec.z();
            _44 = 1.0f;
        }

        void SetRotationEuler(math::Vector3f rotation)
        {
            auto euler = math::RotationToRadian(rotation);

            matrix x(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, std::cos(euler.x()), -std::sin(euler.x()), 0.0f,
                0.0f, std::sin(euler.x()), std::cos(euler.x()), 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);

            matrix y(
                std::cos(euler.y()), 0.0f, std::sin(euler.y()), 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                -std::sin(euler.y()), 0.0f, std::cos(euler.y()), 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);

            matrix z(
                std::cos(euler.z()), -std::sin(euler.z()), 0.0f, 0.0f,
                std::sin(euler.z()), std::cos(euler.z()), 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);

            matrix rotMatrix = (x * y * z);
            rotMatrix.SetPosition(Position());

            *this = rotMatrix;
        }

        math::Vector3f RotationEuler()
        {
            float T1 = std::atan2<T>(m[2][1], m[2][2]);
            float C2 = std::sqrt(m[0][0] * m[0][0] + m[1][0] * m[1][0]);
            float T2 = std::atan2<T>(-m[2][0], C2);
            float S1 = std::sin(T1);
            float C1 = std::cos(T1);
            float T3 = std::atan2<T>(S1*m[0][2] - C1*m[0][1], C1*m[1][1] - S1*m[1][2]);
            return math::RadianToRotation({ -T1, -T2, -T3 });
        }

        const static matrix identity;
    };



    template<typename T>
    const matrix<T> matrix<T>::identity;

    using Matrix = matrix<float>;
};
