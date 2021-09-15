//
// Created by exede on 13/9/2021.
//
#ifndef TEST_SPEAKER_LIB_H
#define TEST_SPEAKER_LIB_H
LONG lRealPlayHandle;
LONG lUserID;
extern "C" _declspec(dllexport) int start_stream();
extern "C" _declspec(dllexport) int stop_stream();
#endif //TEST_SPEAKER_LIB_H
