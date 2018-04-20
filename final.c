#include<stdio.h>

#define Energy 100  //机器人的能量最大值
#define RNUM  4//机器人总个数
#define TNUM  5//任务总数
#define Vhigh 3 //高速段限速
#define Vlow 2  //低速段限速
#define a1  3   //计算能量消耗时，距离的系数
#define a2  2 //计算能量消耗时，负载的系数
#define w1  0.3  //计算代价矩阵a时，第一个指标高速段距离的权重
#define w2  0.2  //计算代价矩阵a时，第二个指标低速段距离的权重
#define w3  0.5  //计算代价矩阵a时，第三个指标机器人现有能量的权重
#define maxabs 20
typedef struct robot
{
	// int num; //机器人编号
	float E1;//机器人当前能量值
	float s1;  //机器人完成当前执行任务所需的剩余路程中高速段
	float s2; //机器人完成当前执行任务所需的剩余路程中低速段
	float d1;  //为机器人完成当前任务之后去往工作台需要的路程中高速段
	float d2;  //为机器人完成当前任务之后去往工作台需要的路程中低速段
			   //float Tr; //为机器人到达任务开始地点需要的时间
}robot;

typedef struct task
{
	char label; //工作的标识标签，“N”为普通任务，“C”为充电任务
	int n;      //工作的编号
	float x1[2]; //领取任务的所在地x1
	float x2[2];//任务终点所在地x2
	float td1;  //执行任务运动轨迹中高速段的路程td1
	float td2;  //执行任务运动轨迹中低速段的路程td2
	float weigh; //货架重量weigh
	float Tmax; //任务需要在什么时间内认领接受Tmax

}task;

int find(int s[], int len, double key);
void auction(int array[][TNUM]);
double max(double s[], int len);

int main()
{
	int i, j;
	robot r1[RNUM];
	task r2[TNUM];
	float Tr[RNUM][TNUM];  //执行任务耗时
	float Ec[RNUM][TNUM];  //能量消耗量
	int a[RNUM][TNUM];     //代价矩阵
	int H1, H2, H3;   //计算代价矩阵的三个指标
	int *from;
	printf("/**************Input the informaiton  of robots********/\n");
	for (i = 0; i < RNUM; i++)
	{
		printf("\nInput the informaiton  of the NO.%d robot\n", i + 1);
		printf("Input d1,d2,s1,s2 \n");
		scanf_s("%f %f %f %f", &r1[i].d1, &r1[i].d2, &r1[i].s1, &r1[i].s2);
		printf("Input the current energy \n");
		scanf_s("%f", &r1[i].E1);
	}

	printf("/*********Input the informaiton  of tasks************/\n");
	for (j = 0; j < TNUM; j++)
	{
		printf("\n Input the informaiton  of the NO.%d tasks\n", j + 1);
		printf("Input the location where task begins: \n");
		scanf_s("%f %f", &r2[j].x1[1], &r2[j].x1[2]);
		printf("the location where task is over:\n");
		scanf_s("%f %f", &r2[j].x2[1], &r2[j].x2[2]);
		printf("Input td1,td2\n");
		scanf_s("%f %f", &r2[j].td1, &r2[j].td2);
		printf("Input the weigh of goods:\n");
		scanf_s("%f", &r2[j].weigh);
		printf("Input the max waiting time\n");
		scanf_s("%f", &r2[j].Tmax);
	}
	printf("____________the calculating results______________\n");
	/*当机器人在最大等待时间之内，到达接受工作的地点，且从去往任务台到任务完成总过程剩余能量不低于总能量的10%时，
	机器人可以参与投标*/

	for (i = 0; i<RNUM; i++)
		for (j = 0; j < TNUM; j++)
		{
			//计算到工作台的用时Tr，整个过程耗费能量值Ec.
			Tr[i][j] = (r1[i].s1 + r1[i].d1) / Vhigh + (r1[i].s2 + r1[i].d2) / Vlow;
			Ec[i][j] = a1*(r1[i].s1 + r1[i].s2 + r1[i].d1 + r1[i].d2 + r2[j].td1) + r2[j].td2 + a2*r2[j].weigh;
			printf("The robot %d for task %d spends time:%f s,and costs energy %f\n", i + 1, j + 1, Tr[i][j], Ec[i][j]);

			if ((Tr[i][j] > r2[j].Tmax) || (r1[i].E1 - Ec[i][j] < (Energy*0.1)))
			{
				printf("The robot %d can't bid for the task.\n", i + 1);
				a[i][j] = -1;    //如果不能竞标，代价设置为-1
			}
			else
			{
				printf("The robot %d can bid for the task.\n", i + 1);
				H1 = 100 / (r1[i].s1 + r1[i].d1 + r2[j].td1);
				H2 = 100 / (r1[i].s2 + r1[i].d2 + r2[j].td2);
				H3 = r1[i].E1;
				a[i][j] = w1*H1 + w2*H2 + w3*H3; //可以投标，计算代价a
			}
		}
	//输出代价矩阵a
	printf("_____________The cost matrix a is showed below_____________\n");
	for (i = 0; i < RNUM; i++)
	{
		for (j = 0; j < TNUM; j++)
		{
			printf("%d\t", a[i][j]);
		}
		printf("\n");
	}
	//主函数调用拍卖算法
	auction(a);

	system("pause");
	return 0;
}

