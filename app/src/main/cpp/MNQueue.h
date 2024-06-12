//
// Created by qing'guang on 2024/5/28.
//

#ifndef IJKPLAYER_MNQUEUE_H
#define IJKPLAYER_MNQUEUE_H

#include "queue"
#include "pthread.h"

extern "C"{
#include "../../../../../../IJKPlayer/app/src/main/cpp/include/libavcodec/avcodec.h"
};

class MNQueue {
public:
    std::queue<AVPacket *> queuePacket;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;
    bool playStatus = false;

public:
    MNQueue();
    ~MNQueue();
    int push(AVPacket *packet);
    int getAvPacket(AVPacket *packet);
    int size();
    void clearAvpacket();

};


#endif //IJKPLAYER_MNQUEUE_H
