#include "CppUnitTest.h"
#include "../RedFortress/Camera.h"
#include "../RedFortress/GamePad.h"
#include <tchar.h>

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#if defined(NDEBUG)
#pragma comment(lib, "d3dx9.lib")
#else
#pragma comment(lib, "d3dx9d.lib")
#endif

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest2
{
    TEST_CLASS(CameraTest)
    {
    public:
        
        TEST_METHOD(CameraTest_TestMethod1)
        {
            D3DXMATRIX viewMat = Camera::GetViewMatrix();
        }

        TEST_METHOD(CameraTest_TestMethod2)
        {
            D3DXMATRIX projMat = Camera::GetProjMatrix();
        }

        TEST_METHOD(CameraTest_TestMethod3)
        {
            D3DXVECTOR3 eyePos { 0.0f, 2.0f, -2.0f };
            D3DXVECTOR3 lookAtPos { 0.0f, 0.0f, 0.0f };
            float viewAngle { D3DX_PI / 4 };
            Camera::SetLookAtPos(lookAtPos);
        }

        TEST_METHOD(CameraTest_TestMethod4)
        {
            Camera::GetEyePos();
        }

        TEST_METHOD(CameraTest_TestMethod5)
        {
            Camera::GetRadian();
        }

        TEST_METHOD(CameraTest_TestMethod6)
        {
            Camera::SetRadian(1.f);
        }

        TEST_METHOD(CameraTest_TestMethod7)
        {
            LPDIRECTINPUT8 m_directInput = nullptr;
            auto hInstance = (HINSTANCE)GetModuleHandle(0);
            HRESULT ret = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
                (LPVOID*)&m_directInput, NULL);

            HWND hWnd = CreateWindow(_T("dummy"),
                                     _T("dummy"),
                                     WS_OVERLAPPEDWINDOW,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     NULL,
                                     NULL,
                                     hInstance,
                                     NULL);

            GamePad::Init(m_directInput, hWnd);
            Camera::Update();

            DestroyWindow(hWnd);

            // DestroyWindow���s������A���b�Z�[�W��������������̂�҂�
            MSG msg = { 0 };
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        TEST_METHOD(CameraTest_TestMethod8)
        {
            D3DXVECTOR3 world = {};
            Camera::GetScreenPos(world);
        }

        TEST_METHOD(CameraTest_TestMethod9)
        {
            eCameraMode mode = eCameraMode::BATTLE;
            Camera::SetCameraMode(mode);
        }

        TEST_METHOD(CameraTest_TestMethod10)
        {
            Camera::SetHouseMode(true);
            Camera::SetHouseMode(false);
        }
    };
}
