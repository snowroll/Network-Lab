#include "sysinclude.h"

extern void SendFRAMEPacket(unsigned char* pData, unsigned int len);

#define WINDOW_SIZE_STOP_WAIT 1
#define WINDOW_SIZE_BACK_N_FRAME 4
#define BUFFER_SIZE 50

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
typedef struct frame_packet{
	frame* per_frame;
	unsigned int len;
};

frame_packet buffer[BUFFER_SIZE];
static int head = 1, tail = 1, win_size = 0;

int stud_slide_window_stop_and_wait(char *pBuffer, int bufferSize, UINT8 messageType)
{
	frame* tmp_frame = (frame*)pBuffer;
	switch(messageType){
		case MSG_TYPE_SEND:{
			buffer[tail % BUFFER_SIZE].per_frame = tmp_frame;
			buffer[tail % BUFFER_SIZE].len = bufferSize;
			tail++;		
			if(size >= WINDOW_SIZE_STOP_WAIT){
				return -1;
			}
			size++;
			SendFRAMEPacket((unsigned char*)pBuffer, bufferSize);
		}
		break;

		case MSG_TYPE_RECEIVE:{
			if(size < WINDOW_SIZE_STOP_WAIT || ntohl(packet->head.ack) != head){
				return -1;
			}
			head++;	size--;
			for(int i = head; i < tail && size < WINDOW_SIZE_STOP_WAIT; i++){
				SendFRAMEPacket((unsigned char*)buffer[head % BUFFER_SIZE].per_frame, buffer[head % BUFFER_SIZE].len);
				size++;
			}

		}
		break;

		case MSG_TYPE_TIMEOUT:{
			SendFRAMEPacket((unsigned char*)buffer[head % BUFFER_SIZE].per_frame, buffer[head % BUFFER_SIZE].len);
		}
		break;
		default break;
	}
	return 0;
}

int stud_slide_window_back_n_frame(char *pBuffer, int bufferSize, UINT8 messageType)
{
	frame* tmp_frame = (frame*)pBuffer;
	switch(messageType){
		case MSG_TYPE_SEND:{
			buffer[tail % BUFFER_SIZE].per_frame = tmp_frame;
			buffer[tail % BUFFER_SIZE].len = bufferSize;
			tail++;		
			if(size >= WINDOW_SIZE_BACK_N_FRAME){
				return -1;
			}
			size++;
			SendFRAMEPacket((unsigned char*)pBuffer, bufferSize);
		}
		break;

		case MSG_TYPE_RECEIVE:{
			while(head <= ntohl(tmp_frame->head.ack)){
				head++; size--;
			}
			head++;	size--;
			for(int i = head + size; i < tail && size < WINDOW_SIZE_BACK_N_FRAME; i++){
				SendFRAMEPacket((unsigned char*)buffer[i % BUFFER_SIZE].per_frame, buffer[i % BUFFER_SIZE].len);
				size++;
			}
		}
		break;

		case MSG_TYPE_TIMEOUT:{
			for(int i = 0; i < tail - head && i < WINDOW_SIZE_BACK_N_FRAME; i++){
				SendFRAMEPacket((unsigned char*)buffer[(head + i) % BUFFER_SIZE].per_frame, buffer[(head + i) % BUFFER_SIZE].len);
			}
		}
		break;
		default break;
	}
	return 0;
}

int stud_slide_window_choice_frame_resend(char *pBuffer, int bufferSize, UINT8 messageType)
{
	return 0;
}
