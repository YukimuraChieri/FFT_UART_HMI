#include "key.h"
#include "delay.h"

// ���尴������
KEY_Object_T KEY1_Obj, KEY2_Obj, KEY3_Obj;


// ������ʼ������
void KEY_Init(void)
{
	// ����IO����
	RCC->APB2ENR|=1<<2;    		// ʹ��PORTAʱ��	 
	GPIOA->CRL&=0XFF000FFF; 	// PA3/4/5����
	GPIOA->CRL|=0X00888000;	
  GPIOA->ODR|=7<<3;					// PA3/4/5��������
	
	KEY1_Obj.state = KEY_Up;	// ��ʼ������1����״̬
	KEY2_Obj.state = KEY_Up;	// ��ʼ������2����״̬
	KEY3_Obj.state = KEY_Up;	// ��ʼ������3����״̬
	
	KEY1_Obj.action = KEY_Action_None;	// ��ʼ����������
	KEY2_Obj.action = KEY_Action_None;	// ��ʼ����������
	KEY3_Obj.action = KEY_Action_None;	// ��ʼ����������
}


// ����״̬��
void KEY_StateMachine(KEY_Object_T* key, uint8_t pin)
{
	switch(key->state)
	{
		case KEY_Up: {									// ������̧��״̬
			if (0 == pin)									// �������ŵ͵�ƽ
			{		
				key->state = KEY_Pre_Down;	// �л�������Ԥ����״̬
				key->cnt = 0;								// �����ʱ����ֵ
			}
		} break;
		
		case KEY_Pre_Down: {						// ����Ԥ����״̬
			if (10 == key->cnt)						// ��ʱ�����ﵽ10ms
			{
				if (0 != pin)								// pin��Ϊ�ߵ�ƽ
				{
					key->state = KEY_Up;			// �л��ذ�����̧��״̬
				}
				else
				{
					key->state = KEY_Down;					// �л��������Ѱ���״̬
					key->action = KEY_Action_Down;	// �������¶���
				}
			}
			key->cnt++;
		} break;
		
		case KEY_Down: {								// �����Ѱ���״̬
			if (0 != pin)									// �������Ÿߵ�ƽ
			{	
				key->state = KEY_Pre_Up;		// �л�������Ԥ̧��״̬
				key->cnt = 0;								// �����ʱ����ֵ
			}
		} break;
		
		case KEY_Pre_Up: {							// ����Ԥ̧��״̬
			if (10 == key->cnt)						// ��ʱ�����ﵽ10ms
			{
				if (0 == pin)								// pin��Ϊ�͵�ƽ
				{
					key->state = KEY_Down;		// �л��ذ����Ѱ���״̬
				}
				else
				{
					key->state = KEY_Up;					// �л��������Ѱ���״̬
					key->action = KEY_Action_Up;	// ����̧����
				}
			}
			key->cnt++;
		} break;
	}
}

// ��ȡ������־������0
KEY_Action_E KEY_Get_Flag(KEY_Object_T* key)
{
	KEY_Action_E action = KEY_Action_None;
	
	if (KEY_Action_None != key->action)
	{
		action = key->action;
		key->action = KEY_Action_None;
	}
	
	return action;
}

