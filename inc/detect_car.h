#define D_CAR_NUM                 8  //最多检测车辆数目
#define FORWARD_TIME              200  //检测一帧所需时间，毫秒
#define WAIT_TIME          1 * 30 * 1000  //停车多长时间开始产生入库标志
#define CHECK_CAR_IN_TIME         15  //每次检测判断车进入库的时间间隔
#define OBJECT_MIN_DISTANCE 100  //两帧目标移动的距离差值
#define CAR_LEVEL_TIME      10  //队列多少帧没有更新后清楚整个队列

#define DEBUG_LOG                      1
#define DEBUG                          0

#define DEBUG_2                        1

#define CAR_IN_NUM                     10 //队列里面坐标处于车位内的数量

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
 * 功能：判断点是否在多边形内
 * 方法：求解通过该点的水平线（射线）与多边形各边的交点
 * 结论：单边交点为奇数，成立!
 * 参数：p 指定的某个点
		 ptPolygon 多边形的各个顶点坐标（首末点可以不一致）
		 nCount 多边形定点的个数
 * 说明：
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
		// 求解 y=p.y 与 p1p2 的交点
		if (p1.y == p2.y) // p1p2 与 y=p.y平行 
			continue;
		if (p.y < min(p1.y, p2.y)) // 交点在p1p2延长线上 
			continue;
		if (p.y >= max(p1.y, p2.y)) // 交点在p1p2延长线上 
			continue;
		// 求交点的 X 坐标 -------------------------------------------------------------- 
		x = (double)(p.y - p1.y) * (double)(p2.x - p1.x) / (double)(p2.y - p1.y) + p1.x;
		if (x > p.x)
		{
			nCross++; // 只统计单边交点 
		}
	}
	// 单边交点为偶数，点在车位之外 ---
	if (nCross % 2 == 1)
	{
		printf("CAR IN PARK.\n");
		return 1;
	}
	else
	// 单边交点为奇数，点在车位之内 ---
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
	if (full_flag == 0) //不满的情况下，进行数据入队操作。
	{
	    //处理一个点。
		//入队
		cur_front = enQueue(one_point, cur_back, cur_front, one_car_pint_num, middle_data);
		front[buffer_num] = cur_front;
	}
	else
	{
		printf("[error] : first must keep buffer empty\n");
	}
	//此车辆进入此buffer进行管理
	car_exist_flag[buffer_num] = 1;
}

int get_middle_data(int mid_x, int mid_y)
{
	int data = 0;
	data = (mid_y<<16)+ mid_x;
	return data;
}

// 计算车辆目标两点之间的直线距离的平方
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

// 入队列
int enQueue(int *a,int back,int front,int max, int data){
    //添加判断语句，如果front超过max，则直接将其从a[0]重新开始存储，如果front+1和back重合，则表示数组已满
    if ((front+1)%max==back) {
        printf("空间已满");
        return front;
    }
    a[front%max]=data;
    front++;
	front=front%max;
	//printf("入队 = %d, data = %d\n", front, data);
    return front;
}

// 读取有效队列里面的最新坐标
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

// 出队列
int  deQueue(int *a,int back,int front, int max){
    //如果front==rear，表示队列为空
    if(back==front%max) {
        printf("队列为空");
        return back;
    }
    //printf("%d ",a[back]);
    //back不再直接 +1，而是+1后同max进行比较，如果=max，则直接跳转到 a[0]
    back=(back+1)%max;
    return back;
}

// 队列满产生标志
int is_queue_full(int cur_back, int cur_front, int max)
{
	int full_flag = 0;
	if ((cur_front+1)%max==cur_back)
	{
		//printf("空间已满标志\n");
		full_flag = 1;
	}
	else 
	{
		full_flag = 0;
	}
	return full_flag;
}

// 获取队列长度 前向标志和后向标志的值
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

//  存储轨迹数据进buff
void put_mid_data_buffer(int middle_data, int select_buffer_num, int *front, int *back, int one_car_pint_num, int *p_buffer)
{
	int ret_cur_front = 0;

	int cur_front = front[select_buffer_num];
	int cur_back = back[select_buffer_num];
	int full_flag = is_queue_full(cur_back, cur_front, one_car_pint_num);
	if (full_flag == 0) //不满的情况下，进行数据入队操作。
	{
	    //处理一个点。
		//入队
		cur_front = enQueue(p_buffer, cur_back, cur_front, one_car_pint_num, middle_data);
		front[select_buffer_num] = cur_front;
	}
	else //满的情况下，先出队数据。
	{
			//先出队
		cur_back=deQueue(p_buffer, cur_back, cur_front, one_car_pint_num);
		back[select_buffer_num] = cur_back;
		//再入队
		cur_front = enQueue(p_buffer, cur_back, cur_front, one_car_pint_num, middle_data);
		front[select_buffer_num] = cur_front; 
	}
}

// 检查车位里面有没有车
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
		//判断是否在车位内
		p.x = mid_x;
		p.y = mid_y;
		
		if (PtInPolygon(p, A, count) == 1) 
		{
			//printf("在车位内\n");
			car_in_num ++;

		}
		//else
		//{
		//	printf("在车位外\n");
		//}
	}
	// 当检测到在车位里面的坐标点数量超过设置的阈值时，产生停车标志
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
		printf("表示车已经离开了");
	}
	return car_leaving_flag;
}
