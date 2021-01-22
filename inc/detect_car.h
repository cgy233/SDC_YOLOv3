#define D_CAR_NUM                 8  //����⳵����Ŀ
#define FORWARD_TIME              200  //���һ֡����ʱ�䣬����
#define WAIT_TIME          1 * 30 * 1000  //ͣ���೤ʱ�俪ʼ��������־
#define CHECK_CAR_IN_TIME         15  //ÿ�μ���жϳ�������ʱ����
#define OBJECT_MIN_DISTANCE 100  //��֡Ŀ���ƶ��ľ����ֵ
#define CAR_LEVEL_TIME      10  //���ж���֡û�и��º������������

#define DEBUG_LOG                      1
#define DEBUG                          0

#define DEBUG_2                        1

#define CAR_IN_NUM                     10 //�����������괦�ڳ�λ�ڵ�����

//ST_POINT A[] = {{765, 491}, {872, 483}, {1010, 530},  {875, 541}};

#define PARK_COR_X1 774
#define PARK_COR_Y1 401

#define PARK_COR_X2 970
#define PARK_COR_Y2 401

#define PARK_COR_X3 970
#define PARK_COR_Y3 512

#define PARK_COR_X4 774
#define PARK_COR_Y4 512

#define  NEW_DETECT_F                    1 

#define max(a,b)(a>b?a:b)
#define min(a,b)(a<b?a:b)

/**
 * Handle a single line
 * You could implement your code function here.
 */
typedef struct tagST_POINT {
	int x;
	int y;
} ST_POINT;

/**
 * ���ܣ��жϵ��Ƿ��ڶ������
 * ���������ͨ���õ��ˮƽ�ߣ����ߣ������θ��ߵĽ���
 * ���ۣ����߽���Ϊ����������!
 * ������p ָ����ĳ����
		 ptPolygon ����εĸ����������꣨��ĩ����Բ�һ�£�
		 nCount ����ζ���ĸ���
 * ˵����
 */
#if NEW_DETECT_F
int PtInPolygon(ST_POINT p, ST_POINT* ptPolygon, int nCount)
{
	int nCross = 0, i;
	double x;
	ST_POINT p1, p2, p3, p4;


	p1 = ptPolygon[0];
	p2 = ptPolygon[1];
	p3 = ptPolygon[2];
	p4 = ptPolygon[4];

	if ((p.x > p1.x) && (p.x < p3.x))
	{
		if ((p.y > p1.y) && (p.y < p3.y))
		{
			return 1;
		}
	}
	
	return 0;
}

#else
int PtInPolygon(ST_POINT p, ST_POINT* ptPolygon, int nCount)
{
	int nCross = 0, i;
	double x;
	ST_POINT p1, p2;

	for (i = 0; i < nCount; i++)
	{
		p1 = ptPolygon[i];
		p2 = ptPolygon[(i + 1) % nCount];
		// ��� y=p.y �� p1p2 �Ľ���
		if (p1.y == p2.y) // p1p2 �� y=p.yƽ�� 
			continue;
		if (p.y < min(p1.y, p2.y)) // ������p1p2�ӳ����� 
			continue;
		if (p.y >= max(p1.y, p2.y)) // ������p1p2�ӳ����� 
			continue;
		// �󽻵�� X ���� -------------------------------------------------------------- 
		x = (double)(p.y - p1.y) * (double)(p2.x - p1.x) / (double)(p2.y - p1.y) + p1.x;
		if (x > p.x)
		{
			nCross++; // ֻͳ�Ƶ��߽��� 
		}
	}
	// ���߽���Ϊż�������ڳ�λ֮�� ---
	if (nCross % 2 == 1)
	{
		printf("CAR IN PARK.\n");
		return 1;
	}
	else
	// ���߽���Ϊ���������ڳ�λ֮�� ---
	{
		return 0;
	}
	//return (nCross % 2 == 1); 
}
#endif

void trigger_car_track(int middle_data, int buffer_num, int one_car_pint_num, int *front, int *back, int *car_exist_flag, int *one_point)
{

	int cur_front = front[buffer_num];
	int cur_back = back[buffer_num];
	int full_flag = is_queue_full(cur_back, cur_front, one_car_pint_num);
	if (full_flag == 0) //����������£�����������Ӳ�����
	{
	    //����һ���㡣
		//���
		cur_front = enQueue(one_point, cur_back, cur_front, one_car_pint_num, middle_data);
		front[buffer_num] = cur_front;
	}
	else
	{
		printf("[error] : first must keep buffer empty\n");
	}
	//�˳��������buffer���й���
	car_exist_flag[buffer_num] = 1;
}

int get_middle_data(int mid_x, int mid_y)
{
	int data = 0;
	data = (mid_y<<16)+ mid_x;
	return data;
}

// ���㳵��Ŀ������֮���ֱ�߾����ƽ��
int calc_distant(int middle_data, int cur_data)
{
	int mid_x = 0;
	int mid_y = 0;
	int cur_mid_x = 0;
	int cur_mid_y = 0;
	int distance = 0;

	mid_x = middle_data &0xffff;
	mid_y = (middle_data>>16) &0xffff;

	cur_mid_x = cur_data &0xffff;
	cur_mid_y = (cur_data>>16) &0xffff;
#if DEBUG_LOG
	printf("cur_mid_x: %d\n", cur_mid_x);
	printf("cur_mid_y: %d\n", cur_mid_y);
#endif

	//distance = abs(mid_x - cur_mid_x) + abs(mid_y - cur_mid_y);
	distance = (mid_x - cur_mid_x)*(mid_x - cur_mid_x) + (mid_y - cur_mid_y)*(mid_y - cur_mid_y);

	return distance;
}

