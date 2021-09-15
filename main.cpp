#include <stdio.h>
#include <iostream>
#include <fstream>
#include "Windows.h"
#include "plaympeg4.h"
#include "char_conversion.h"
#include "HCNetSDK.h"
#include <time.h>
using namespace std;
typedef HWND (WINAPI *PROCGETCONSOLEWINDOW)();
PROCGETCONSOLEWINDOW GetConsoleWindowAPI;
LONG lPort = -1; //全局的播放库port号
std::string GBKToUTF8(const std::string& strGBK)
{
    std::string strOutUTF8 = "";
    WCHAR * str1;
    int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
    str1 = new WCHAR[n];
    MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);
    n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
    char * str2 = new char[n];
    WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
    strOutUTF8 = str2;
    delete[]str1;
    str1 = NULL;
    delete[]str2;
    str2 = NULL;
    return strOutUTF8;
}
int iNum=0;
BOOL CALLBACK MSesGCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser)
{
    int i=0;
    char filename[100];
    FILE *fSnapPic=NULL;
    FILE *fSnapPicPlate=NULL;

    //以下代码仅供参考，实际应用中不建议在该回调函数中直接处理数据保存文件，
    //例如可以使用消息的方式(PostMessage)在消息响应函数里进行处理。

    switch (lCommand)
    {
        case COMM_UPLOAD_PLATE_RESULT:
        {
            NET_DVR_PLATE_RESULT struPlateResult={0};
            memcpy(&struPlateResult, pAlarmInfo, sizeof(struPlateResult));
            printf("Plate Num: %s\n", struPlateResult.struPlateInfo.sLicense);//车牌号

            switch(struPlateResult.struPlateInfo.byColor)//车牌颜色
            {
                case VCA_BLUE_PLATE:
                    printf("Plate: Blue\n");
                    break;
                case VCA_YELLOW_PLATE:
                    printf("Plate: Yellow\n");
                    break;
                case VCA_WHITE_PLATE:
                    printf("Plate: White\n");
                    break;
                case VCA_BLACK_PLATE:
                    printf("Plate: Black\n");
                    break;
                default:
                    break;
            }

            //场景图
            if (struPlateResult.dwPicLen != 0 && struPlateResult.byResultType == 1 )
            {
                sprintf(filename,"testpic_%d.jpg",iNum);
                fSnapPic=fopen(filename,"wb");
                fwrite(struPlateResult.pBuffer1,struPlateResult.dwPicLen,1,fSnapPic);
                iNum++;
                fclose(fSnapPic);
            }
            //车牌图
            if (struPlateResult.dwPicPlateLen != 0 && struPlateResult.byResultType == 1)
            {
                sprintf(filename,"testPicPlate_%d.jpg",iNum);
                fSnapPicPlate=fopen(filename,"wb");
                fwrite(struPlateResult.pBuffer1,struPlateResult.dwPicLen,1,fSnapPicPlate);
                iNum++;
                fclose(fSnapPicPlate);
            }

            //其他信息处理......
            break;
        }
        case COMM_ITS_PLATE_RESULT:
        {
            NET_ITS_PLATE_RESULT struITSPlateResult={0};
            memcpy(&struITSPlateResult, pAlarmInfo, sizeof(struITSPlateResult));

            for (i=0;i < struITSPlateResult.dwPicNum;i++)
            {
                printf("Plate: %s\n", struITSPlateResult.struPlateInfo.sLicense);//车牌号
                // Create and open a text file
                ofstream MyFile("filename.txt",std::ofstream::out | std::ofstream::app);
                char resultStr[30];
                memset(resultStr,0,30*sizeof(char));
                gb2312ToUtf8(resultStr,30,struITSPlateResult.struPlateInfo.sLicense, strlen(struITSPlateResult.struPlateInfo.sLicense));
                // Write to the file
                MyFile << resultStr<<std::endl;

                // Close the file
                MyFile.close();
                switch(struITSPlateResult.struPlateInfo.byColor)//车牌颜色
                {
                    case VCA_BLUE_PLATE:
                        printf("Plate: Blue\n");
                        break;
                    case VCA_YELLOW_PLATE:
                        printf("Plate: Yellow\n");
                        break;
                    case VCA_WHITE_PLATE:
                        printf("Plate: White\n");
                        break;
                    case VCA_BLACK_PLATE:
                        printf("Plate: Black\n");
                        break;
                    default:
                        break;
                }

                //保存场景图
                if ((struITSPlateResult.struPicInfo[i].dwDataLen != 0)&&(struITSPlateResult.struPicInfo[i].byType== 1)||(struITSPlateResult.struPicInfo[i].byType == 2))
                {
                    sprintf(filename,"testITSpic%d_%d.jpg",iNum,i);
                    fSnapPic=fopen(filename,"wb");
                    fwrite(struITSPlateResult.struPicInfo[i].pBuffer, struITSPlateResult.struPicInfo[i].dwDataLen,1,fSnapPic);
                    iNum++;
                    fclose(fSnapPic);
                }
                //车牌小图片
                if ((struITSPlateResult.struPicInfo[i].dwDataLen != 0)&&(struITSPlateResult.struPicInfo[i].byType == 0))
                {
                    sprintf(filename,"testPicPlate%d_%d.jpg",iNum,i);
                    fSnapPicPlate=fopen(filename,"wb");
                    fwrite(struITSPlateResult.struPicInfo[i].pBuffer, struITSPlateResult.struPicInfo[i].dwDataLen, 1, fSnapPicPlate);
                    iNum++;
                    fclose(fSnapPicPlate);
                }
                //其他信息处理......
            }
            break;
        }
        default:
            break;
    }

    return TRUE;
}
void CALLBACK g_RealDataCallBack_V30(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer,DWORD dwBufSize,DWORD dwUser)
{
    if (dwBufSize > 0 && NET_DVR_STREAMDATA==dwDataType){
        std::ofstream myfile;
        myfile.open ("example.h264", std::ios_base::app);
        myfile.write((char *)pBuffer,dwBufSize);
    }
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
//设置异常消息回调函数
    NET_DVR_SetExceptionCallBack_V30(0, NULL,g_ExceptionCallBack, NULL);
    //---------------------------------------
// 获取控制台窗口句柄
    HMODULE hKernel32 = GetModuleHandle("kernel32");

    GetConsoleWindowAPI = (PROCGETCONSOLEWINDOW)GetProcAddress(hKernel32,"GetConsoleWindow");
    //---------------------------------------
// 注册设备
    LONG lUserID;
    //登录参数，包括设备地址、登录用户、密码等
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    struLoginInfo.bUseAsynLogin = 0; //同步登录方式
    strcpy(struLoginInfo.sDeviceAddress, "169.254.76.200"); //设备IP地址
    struLoginInfo.wPort = 8000; //设备服务端口
    strcpy(struLoginInfo.sUserName, "admin"); //设备登录用户名
    strcpy(struLoginInfo.sPassword, "ZR123456"); //设备登录密码 ONVDTR for 6080
//设备信息, 输出参数
    printf("Press to start\n");
    getchar();
    printf("Start\n");
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
    lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
    if (lUserID < 0)
    {
        printf("Login failed, error code: %d\n", NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return -1;
    }
    //---------------------------------------
//启动预览并设置回调数据流
    LONG lRealPlayHandle;
    HWND hWnd = GetConsoleWindowAPI(); //获取窗口句柄
    NET_DVR_PREVIEWINFO struPlayInfo = {0};
    struPlayInfo.hPlayWnd = NULL; //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
    struPlayInfo.lChannel = 1; //预览通道号
    struPlayInfo.dwStreamType = 0; //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
    struPlayInfo.dwLinkMode = 0; //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
    struPlayInfo.bBlocked = 1; //0- 非阻塞取流，1- 阻塞取流
    lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, NULL, NULL);
    if (lRealPlayHandle < 0)
    {
        printf("NET_DVR_RealPlay_V40 error:%lu\n",NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return -1;
    }
    //设置报警回调函数
    NET_DVR_SetDVRMessageCallBack_V31(MSesGCallback, NULL);

    //启用布防
    NET_DVR_SETUPALARM_PARAM struSetupParam={0};
    struSetupParam.dwSize=sizeof(NET_DVR_SETUPALARM_PARAM);
    struSetupParam.byLevel = 1; //布防优先级：0- 一等级（高），1- 二等级（中）
    struSetupParam.byAlarmInfoType = 1; //上传报警信息类型: 0- 老报警信息(NET_DVR_PLATE_RESULT), 1- 新报警信息(NET_ITS_PLATE_RESULT)

    LONG lHandle = NET_DVR_SetupAlarmChan_V41(lUserID,&struSetupParam);
    if (lHandle < 0)
    {
        printf("NET_DVR_SetupAlarmChan_V41 failed, error code: %d\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return -8;
    }
    printf("Set Alarm success\n");

    //---------------------------------------
    //网络触发抓拍

    NET_DVR_SNAPCFG struSnapCfg;
    memset(&struSnapCfg, 0, sizeof(NET_DVR_SNAPCFG));

    //结构体大小
    struSnapCfg.dwSize = sizeof(NET_DVR_SNAPCFG);

    //线圈抓拍次数，0-不抓拍，非0-连拍次数，目前最大5次
    struSnapCfg.bySnapTimes  = 5;

    //抓拍等待时间，单位ms，取值范围[0,60000]
    struSnapCfg.wSnapWaitTime   = 1000;

    //连拍间隔时间，单位ms，取值范围[67,60000]
    struSnapCfg.wIntervalTime[0]  = 2000;
    struSnapCfg.wIntervalTime[1]  = 2000;

    //触发IO关联的车道号，取值范围[0,9]
    struSnapCfg.byRelatedDriveWay = 0;

    //网络触发连拍
    if (!NET_DVR_ContinuousShoot(lUserID, &struSnapCfg))
    {
        printf("NET_DVR_ContinuousShoot failed, error code: %d\n", NET_DVR_GetLastError());
        return -9;
    }
    printf("Trigger shot\n");
    NET_DVR_SaveRealData_V30(lRealPlayHandle,STREAM_3GPP,"test.3gpp");
    //NET_DVR_SetRealDataCallBack(lRealPlayHandle,g_RealDataCallBack_V30,lUserID);
    Sleep(60000);
    //撤销布防上传通道
    if (!NET_DVR_CloseAlarmChan_V30(lHandle))
    {
        printf("NET_DVR_CloseAlarmChan_V30 failed, error code: %d\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return -5;
    }
    //---------------------------------------
    //关闭预览
    NET_DVR_StopRealPlay(lRealPlayHandle);
    //注销用户
    NET_DVR_Logout(lUserID);
    //释放SDK资源
    NET_DVR_Cleanup();
    return 0;
}