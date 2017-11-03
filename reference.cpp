#include "sysinclude.h" 
#include <iostream> 
using namespace std;   
extern void SendFRAMEPacket(unsigned char* pData, unsigned int len); //1比特滑动窗口

#define WINDOW_SIZE_STOP_WAIT 1 //回退N帧协议  
#define WINDOW_SIZE_BACK_N_FRAME 4 //缓存区大小  
#define BUFFER_SIZE 50

typedef enum {DATA,ACK,NAK} Frame_kind; //帧头  
typedef struct Frame_head {  
	Frame_kind kind;  
	unsigned int seq;  
	unsigned int ack;  
	unsigned char data[100]; 
}; 
//帧  
typedef struct Frame {  
	Frame_head head;  
	unsigned int size; 
}; 
//缓存区  
Frame buffer[BUFFER_SIZE];  
//当前希望确认的帧，最近缓存的帧，下一次要发送的帧  
unsigned int expect_frame = 0, last_buffered_frame = 0, next_frame = 0;  
/*  
* 停等协议测试函数 
*/  
int stud_slide_window_stop_and_wait(char *pBuffer, int bufferSize, UINT8 messageType) { 
	 Frame_head* p = (Frame_head*)pBuffer;  //
	 unsigned int frameNum = ntohl(*(unsigned int*)pBuffer);  
	 switch (messageType)  {   //网络层要发送一帧数据时   
	 	case MSG_TYPE_SEND : {    //缓存当前要发送的帧    
			buffer[last_buffered_frame % BUFFER_SIZE].head = *p;    
			buffer[last_buffered_frame % BUFFER_SIZE].size = bufferSize;    //更新下次缓存的位置    
			++ last_buffered_frame;    //如果当前有空闲的窗口可以用
			if (last_buffered_frame - expect_frame <= WINDOW_SIZE_STOP_WAIT){     //发送缓存帧   
				SendFRAMEPacket((unsigned char*)pBuffer, (unsigned int)bufferSize);     //窗口上界加1     
				++ next_frame;    
			}              
			return 0;   
		}
		//物理层收到一帧数据时   
		case MSG_TYPE_RECEIVE : {    //解码确认信号              
			unsigned int ack = ntohl(p->ack);    //试探              
			for (int i = expect_frame; i < next_frame; ++ i){     
				unsigned int exp_ack = ntohl(buffer[i % BUFFER_SIZE].head.seq);     //如果确认消息大于等于期望消息，则说明期望帧已经被确认过了     
				if (ack >= exp_ack)     {      //期望帧+1      
					++ expect_frame;      //如果有缓存帧没发出      
					if (next_frame < last_buffered_frame)      {       //发出一个缓存帧       
						SendFRAMEPacket((unsigned char*)(&buffer[next_frame % BUFFER_SIZE]), buffer[next_frame % BUFFER_SIZE].size);       //窗口上界+1       
						++ next_frame;      
					}      //否则，无法继续发送帧，退出。等待下次再更新期望确认帧      
					else break;     
				}    
			}    
			return 0;   
		} 
		//超时      
		case MSG_TYPE_TIMEOUT : {    //超时重发，把当前窗口内未确认的帧重发一遍    
			for (int i = expect_frame; i < next_frame; ++ i)    { 
				    if (frameNum > ntohl(buffer[i % BUFFER_SIZE].head.seq)) 
						continue;     
					SendFRAMEPacket((unsigned char*)(&buffer[i % BUFFER_SIZE]), buffer[i % BUFFER_SIZE].size);   
			}    
			return 0;   
		}             
		default : break;  
	}  
	return -1; 
}

/*  
* 回退n帧测试函数 
*/  
int stud_slide_window_back_n_frame(char *pBuffer, int bufferSize, UINT8 messageType) {      
	Frame_head* p = (Frame_head*)pBuffer;  
	unsigned int timeoutNum = *(unsigned int*)pBuffer;  
	switch (messageType)  {   //网络层要发送一帧数据时   
		case MSG_TYPE_SEND : {    
			buffer[last_buffered_frame % BUFFER_SIZE].head = *p;    
			buffer[last_buffered_frame % BUFFER_SIZE].size = bufferSize;    
			++ last_buffered_frame;    //窗口数未达上限，则发送当前缓存帧，更新窗口上界    
			if (last_buffered_frame - expect_frame <= WINDOW_SIZE_BACK_N_FRAME)    {     
				SendFRAMEPacket((unsigned char*)z, (unsigned int)bufferSize);     
				++ next_frame;    
			}              
			return 0;   
		}   //物理层接收到一帧数据时   
		case MSG_TYPE_RECEIVE : {              
			unsigned int ack = ntohl(p->ack);              
			cout << "receive ack : " << ack << endl;              
			for (int i = expect_frame; i < next_frame; ++ i)    {     
				unsigned int exp_ack = ntohl(buffer[i % BUFFER_SIZE].head.seq);     
				cout << "exp_ack : " << exp_ack << endl; 
				if (ack >= exp_ack)     {      
					++ expect_frame;      
					if (next_frame < last_buffered_frame)      {       
						SendFRAMEPacket((unsigned char*)(&buffer[next_frame % BUFFER_SIZE]), buffer[next_frame % BUFFER_SIZE].size);       
						++ next_frame;      
					}     
				}     
				else break;    
			}    
			return 0;   
		}
		//超时      
		case MSG_TYPE_TIMEOUT : {    
			cout << "time out : " << timeoutNum << endl;    //超时重发    
			for (int i = expect_frame; i < next_frame; ++ i)    {     
				unsigned int frameNum = ntohl(buffer[i % BUFFER_SIZE].head.seq);     //
				if (timeoutNum > frameNum) continue;     
				cout << frameNum << endl;     
				SendFRAMEPacket((unsigned char*)(&buffer[i % BUFFER_SIZE]), buffer[i % BUFFER_SIZE].size);    
			}    
			return 0;   
		}          
		default : break;  
	}  
	return -1; 
}

/*  
* 选择性重传测试函数 
*/  
int stud_slide_window_choice_frame_resend(char *pBuffer, int bufferSize, UINT8 messageType) {  
	return 0; 
}
