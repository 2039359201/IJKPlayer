//
// Created by qing'guang on 2024/5/28.
//

#include "MNQueue.h"

MNQueue::MNQueue() {
    pthread_mutex_init(&mutexPacket,NULL);
    pthread_cond_init(&condPacket,NULL);
}

MNQueue::~MNQueue(){
    clearAvpacket();
}

int MNQueue::push(AVPacket *packet) {
    pthread_mutex_lock(&mutexPacket);
    queuePacket.push(packet);
    pthread_cond_signal(&condPacket);
    pthread_mutex_unlock(&mutexPacket);

    return 0;
}

int MNQueue::getAvPacket(AVPacket *packet) {
    pthread_mutex_lock(&mutexPacket);
    while (true){
        if(queuePacket.size()>0){
            AVPacket *avPacket = queuePacket.front();
            if (av_packet_ref(packet, avPacket) == 0) {
                queuePacket.pop();
                av_packet_free(&avPacket);
            }
            pthread_mutex_unlock(&mutexPacket);
            return 0;
        }else {
            pthread_cond_wait(&condPacket, &mutexPacket);
        }
    }
}

void MNQueue::clearAvpacket() {
    pthread_mutex_lock(&mutexPacket);
    while (!queuePacket.empty()) {
        AVPacket *packet = queuePacket.front();
        av_packet_free(&packet);
        queuePacket.pop();
    }
    pthread_mutex_unlock(&mutexPacket);
}

int MNQueue::size() {
    pthread_mutex_lock(&mutexPacket);
    int size = queuePacket.size();
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}
