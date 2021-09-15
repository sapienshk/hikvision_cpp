#include <stdio.h>
#include <iostream>
#include "Windows.h"
#include "HCNetSDK.h"
using namespace std;
typedef HWND (WINAPI *PROCGETCONSOLEWINDOW)();
PROCGETCONSOLEWINDOW GetConsoleWindowAPI;
void CALLBACK fVoiceDataCallBack(LONG lVoiceComHandle, char *pRecvDataBuffer, DWORD dwBufSize, BYTE byAudioFlag, void*
pUser)
{
    //printf("receive voice data, %d\n", dwBufSize);
}
void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
    char tempbuf[256] = {0};
    switch(dwType)
    {
        case EXCEPTION_RECONNECT: //预览时重连
            printf("----------reconnect--------%d\n", time(NULL));
            break;
        default:
            break;
    }
}
int main() {
    //---------------------------------------
    // 初始化
    NET_DVR_Init();
    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(10000, true);
    //---------------------------------------
    // 注册设备
    //设置异常消息回调函数
    NET_DVR_SetExceptionCallBack_V30(0, NULL,g_ExceptionCallBack, NULL);
    //---------------------------------------
// 获取控制台窗口句柄
    HMODULE hKernel32 = GetModuleHandle("kernel32");
    GetConsoleWindowAPI = (PROCGETCONSOLEWINDOW)GetProcAddress(hKernel32,"GetConsoleWindow");
    //login
    LONG lUserID;
    NET_DVR_DEVICEINFO_V30 struDeviceInfo;
    lUserID = NET_DVR_Login_V30("git.hq.gd", 6080, "admin", "ONVDTR", &struDeviceInfo);
    //lUserID = NET_DVR_Login_V30("git.hq.gd", 6081, "admin", "123456", &struDeviceInfo);
    if (lUserID < 0)
    {
        printf("Login error, %d\n", NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return -3;
    }

    //语音对讲
    LONG lVoiceHanle;
    lVoiceHanle = NET_DVR_StartVoiceCom_V30(lUserID, 1,0, fVoiceDataCallBack, NULL);
    if (lVoiceHanle < 0)
    {
        printf("NET_DVR_StartVoiceCom_V30 error, %d!\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return -2;
    }
    LONG lRealPlayHandle;
    HWND hWnd = GetConsoleWindowAPI(); //获取窗口句柄
    NET_DVR_PREVIEWINFO struPlayInfo = {0};
    struPlayInfo.hPlayWnd = hWnd; //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
    struPlayInfo.lChannel = 1; //预览通道号
    struPlayInfo.dwStreamType = 0; //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
    struPlayInfo.dwLinkMode = 0; //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
    struPlayInfo.bBlocked = 1; //0- 非阻塞取流，1- 阻塞取流
    lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, NULL, NULL);

    //Sleep(10000); //millisecond
    bool isTurning = false;
    while(true){
        auto left = GetKeyState(VK_LEFT);
        auto right = GetKeyState(VK_RIGHT);
        auto up = GetKeyState(VK_UP);
        auto down = GetKeyState(VK_DOWN);
        auto exit = GetKeyState(VK_SPACE);
        if(exit==1){
            break;
        }
        if(!(left & 0x80) && isTurning){
            NET_DVR_PTZControl(lRealPlayHandle,PAN_LEFT,1);
            isTurning = false;
        }else if(left & 0x80 && !isTurning){
            NET_DVR_PTZControl(lRealPlayHandle,PAN_LEFT,0);
            isTurning = true;
        }
        if(!(right & 0x80) && isTurning){
            NET_DVR_PTZControl(lRealPlayHandle,PAN_RIGHT,1);
            isTurning = false;
        }else if(right & 0x80 && !isTurning){
            NET_DVR_PTZControl(lRealPlayHandle,PAN_RIGHT,0);
            isTurning = true;
        }
        if(!(up & 0x80) && isTurning){
            NET_DVR_PTZControl(lRealPlayHandle,TILT_UP,1);
            isTurning = false;
        }else if(up & 0x80 && !isTurning){
            NET_DVR_PTZControl(lRealPlayHandle,TILT_UP,0);
            isTurning = true;
        }
        if(!(down & 0x80) && isTurning){
            NET_DVR_PTZControl(lRealPlayHandle,TILT_DOWN,1);
            isTurning = false;
        }else if(down & 0x80 && !isTurning){
            NET_DVR_PTZControl(lRealPlayHandle,TILT_DOWN,0);
            isTurning = true;
        }
        Sleep(100);
    }
    getchar();
    //关闭语音对讲
    if (!NET_DVR_StopVoiceCom(lVoiceHanle))
    {
        printf("NET_DVR_StopVoiceCom error, %d!\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return -1;
    }
//关闭预览
    NET_DVR_StopRealPlay(lRealPlayHandle);
    //注销用户
    NET_DVR_Logout(lUserID);
    //释放 SDK 资源
    NET_DVR_Cleanup();
    return 0;
}
