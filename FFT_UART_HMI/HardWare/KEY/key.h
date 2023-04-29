#ifndef __KEY_H
#define __KEY_H

#include "sys.h"

#define KEY1_Pin	PAin(3)	// ��������1
#define KEY2_Pin	PAin(4)	// ��������2
#define KEY3_Pin	PAin(5)	// ��������3

// ����״̬ö�����Ͷ���
typedef enum
{
	KEY_Up = 0,			// ������̧��
	KEY_Pre_Down,		// ����Ԥ����
	KEY_Down,				// �����Ѱ���
	KEY_Pre_Up			// ����Ԥ̧��
}KEY_STATE_E;

// ��������ö�����Ͷ���
typedef enum
{
	KEY_Action_Down = 0,	// �������¶���
	KEY_Action_Up,				// ����̧����
	KEY_Action_None				// �����޶���
}KEY_Action_E;

// ��������ṹ�����Ͷ���
typedef struct
{
	KEY_STATE_E state;		// ����״̬
	uint16_t cnt;					// ��ʱ����
	KEY_Action_E action;	// ��������
}KEY_Object_T;


// ������������
extern KEY_Object_T KEY1_Obj, KEY2_Obj, KEY3_Obj;

void KEY_Init(void);				// ������ʼ������
void KEY_StateMachine(KEY_Object_T* key, uint8_t pin);	// ����״̬��
KEY_Action_E KEY_Get_Flag(KEY_Object_T* key);	// ��ȡ������־



#endif

