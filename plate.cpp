#include <stdio.h>
#include <iostream>
#include "Windows.h"
#include "HCNetSDK.h"
using namespace std;

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
            printf("车牌号: %s\n", struPlateResult.struPlateInfo.sLicense);//车牌号

            switch(struPlateResult.struPlateInfo.byColor)//车牌颜色
            {
                case VCA_BLUE_PLATE:
                    printf("车辆颜色: 蓝色\n");
                    break;
                case VCA_YELLOW_PLATE:
                    printf("车辆颜色: 黄色\n");
                    break;
                case VCA_WHITE_PLATE:
                    printf("车辆颜色: 白色\n");
                    break;
                case VCA_BLACK_PLATE:
                    printf("车辆颜色: 黑色\n");
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
                printf("车牌号: %s\n", struITSPlateResult.struPlateInfo.sLicense);//车牌号

                switch(struITSPlateResult.struPlateInfo.byColor)//车牌颜色
                {
                    case VCA_BLUE_PLATE:
                        printf("车辆颜色: 蓝色\n");
                        break;
                    case VCA_YELLOW_PLATE:
                        printf("车辆颜色: 黄色\n");
                        break;
                    case VCA_WHITE_PLATE:
                        printf("车辆颜色: 白色\n");
                        break;
                    case VCA_BLACK_PLATE:
                        printf("车辆颜色: 黑色\n");
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

void main()
{
    //---------------------------------------
    //初始化
    NET_DVR_Init();

    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(10000, true);

    //---------------------------------------
    //注册设备

    //登录参数，包括设备地址、登录用户、密码等
    LONG lUserID = -1;
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    struLoginInfo.bUseAsynLogin = 0; //同步登录方式
    strcpy(struLoginInfo.sDeviceAddress, "192.0.0.64"); //设备IP地址
    struLoginInfo.wPort = 8000; //设备服务端口
    strcpy(struLoginInfo.sUserName, "admin"); //设备登录用户名
    strcpy(struLoginInfo.sPassword, "abcd1234"); //设备登录密码

    //设备信息, 输出参数
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};

    lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
    if (lUserID < 0)
    {
        printf("Login failed, error code: %d\n", NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return;
    }

    //---------------------------------------
    //报警布防

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
        return;
    }
    printf("布防成功!\n");

    //---------------------------------------
    //网络触发抓拍	

    NET_DVR_SNAPCFG struSnapCfg;
    memset(&struSnapCfg, 0, sizeof(NET_DVR_SNAPCFG));

    //结构体大小
    struSnapCfg.dwSize = sizeof(NET_DVR_SNAPCFG);

    //线圈抓拍次数，0-不抓拍，非0-连拍次数，目前最大5次 
    struSnapCfg.bySnapTimes  = 3;

    //抓拍等待时间，单位ms，取值范围[0,60000]
    struSnapCfg.wSnapWaitTime   = 1000;

    //连拍间隔时间，单位ms，取值范围[67,60000]
    struSnapCfg.wIntervalTime[0]  = 1000;
    struSnapCfg.wIntervalTime[1]  = 1000;

    //触发IO关联的车道号，取值范围[0,9]
    struSnapCfg.byRelatedDriveWay = 0;

    //网络触发连拍
    if (!NET_DVR_ContinuousShoot(lUserID, &struSnapCfg))
    {
        printf("NET_DVR_ContinuousShoot failed, error code: %d\n", NET_DVR_GetLastError());
        return;
    }
    printf("网络触发连拍!\n");

    Sleep(20000); //等待接收数据

    //---------------------------------------
    //退出

    //撤销布防上传通道
    if (!NET_DVR_CloseAlarmChan_V30(lHandle))
    {
        printf("NET_DVR_CloseAlarmChan_V30 failed, error code: %d\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return;
    }

    //注销用户
    NET_DVR_Logout(lUserID);

    //释放SDK资源
    NET_DVR_Cleanup();

    return;
}