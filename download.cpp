#include <stdio.h>
#include <iostream>
#include "Windows.h"
#include "HCNetSDK.h"
using namespace std;
int saveRecordFile(int userId,char * srcfile,char * destfile)
{
    int bRes = 1;
    int hPlayback = 0;
    //按文件名下载录像
    if( (hPlayback = NET_DVR_GetFileByName(userId, srcfile, destfile)) < 0 )
    {
        printf( "GetFileByName failed. error[%d]\n", NET_DVR_GetLastError());
        bRes= -1;
        return bRes;
    }
//开始下载
    if(!NET_DVR_PlayBackControl_V40(hPlayback, NET_DVR_PLAYSTART, NULL,0,NULL,NULL))
    {
        printf("play back control failed [%d]\n",NET_DVR_GetLastError());
        bRes=-1;
        return bRes;
    }
    int nPos = 0;
    for(nPos = 0; nPos < 100&&nPos>=0; nPos = NET_DVR_GetDownloadPos(hPlayback))
    {
        printf("Be downloading...%d %%\n", nPos); //下载进度
        Sleep(5000); //millisecond
    }
    printf("have got %d\n", nPos);
    //停止下载
    if(!NET_DVR_StopGetFile(hPlayback))
    {
        printf("failed to stop get file [%d]\n",NET_DVR_GetLastError());
        bRes = -1;
        return bRes;
    }
    printf("%s\n",srcfile);
    if(nPos<0||nPos>100)
    {
        printf("download err [%d]\n",NET_DVR_GetLastError());
        bRes=-1;
        return bRes;
    }
    else
    {
        return 0;
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
    LONG lUserID;
    NET_DVR_DEVICEINFO_V30 struDeviceInfo;
    lUserID = NET_DVR_Login_V30("git.hq.gd", 6080, "admin", "ONVDTR", &struDeviceInfo);
    if (lUserID < 0)
    {
        printf("Login error, %d\n", NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return -1;
    }

    NET_DVR_FILECOND_V40 struFileCond={0};
    struFileCond.dwFileType = 0xFF;
    struFileCond.lChannel = 1; //通道号
    struFileCond.dwIsLocked = 0xFF;
    struFileCond.dwUseCardNo = 0;
    struFileCond.struStartTime.dwYear = 2019; //开始时间
    struFileCond.struStartTime.dwMonth = 3;
    struFileCond.struStartTime.dwDay = 1;
    struFileCond.struStartTime.dwHour = 10;
    struFileCond.struStartTime.dwMinute = 6;
    struFileCond.struStartTime.dwSecond =50;
    struFileCond.struStopTime.dwYear = 2022; //结束时间
    struFileCond.struStopTime.dwMonth = 3;
    struFileCond.struStopTime.dwDay = 1;
    struFileCond.struStopTime.dwHour = 11;
    struFileCond.struStopTime.dwMinute = 7;
    struFileCond.struStopTime.dwSecond = 0;
    //---------------------------------------
    //查找录像文件
    int lFindHandle = NET_DVR_FindFile_V40(lUserID, &struFileCond);
    if(lFindHandle < 0)
    {
        printf("find file fail,last error %d\n",NET_DVR_GetLastError());
        return -2;
    }
    NET_DVR_FINDDATA_V40 struFileData;
    while(true)
    {
        //逐个获取查找到的文件信息
        int result = NET_DVR_FindNextFile_V40(lFindHandle, &struFileData);
        if(result == NET_DVR_FILE_NOFIND || result == NET_DVR_ISFINDING)
        {
            continue;
        }
        else if(result == NET_DVR_FILE_SUCCESS) //获取文件信息成功
        {
            char strFileName[256] = {0};
            sprintf(strFileName, "./%s", struFileData.sFileName);
            saveRecordFile(lUserID, struFileData.sFileName, strFileName);
            break;
        }
        else if(result == NET_DVR_NOMOREFILE) //未查找到文件或者查找结束
        {
            break;
        }
        else
        {
            printf("find file fail for illegal get file state");
            break;
        }
    }
    //停止查找
    if(lFindHandle >= 0)
    {
        NET_DVR_FindClose_V30(lFindHandle);
    }

    //注销用户
    NET_DVR_Logout(lUserID);
    //释放 SDK 资源
    NET_DVR_Cleanup();
    return 0;
}