// �����
int enQueue(int *a,int back,int front,int max, int data){
    //����ж���䣬���front����max����ֱ�ӽ����a[0]���¿�ʼ�洢�����front+1��back�غϣ����ʾ��������
    if ((front+1)%max==back) {
        printf("�ռ�����");
        return front;
    }
    a[front%max]=data;
    front++;
	front=front%max;
	//printf("��� = %d, data = %d\n", front, data);
    return front;
}

// ��ȡ��Ч�����������������
int get_cur_data_queue(int *a,int front, int max){
    int data = 0;
	int last_indx = 0;
	if (front == 0)
	{
		last_indx = max - 1;
	}
	else
	{
		last_indx = front - 1;
	}

	data = a[last_indx];
    return data;
}

// ������
int  deQueue(int *a,int back,int front, int max){
    //���front==rear����ʾ����Ϊ��
    if(back==front%max) {
        printf("����Ϊ��");
        return back;
    }
    //printf("%d ",a[back]);
    //back����ֱ�� +1������+1��ͬmax���бȽϣ����=max����ֱ����ת�� a[0]
    back=(back+1)%max;
    return back;
}

// ������������־
int is_queue_full(int cur_back, int cur_front, int max)
{
	int full_flag = 0;
	if ((cur_front+1)%max==cur_back)
	{
		//printf("�ռ�������־\n");
		full_flag = 1;
	}
	else 
	{
		full_flag = 0;
	}
	return full_flag;
}

// ��ȡ���г��� ǰ���־�ͺ����־��ֵ
int get_queue_len(int cur_back, int cur_front, int max)
{
	int len = 0;
	if ((cur_front == 0) && (cur_back == 0))
	{
		len = 0;
	}
	else if (cur_front > cur_back)
	{
		len = cur_front - cur_back;
	}
	else if (cur_front < cur_back)
	{
		len = max-(cur_back - cur_front);
	}
	else
	{
		printf("[error] len ");

	}
	return len;
}

//  �洢�켣���ݽ�buff
void put_mid_data_buffer(int middle_data, int select_buffer_num, int *front, int *back, int one_car_pint_num, int *p_buffer)
{
	int ret_cur_front = 0;

	int cur_front = front[select_buffer_num];
	int cur_back = back[select_buffer_num];
	int full_flag = is_queue_full(cur_back, cur_front, one_car_pint_num);
	if (full_flag == 0) //����������£�����������Ӳ�����
	{
	    //����һ���㡣
		//���
		cur_front = enQueue(p_buffer, cur_back, cur_front, one_car_pint_num, middle_data);
		front[select_buffer_num] = cur_front;
	}
	else //��������£��ȳ������ݡ�
	{
			//�ȳ���
		cur_back=deQueue(p_buffer, cur_back, cur_front, one_car_pint_num);
		back[select_buffer_num] = cur_back;
		//�����
		cur_front = enQueue(p_buffer, cur_back, cur_front, one_car_pint_num, middle_data);
		front[select_buffer_num] = cur_front; 
	}
}

// ��鳵λ������û�г�
int check_car_weather_in(int *buffer,int front, int back, int max)
{
    int data = 0;
	int last_indx = 0;
	int car_in_flag = 0;
	int car_in_num = 0;
	int mid_x = 0;
	int mid_y = 0;
	int i = 0;

	ST_POINT p;
	//ST_POINT A[] = {{1294, 744}, {1466, 729}, {1729, 812},  {1532, 841}};
	//ST_POINT A[] = {{738, 486}, {848, 480}, {991, 526},  {855, 536}};
	//ST_POINT A[] = {{765, 491}, {872, 483}, {1010, 530},  {875, 541}};
	ST_POINT A[] = { {PARK_COR_X1,PARK_COR_Y1}, {PARK_COR_X2, PARK_COR_Y2}, {PARK_COR_X3,PARK_COR_Y3},  {PARK_COR_X4,PARK_COR_Y4} };


	int count = 4;
	int len = get_queue_len(back, front, max);

	for(i = 0;i < len; i++)
	{
		last_indx = front - (i + 1);
		if (last_indx < 0)
		{
			last_indx = max + last_indx;
		}
		data = buffer[last_indx];
		mid_x = data &0xffff;
	    mid_y = (data>>16) &0xffff;	
		//�ж��Ƿ��ڳ�λ��
		p.x = mid_x;
		p.y = mid_y;
		
		if (PtInPolygon(p, A, count) == 1) 
		{
			//printf("�ڳ�λ��\n");
			car_in_num ++;

		}
		//else
		//{
		//	printf("�ڳ�λ��\n");
		//}
	}
	// ����⵽�ڳ�λ���������������������õ���ֵʱ������ͣ����־
	if (car_in_num >= CAR_IN_NUM)
	{
		car_in_flag = 1;
#if DEBUG
		output_buffer_data(buffer, front, back, max);
#endif
	}
	return car_in_flag;
}

int get_car_buffer_num(int *car_exist_flag)
{
	int buffer_num = 0;
	int is_empty = 0;
	int i = 0;
	for(i=0;i<D_CAR_NUM; i++)
	{
		if (car_exist_flag[i]  == 0)
		{
			buffer_num = i;
			is_empty = 1;
			break;
		}
	}
	if (is_empty == 0)
	{
		printf("[error] : car_exist_flag\n");

	}
	return buffer_num;
}

int check_last_frame(int last_frame_num, int frame_num)
{
	int car_leaving_flag = 0;
	if (frame_num - last_frame_num > CAR_LEVEL_TIME)
	{
		car_leaving_flag = 1;
		printf("��ʾ���Ѿ��뿪��");
	}
	return car_leaving_flag;
}
