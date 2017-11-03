#include "sysinclude.h" 
#include <iostream> 
using namespace std;   
extern void SendFRAMEPacket(unsigned char* pData, unsigned int len); //1���ػ�������

#define WINDOW_SIZE_STOP_WAIT 1 //����N֡Э��  
#define WINDOW_SIZE_BACK_N_FRAME 4 //��������С  
#define BUFFER_SIZE 50

typedef enum {DATA,ACK,NAK} Frame_kind; //֡ͷ  
typedef struct Frame_head {  
	Frame_kind kind;  
	unsigned int seq;  
	unsigned int ack;  
	unsigned char data[100]; 
}; 
//֡  
typedef struct Frame {  
	Frame_head head;  
	unsigned int size; 
}; 
//������  
Frame buffer[BUFFER_SIZE];  
//��ǰϣ��ȷ�ϵ�֡����������֡����һ��Ҫ���͵�֡  
unsigned int expect_frame = 0, last_buffered_frame = 0, next_frame = 0;  
/*  
* ͣ��Э����Ժ��� 
*/  
int stud_slide_window_stop_and_wait(char *pBuffer, int bufferSize, UINT8 messageType) { 
	 Frame_head* p = (Frame_head*)pBuffer;  //
	 unsigned int frameNum = ntohl(*(unsigned int*)pBuffer);  
	 switch (messageType)  {   //�����Ҫ����һ֡����ʱ   
	 	case MSG_TYPE_SEND : {    //���浱ǰҪ���͵�֡    
			buffer[last_buffered_frame % BUFFER_SIZE].head = *p;    
			buffer[last_buffered_frame % BUFFER_SIZE].size = bufferSize;    //�����´λ����λ��    
			++ last_buffered_frame;    //�����ǰ�п��еĴ��ڿ�����
			if (last_buffered_frame - expect_frame <= WINDOW_SIZE_STOP_WAIT){     //���ͻ���֡   
				SendFRAMEPacket((unsigned char*)pBuffer, (unsigned int)bufferSize);     //�����Ͻ��1     
				++ next_frame;    
			}              
			return 0;   
		}
		//������յ�һ֡����ʱ   
		case MSG_TYPE_RECEIVE : {    //����ȷ���ź�              
			unsigned int ack = ntohl(p->ack);    //��̽              
			for (int i = expect_frame; i < next_frame; ++ i){     
				unsigned int exp_ack = ntohl(buffer[i % BUFFER_SIZE].head.seq);     //���ȷ����Ϣ���ڵ���������Ϣ����˵������֡�Ѿ���ȷ�Ϲ���     
				if (ack >= exp_ack)     {      //����֡+1      
					++ expect_frame;      //����л���֡û����      
					if (next_frame < last_buffered_frame)      {       //����һ������֡       
						SendFRAMEPacket((unsigned char*)(&buffer[next_frame % BUFFER_SIZE]), buffer[next_frame % BUFFER_SIZE].size);       //�����Ͻ�+1       
						++ next_frame;      
					}      //�����޷���������֡���˳����ȴ��´��ٸ�������ȷ��֡      
					else break;     
				}    
			}    
			return 0;   
		} 
		//��ʱ      
		case MSG_TYPE_TIMEOUT : {    //��ʱ�ط����ѵ�ǰ������δȷ�ϵ�֡�ط�һ��    
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
* ����n֡���Ժ��� 
*/  
int stud_slide_window_back_n_frame(char *pBuffer, int bufferSize, UINT8 messageType) {      
	Frame_head* p = (Frame_head*)pBuffer;  
	unsigned int timeoutNum = *(unsigned int*)pBuffer;  
	switch (messageType)  {   //�����Ҫ����һ֡����ʱ   
		case MSG_TYPE_SEND : {    
			buffer[last_buffered_frame % BUFFER_SIZE].head = *p;    
			buffer[last_buffered_frame % BUFFER_SIZE].size = bufferSize;    
			++ last_buffered_frame;    //������δ�����ޣ����͵�ǰ����֡�����´����Ͻ�    
			if (last_buffered_frame - expect_frame <= WINDOW_SIZE_BACK_N_FRAME)    {     
				SendFRAMEPacket((unsigned char*)z, (unsigned int)bufferSize);     
				++ next_frame;    
			}              
			return 0;   
		}   //�������յ�һ֡����ʱ   
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
		//��ʱ      
		case MSG_TYPE_TIMEOUT : {    
			cout << "time out : " << timeoutNum << endl;    //��ʱ�ط�    
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
* ѡ�����ش����Ժ��� 
*/  
int stud_slide_window_choice_frame_resend(char *pBuffer, int bufferSize, UINT8 messageType) {  
	return 0; 
}
