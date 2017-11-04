#include "sysinclude.h"
#include <queue>
#include <iostream>
using namespace std;

extern void SendFRAMEPacket(unsigned char* pData, unsigned int len);

#define WINDOW_SIZE_STOP_WAIT 1
#define WINDOW_SIZE_BACK_N_FRAME 4

typedef enum {data,ack,nak} frame_kind; 
typedef struct frame_head
{
	frame_kind kind;              
	unsigned int seq;             
	unsigned int ack;             
	unsigned char data[100];      
};
typedef struct frame
{
	frame_head head;              
	unsigned int size;            
};
typedef struct packet{
	frame* one_frame;
	int len;
};
queue<packet> buffer;
int size = 0;  //目前窗口的长度
bool yes_no = true;


int stud_slide_window_stop_and_wait(char *pBuffer, int bufferSize, UINT8 messageType)
{
	packet cur_frame_packet;  //将帧打包
	cur_frame_packet.one_frame = new frame();
	cur_frame_packet.one_frame = (frame*)pBuffer;
	cur_frame_packet.len = bufferSize;

	if(messageType == MSG_TYPE_SEND){
		packet.push_back(cur_frame_packet);
		if(yes_no){  //停等协议，窗口空闲即可发送
			yes_no = false;  //等待确认的过程，不能再次发送
			SendFRAMEPacket((unsigned char*)pBuffer, bufferSize);  //发送当前帧
		} 
	}
	else if(messageType == MSG_TYPE_RECEIVE){
		unsigned int ack = cur_frame_packet.one_frame->head.seq;
		if(buffer.size() > 0){
			if(ack == buffer.front().one_frame->head.seq){
				buffer.pop();
				yes_no = true;  //接受到帧，可以再次发送
				if(buffer.size() > 0){  //此时buffer还有缓存帧的话，直接发送
					SendFRAMEPacket((unsigned char*)buffer.front().one_frame, buffer.front().len);
					yes_no = false;
				}
			}

		}
	}
	else if(messageType == MSG_TYPE_TIMEOUT){
		SendFRAMEPacket((unsigned char*)buffer.front().one_frame, buffer.front().len);
	}
	return 0;
}

int stud_slide_window_back_n_frame(char *pBuffer, int bufferSize, UINT8 messageType)
{
	/*static int head = 1;
	static int tail = 1;
	static int size = 0;
	static char list[30][100];
	static int list_size[30];	
	if(messageType == MSG_TYPE_SEND){
		list_size[tail] = bufferSize;
		for(int i = 0 ; i < list_size[tail]; i++){
			list[tail][i] = pBuffer[i];
		}
		tail++;
		if(size >= WINDOW_SIZE_BACK_N_FRAME){
			return -1;
		}
		size++;
		SendFRAMEPacket((unsigned char *)pBuffer, bufferSize);
	}
	else if(messageType == MSG_TYPE_RECEIVE){
		frame *packet = (frame *)pBuffer;
		while(head <= ntohl(packet->head.ack)){
			head++;
			size--;
		}
		for (int i = head + size; i < tail && size < WINDOW_SIZE_BACK_N_FRAME; i++){
			SendFRAMEPacket((unsigned char *)list[i], list_size[i]);
			size++;
		}
	}
	else if(messageType == MSG_TYPE_TIMEOUT){
		for(int i = 0; i < tail - head && i < WINDOW_SIZE_BACK_N_FRAME; i++){
			SendFRAMEPacket((unsigned char *)list[head + i], list_size[head + i]);
		}
	}*/
	return 0;
}

int stud_slide_window_choice_frame_resend(char *pBuffer, int bufferSize, UINT8 messageType)
{
	return 0;
}
