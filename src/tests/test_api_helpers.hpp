#ifndef __test_api_helpers_hpp__
#define __test_api_helpers_hpp__

#include <DirectXMath.h>
using namespace DirectX;

using Mat4 = Eigen::Matrix4f;
using Vec4 = Eigen::Vector4f;
using Vec3 = Eigen::Vector3f;

struct alignas(256) CameraCB
{
	Mat4 view;
	Mat4 projection;
};

struct alignas(256) CameraCBRT
{
	Vec3 camera_pos;
	float pad0;
	Vec3 camera_forward;
	float pad1;
	Vec3 camera_right;
	float pad2;
	Vec3 camera_up;
	float tanHalfFov;
	float aspect;
	float pad4[2];
};

struct alignas(256) ObjectCB
{
	Mat4 world = Mat4::Identity();
};

inline Eigen::Matrix4f XMMatrixToEigen(const XMMATRIX& xm)
{
	XMFLOAT4X4 temp;
	XMStoreFloat4x4(&temp, xm);

	Eigen::Matrix4f e;

	e << temp._11, temp._12, temp._13, temp._14,
		temp._21, temp._22, temp._23, temp._24,
		temp._31, temp._32, temp._33, temp._34,
		temp._41, temp._42, temp._43, temp._44;

	return e;
}

inline void get_transforms(Mat4& w, Mat4& v, Mat4& p)
{

	XMMATRIX world = XMMatrixIdentity();
	//	XMMatrixRotationY(XMConvertToRadians(45.0f));

	XMVECTOR eye =
		XMVectorSet(0.0f, 0.0f, -3.0f, 1.0f);

	XMVECTOR target =
		XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	XMVECTOR up =
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view =
		XMMatrixLookAtLH(eye, target, up);

	XMMATRIX projection =
		XMMatrixPerspectiveFovLH(
			XM_PIDIV4,
			800.0f / 600.0f,
			0.1f,
			100.0f);

	w = XMMatrixToEigen(world);
	v = XMMatrixToEigen(view);
	p = XMMatrixToEigen(projection);
}

inline float get_delta_time()
{
	using clock = std::chrono::high_resolution_clock;

	static auto previous_time = clock::now();

	auto current_time = clock::now();

	std::chrono::duration<float> delta =
		current_time - previous_time;

	previous_time = current_time;

	return delta.count();
}

inline Mat4 rotate_triangle(float dt) {

	static float angle = 0.0f;
	static float speed =
		XMConvertToRadians(85.0f);

	auto t =
		Eigen::AngleAxisf(
			dt * speed,
			Vec3(0, 1, 0));

	Mat4 rotation =
		Mat4::Identity();

	rotation.block<3, 3>(0, 0) =
		Eigen::AngleAxisf(
			angle,
			Vec3(0, 1, 0))
		.toRotationMatrix();

	angle += dt * speed;
	return rotation;
}

#endif