#include "sysinclude.h"

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

int stud_slide_window_stop_and_wait(char *pBuffer, int bufferSize, UINT8 messageType)
{
	static int head = 1;                   //待发送的帧
	static int tail = 1;                   //数组最后的帧
	static int size = 0;                   //窗口长度
	static char list[30][100];           //帧数组
	static int list_size[30];	         //帧长度数组
	if(messageType == MSG_TYPE_SEND){
		list_size[tail] = bufferSize;
		for(int i = 0 ; i < list_size[tail] ; i++){
			list[tail][i] = pBuffer[i];
		}
		
		tail++;		
		if(size >= WINDOW_SIZE_STOP_WAIT){
			return -1;
		}
		size++;
		SendFRAMEPacket((unsigned char*)pBuffer, bufferSize);
	}
	else if(messageType == MSG_TYPE_RECEIVE){
		frame *packet = (frame *)pBuffer;
		if(size < WINDOW_SIZE_STOP_WAIT || ntohl(packet->head.ack) != head){
			return -1;
		}
		head++;
		size--;
		for(int i = head ; i < tail && size < WINDOW_SIZE_STOP_WAIT ; i++){
			SendFRAMEPacket((unsigned char*)list[head], list_size[head]);
			size++;
		}
	}
	else if(messageType == MSG_TYPE_TIMEOUT){
		SendFRAMEPacket((unsigned char*)list[head], list_size[head]);
	}
	return 0;
}

int stud_slide_window_back_n_frame(char *pBuffer, int bufferSize, UINT8 messageType)
{
	static int head = 1;
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
	}
	return 0;
}

int stud_slide_window_choice_frame_resend(char *pBuffer, int bufferSize, UINT8 messageType)
{
	return 0;
}