//find 函数，找到数组中值为key的元素的个数
int find(int s[], int len, double key)
{
	int i, j = 0;
	for (i = 0; i < len; i++)
	{
		if (s[i] == key)
			j++;
	}

	return j;
}

double max(double s[], int len)
{
	int i;
	double result = -1e10;
	for (i = 0; i < len; i++)
	{
		if (s[i] >= result)
			result = s[i];
	}
	return result;
}


void auction(int array[][TNUM])
{
	double p[TNUM] = { 0 };				  //下标对应物编号，值为物的价格
	double q[RNUM] = { 0 };
	double lamda, delta;
	int h,k;
	int i, j, J;
	double eplison = 1 / (double)(TNUM + 1);  //初始化放缩尺度
	int s1[RNUM];					  //s1下标代表机器人，对应值为分配的任务
	int s2[TNUM];				  //s2下标代表任务，对应数组中的元素为被分配给的机器人
	for (i = 0; i < RNUM; i++) {
		s1[i] = -1;
	}
	for (i = 0; i < TNUM; i++) {
		s2[i] = -1;
	}
	
	double vi, wi, vij;			  //vi为最大利润，wi为次高利润，vij为计算出的当前利润
	double vj, wj, pji;           //逆向拍卖时候用到。
	int s10 = find(s1, RNUM, -1);    //s10为找到还没有建立分配的人的个数，不等于零，则要不断进行投标
									 //double Pj_idx;
	if (RNUM == TNUM)
	{
		while (s10)
		{
			//bidding phase

			double bid[TNUM] = { 0 };     //初始化竞价bid数组。
			int tmps1[RNUM] = { 0 };

			for (i = 0; i < RNUM; i++)
			{
				if (s1[i] == -1)  //未进行分配任务的机器人i选中
				{
					vi = -3 * TNUM*maxabs;
					wi = -5 * TNUM*maxabs;

					//每一个机器人对应的任务，计算最大利润vi和次大利润wi
					for (j = 0; (array[i][j] != -1) && (j < TNUM); j++)
					{
						vij = array[i][j] - p[j];
						if (vij > wi)
							if (vij > vi)
							{
								J = j;
								wi = vi;
								vi = vij;
							}
							else
								wi = vij;
					}
					//vi大于零，找到了最大利润，进行投标价的计算
					if (vi >= 0)
					{
						wi = wi > 0 ? wi : 0;

						tmps1[i] = J;
						bid[i] = p[J] + (double)vi - (double)wi + eplison;

					}
					//vi小于零，未找到最大利润，将其分配设为-10，分配不了
					else if (vi < 0)
						s1[i] = -10;



				}
			}

			//assginment phase
			for (j = 0; j < TNUM; j++)
			{
				int temp0 = -1; //元素记录投标人
				double temp1 = -1;//记录投标价格
				int Ij = -1;
				int i_old = -1;
				//寻找最高投标人Ij
				for (h = 0; h < RNUM; h++)
				{
					if (tmps1[h] == j)
					{
						if (bid[h] > temp1)
						{
							temp0 = h;
							temp1 = bid[h];   //如果当前投标价格大于同一投标的其他人，记录
						}
					}
				}
				Ij = temp0;//Ij记录投标价最高的那个人

						   //如果原来任务j有分配，要去掉分配
				if (s2[j] >= 0)
				{
					i_old = s2[j];
					s1[i_old] = -1;
				}

				//更新对应关系数组，以及价格数组
				if (Ij >= 0)
				{
					s2[j] = Ij;
					s1[Ij] = j;
					p[j] = temp1;
				}
			}

			s10 = find(s1, TNUM, -1);


		}
	}

	if (RNUM < TNUM)
	{
		while (s10)
		{
			//bidding phase

			double bid[TNUM] = { 0 };     //初始化竞价bid数组。
			int tmps1[RNUM];              //暂时记录机器人投标的物
			for (i = 1; i < RNUM; i++)
			{
				tmps1[i] = -1;
			}

			for (i = 0; i < RNUM; i++)
			{
				if (s1[i] == -1)  //未进行分配任务的机器人i选中
				{
					vi = -3 * TNUM*maxabs;
					wi = -5 * TNUM*maxabs;

					//每一个机器人对应的任务，计算最大利润vi和次大利润wi
					for (j = 0; (array[i][j] != -1) && (j < TNUM); j++)
					{
						vij = array[i][j] - p[j];
						if (vij > wi)
							if (vij > vi)
							{
								J = j;
								wi = vi;
								vi = vij;
							}
							else
								wi = vij;
					}
					//vi大于零，找到了最大利润，进行投标价的计算
					if (vi >= 0)
					{
						wi = wi > 0 ? wi : 0;

						tmps1[i] = J;
						bid[i] = p[J] + (double)vi - (double)wi + eplison;

					}
					//vi小于零，未找到最大利润，将其分配设为-10，分配不了
					else if (vi < 0)
						s1[i] = -10;



				}
			}

			//assginment phase
			for (j = 0; j < TNUM; j++)
			{
				int temp0 = -1; //元素记录投标人
				double temp1 = -1;//记录投标价格
				int Ij = -1;
				int i_old = -1;
				//寻找最高投标人Ij
				for (h = 0; h < RNUM; h++)
				{
					if (tmps1[h] == j)
					{
						if (bid[h] > temp1)
						{
							temp0 = h;
							temp1 = bid[h];   //如果当前投标价格大于同一投标的其他人，记录
						}
					}
				}
				Ij = temp0;//Ij记录投标价最高的那个人

						   //如果原来任务j有分配，要去掉分配
				if (s2[j] >= 0)
				{
					i_old = s2[j];
					s1[i_old] = -1;
				}

				//更新对应关系数组，以及价格数组
				if (Ij >= 0)
				{
					s2[j] = Ij;
					s1[Ij] = j;
					p[j] = temp1;
				}
			}

			s10 = find(s1, TNUM, -1);
		}

		for (i = 0; i < RNUM; i++)
			if (s1[i] != -1 && s1[i] != -10)
			{
				k = s1[i];
				q[i] = (double)array[i][k] - p[k];  //用q记录机器人分配到物后所获得利润
			}

		//逆向拍卖
		//void reverse_auction()

		lamda = 5 * TNUM*max(p, TNUM);

		for (k = 0; k < TNUM; k++)
		{
			if ((s2[k] != -1) && (lamda>p[k]))
				lamda = p[k];
		}
		printf("the min price is %lf\n", lamda);
		//找到lamda为已经分配的任务中的最小价格


		int CS_cond = 1;
		for (k = 0; (k < TNUM) && (s2[k] == -1); k++)
		{
			if (p[k] > lamda)
				CS_cond = 0;
		}
		while (CS_cond == 0)
		{


			int Ij;
			int i_old;
			for (j = 0; j < TNUM; j++)
			{
				if ((s2[j] == -1) && (p[j] > lamda))
				{
					vj = -3 * TNUM*maxabs;
					wj = -5 * TNUM*maxabs;
					for (k = 0; (k < RNUM) && (array[k][j] != -1); k++)
					{
						pji = (double)array[k][j] - q[k];
						if (pji > wj)
							if (pji > vj)
							{
								Ij = k;
								wj = vj;
								vj = pji;
							}
							else
								wi = pji;
					}
					if (lamda >= (vj - eplison))
						p[j] = lamda;
					else if (lamda < (vj - eplison))
					{
						delta = (vj - lamda) >(vj - wj + eplison) ? (vj - wj + eplison) : (vj - lamda);
						p[j] = vj - delta;
						q[Ij] = q[Ij] + delta;
					}
					if (s2[j] > 0)
					{
						i_old = s2[j];
						s1[i_old] = -1;
					}
					s1[Ij] = j;
					s2[j] = Ij;

				}
			}

			lamda = 5 * TNUM*max(p, TNUM);

			for (k = 0; k < TNUM; k++)
			{
				if ((s2[k] != -1) && (lamda>p[k]))
					lamda = p[k];
			}

			CS_cond = 1;
			for (k = 0; k < TNUM; k++)
			{
				if ((s2[k] = -1) && p[k] > lamda)
					CS_cond = 0;
			}
		}
	}

	printf("************  s1 *************\n\n");
	for (i = 0; i<RNUM; i++)
		printf("%d\t", s1[i] + 1);
	printf("\n");
	//生成分配矩阵，为1的可以分配
	int X[RNUM][TNUM] = { 0 };
	printf("** The Allocation Matrix ***\n\n");

	for (i = 0; i < RNUM; i++)
	{
		if (s1[i] >= 0)
			X[i][(s1[i])] = 1;
	}
	for (i = 0; i < RNUM; i++)
	{
		for (j = 0; j <TNUM; j++)
		{
			printf("%d\t", X[i][j]);
		}
		printf("\n");
	}

	printf("**** The Total Cost ****\n\n");

	//计算最大化的总收益tcost
	int tcost = 0;
	for (i = 0; i < TNUM; i++)
		for (j = 0; j < RNUM; j++)
			if (X[i][j] == 1)
				tcost = tcost + array[i][j];
	printf("The total cost is %d", tcost);
}