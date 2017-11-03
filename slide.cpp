#include "sysinclude.h"
#include <iostream>
using namespace std;

extern void SendFRAMEPacket(unsigned char* pData, unsigned int len);

#define WINDOW_SIZE_STOP_WAIT 1
#define WINDOW_SIZE_BACK_N_FRAME 4

typedef enum{data, ack, nak}frame_kind;

typedef struct frame_head{
	frame_kind kind;
	unsigned int seq;
	unsigned int ack;
	unsigned char data[100];
};

typedef struct frame{
	frame_head head;
	unsigned int size;
};

frame frame_buffer[50];
unsigned int tail_f = 0, head_f = 0, upper_bound = 0;

int stud_slide_window_stop_and_wait(char* pBuffer, int bufferSize, UINT8 messageType){
	switch(messageType){
		case MSG_TYPE_TIMEOUT:{
			for(int i = tail_f; i < upper_bound; i++){
				int pos = i % 50;
				SendFRAMEPacket((unsigned char*)(&frame_buffer[pos]), frame_buffer[pos].size);;
			}
			return 0;
		}
		case MSG_TYPE_SEND:{
			int head_pos = head_f % 50;
			frame_buffer[head_pos].head = (frame*)pBuffer;
			frame_buffer[head_pos].size = bufferSize;
			head_f++;
			if(head_f - tail_f <= WINDOW_SIZE_STOP_WAIT){
				SendFRAMEPacket((unsigned char*)(&frame_buffer[head_pos]), (unsigned int)bufferSize);
				upper_bound++;
			}
			return 0;
		}
		case MSG_TYPE_RECEIVE:{
			frame* rec_frame = (frame*)pBuffer;
			unsigned int rec_ack = ntohl(rec_frame->head.ack);
			for(int i = tail_f; i < upper_bound; i++){
				int cur_pos = i % 50;
				unsigned int cur_ack = ntohl(frame_buffer[cur_pos].head.seq);
				if(cur_ack <= rec_ack){
					tail_f++;
					if(upper_bound < head_f){
						int upper_pos = upper_bound % 50;
						SendFRAMEPacket((unsigned char*)(&frame_buffer[upper_pos]), frame_buffer[upper_pos].size);
						upper_bound++;
					}
					else break;
				}
			}
			return 0;
		}
		default: break;
	}
	return -1;
}

int stud_slide_window_back_n_frame(char* pBuffer, int bufferSize, UINT8 messageType){
	switch(messageType){
		case MSG_TYPE_TIMEOUT:{
			for(int i = tail_f; i < upper_bound; i++){
				int pos = i % 50;
				SendFRAMEPacket((unsigned char*)(&frame_buffer[pos]), frame_buffer[pos].size);;
			}
		}
		case MSG_TYPE_SEND:{
			int head_pos = head_f % 50;
			frame_buffer[head_pos].head = (frame*)pBuffer;
			frame_buffer[head_pos].size = bufferSize;
			head_f++;
			if(head_f - tail_f <= WINDOW_SIZE_BACK_N_FRAME){
				frame* tmp_frame = (frame*)pBuffer;
				SendFRAMEPacket((unsigned char*)tmp_frame, (unsigned int)bufferSize);
				upper_bound++;
			}
			return 0;
		}
		case MSG_TYPE_RECEIVE:{
			frame* rec_frame = (frame*)pBuffer;
			unsigned int rec_ack = ntohl(rec_frame->head.ack);
			for(int i = tail_f; i < upper_bound; i++){
				int cur_pos = i % 50;
				unsigned int cur_ack = ntohl(frame_buffer[cur_pos].head.seq);
				if(cur_ack <= rec_ack){
					tail_f++;
					if(upper_bound < head_f){
						int upper_pos = upper_bound % 50;
						SendFRAMEPacket((unsigned char*)(&frame_buffer[upper_pos]), frame_buffer[upper_pos].size);
						upper_bound++;
					}
					else break;
				}
			}
			return 0;
		}
		default: break;
	}
	return -1;
}

int stud_slide_window_choice_frame_resend(char *pBuffer, int bufferSize, UINT8 messageType){
	return 0;
}