#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <string>
#include <array>
#include <vector>
#include <signal.h>
#include "isolation_forest1.h"
#include "readdata.h"

using namespace std;

float cores = 1;

void sig_handler(int num)
{
	if(num == SIGINT)
		printf("\n end\n");
	exit(1);
}



int main(int argc,char **argv)
{
	
	
	int flag = 0;
	int i;
	char disk_file[300];
	char net_file[300];
	char cpu_file[300];
	char mem_file1[300];
	char mem_file2[300];
	unsigned long int cpuused,pre_cpuused,cputotal,pre_cputotal;
	unsigned long int memused,memlimit;
	unsigned long int diskread,pre_diskread;
	unsigned long int diskwrite,pre_diskwrite;
	unsigned long int netrx,pre_netrx;
	unsigned long int nettx,pre_nettx;
	float CpuLoadAverage;
	float MemoryLoadAverage;
	float DiskReadAverage;
	float DiskWriteAverage;
	float NetRxAverage;
	float NetTxAverage;
	std::vector<std::array<float, 6>> data;
	sprintf(cpu_file,"/sys/fs/cgroup/cpuacct/docker/%s/cpuacct.usage",argv[1]);
	sprintf(mem_file1,"/sys/fs/cgroup/memory/docker/%s/memory.usage_in_bytes",argv[1]);
	sprintf(mem_file2,"/sys/fs/cgroup/memory/docker/%s/memory.limit_in_bytes",argv[1]);
	sprintf(disk_file,"/sys/fs/cgroup/blkio/docker/%s/blkio.throttle.io_service_bytes",argv[1]);
	sprintf(net_file,"/proc/%s/net/dev",argv[2]);
	
	float data_sum;//用来判断权重是否为0
	float weight_count;//用来计算权重
	std::mt19937 rng(15345);		//随机值种子
	float w[6] = {1,1,1,1,1,1};  //权值的设定，依次为cpu，内存，网络接收，网络发送，磁盘读，磁盘写 度量的权重。初始都为1
	float threshold[6] = {50,50,5,5,5,5};  //权重计算时的对比阈值
	std::vector<float> weight(w,w+6);  //度量权重数组
	iforest::IsolationForest<float, 6> forest;
	std::vector<double> anomaly_scores;  //异常指数
	std::vector<std::vector<int>> anomaly_features(100,vector<int>(6,0));   //孤立特征
	
	time_t now;
	struct tm *timenow;
	
	float anomalyIndex;//10组数据异常值之和
	float features1[6];   //前90个数据的孤立特征平均值
	float features2[6];   //后10个数据的孤立特征平均值
	float features_ratio[6];   //度量的孤立特征比值
	
	float period_init = 4;
	float period = 4;
    float period_new = 4;
	
	int anomaly_type;
	
	signal(SIGINT,sig_handler);
	
	for(i = 0;i < 101;i++)
	{
		std::array<float, 6> temp;
		if(read_disk(disk_file,diskread,diskwrite) == -1)
		    return -1;
	    if(read_net(net_file,netrx,nettx) == -1)
		    return -1;
	    if(read_cpu(cpu_file,cpuused,cputotal) == -1)
		    return -1;
	    if(read_mem(mem_file1,mem_file2,memused,memlimit) == -1)
		    return -1;
	    if(flag)
	    {
		    CpuLoadAverage = (float)(cpuused - pre_cpuused) / 10000000 / (float)(cputotal - pre_cputotal) * 100 * cores;
		    MemoryLoadAverage = (float)memused / (float)memlimit *100;
		    DiskReadAverage = (float)(diskread - pre_diskread) / (1024 * 1024 * period);
		    DiskWriteAverage = (float)(diskwrite - pre_diskwrite) / (1024 * 1024 * period);
		    NetRxAverage = (float)(netrx - pre_netrx) / (1000 * 1000 * period);
		    NetTxAverage = (float)(nettx - pre_nettx) / (1000 * 1000 * period);
			
			temp[0] = CpuLoadAverage;
			temp[1] = MemoryLoadAverage;
			temp[2] = NetRxAverage;
			temp[3] = NetTxAverage;
			temp[4] = DiskReadAverage;
			temp[5] = DiskWriteAverage;
			data.push_back(temp);
			time(&now); 
			timenow = localtime(&now); 
			cout << "time is " << asctime(timenow) <<endl;
		
		    cout << argv[2] << ":cpu:" << CpuLoadAverage << " mem:" << MemoryLoadAverage  << " netrx:" << NetRxAverage 
			<< " nettx:" << NetTxAverage  << " diskread:" << DiskReadAverage<< " diskwrite:" << DiskWriteAverage <<endl;
	    }
		else flag = 1;
	    pre_cpuused = cpuused;
	    pre_cputotal = cputotal;
	    pre_diskread = diskread;
	    pre_diskwrite = diskwrite;
	    pre_netrx = netrx;
	    pre_nettx = nettx;
		sleep(period);
	}
	
	while(1)
	{
		for(i = 0;i <10;i++)
		{
			std::array<float, 6> temp;
			if(read_disk(disk_file,diskread,diskwrite) == -1)
				return -1;
			if(read_net(net_file,netrx,nettx) == -1)
				return -1;
			if(read_cpu(cpu_file,cpuused,cputotal) == -1)
				return -1;
			if(read_mem(mem_file1,mem_file2,memused,memlimit) == -1)
				return -1;
			
			CpuLoadAverage = (float)(cpuused - pre_cpuused) / 10000000 / (float)(cputotal - pre_cputotal) * 100 * cores;
			MemoryLoadAverage = (float)memused / (float)memlimit *100;
			DiskReadAverage = (float)(diskread - pre_diskread) / (1024 * 1024 * period);
			DiskWriteAverage = (float)(diskwrite - pre_diskwrite) / (1024 * 1024 * period);
			NetRxAverage = (float)(netrx - pre_netrx) / (1000 * 1000 * period);
			NetTxAverage = (float)(nettx - pre_nettx) / (1000 * 1000 * period);
				
			temp[0] = CpuLoadAverage;
			temp[1] = MemoryLoadAverage;
			temp[2] = NetRxAverage;
			temp[3] = NetTxAverage;
			temp[4] = DiskReadAverage;
			temp[5] = DiskWriteAverage;
			
			data.erase(data.begin());
			data.push_back(temp);
			time(&now); 
			timenow = localtime(&now); 
			cout << "time is " << asctime(timenow) <<endl;
			
			cout << argv[2] << ":cpu:" << CpuLoadAverage << " mem:" << MemoryLoadAverage  << " netrx:" << NetRxAverage 
			<< " nettx:" << NetTxAverage  << " diskread:" << DiskReadAverage<< " diskwrite:" << DiskWriteAverage <<endl;
			pre_cpuused = cpuused;
			pre_cputotal = cputotal;
			pre_diskread = diskread;
			pre_diskwrite = diskwrite;
			pre_netrx = netrx;
			pre_nettx = nettx;
			
			period = period_new;
			sleep(period);
		}
		
		for(int j = 0;j<6;j++)
		{
			data_sum = 0;
			weight_count = 0; 
			for(int i = 0;i<100;i++){
				data_sum +=data[i][j];
				if(data[i][j]>=threshold[j])//度量大于权重对比阈值，weight_count加1
					weight_count++;
			}
			if(data_sum == 0)//相关度量一直为0，权值设置为0
				weight[j] = 0;
			else//度量不为0，计算权重
				weight[j] = 1 + weight_count/100;
		}
		cout << endl;
		cout << "==================================================" << endl;
		cout << "weights are:" << weight[0] << "," << weight[1] << "," << weight[2] << "," << weight[3] << "," << weight[4] << "," << weight[5] <<endl;
		if (!forest.Build(100, 12345, data, 100, weight))  //第一个参数是树的个数，第2个是随机数生成器的种子，第三个是数据集，第4个是取样数
		{
			cout << "Failed to build Isolation Forest.\n";
			return -1;
		}
		if (!forest.GetAnomalyScores(data, anomaly_scores, anomaly_features))//获取异常指数和孤立特征组
		{
			cout << "Failed to calculate anomaly scores.\n";
			return -1;
		}
		
		anomalyIndex = 0;
		for (int i = 90; i < 100; ++i)   //计算后10个数据的异常值之和
		{
			anomalyIndex +=anomaly_scores[i];
			//输出监测数据值，异常指数和孤立特征统计
			cout<<data[i][0]<<" , "<<data[i][1]<<" , "<<data[i][2]<<" , "<<data[i][3]<<" , "<<data[i][4]<<" , "<<data[i][5]<<", scores:"<<anomaly_scores[i]<<endl;
			cout<<"features: "<<anomaly_features[i][0]<<" , "<<anomaly_features[i][1]<<" , "<<anomaly_features[i][2]<<" , "<<anomaly_features[i][3]<<" , "<<anomaly_features[i][4]<<" , "<<anomaly_features[i][5]<<endl;
		}
		cout<< "anomalyIndex:"  <<anomalyIndex<<endl;
		
		memset(features1,0,6*sizeof(float));
		memset(features2,0,6*sizeof(float));
		for(int j = 0;j < 6;j++)
		{
			for(int i = 0;i<90;i++)
			{
				features1[j]+=anomaly_features[i][j];
			}
			features1[j] = features1[j]/90;
			for(int i = 90;i<100;i++)
			{
				features2[j]+=anomaly_features[i][j];
			}
			features2[j] = features2[j]/10;
			if(features1[j]==0)
				features_ratio[j] = 0;
			else
				features_ratio[j] = features2[j]/features1[j];//异常度量比值
		}
		
		anomaly_type = max_feature(features_ratio);
				
		if(anomalyIndex > 5.4)//5.4为异常阈值
		{
			switch(anomaly_type)
			{
				case 0: cout << "CpuLoad Anomaly" <<endl;break;
				case 1: cout << "MemoryLoad Anomaly" <<endl;break;
				case 2: cout << "NetRx Anomaly" <<endl;break;
				case 3: cout << "NetTx Anomaly" <<endl;break;
				case 4: cout << "DiskRead Anomaly" <<endl;break;
				case 5: cout << "DiskWrite Anomaly" <<endl;break;
			}
		}
		else if(anomalyIndex > 5.2)//5.2为异常敏感阈值
		{
			period_new = period_init / 2;//周期调整
		}
		else
			period_new = period_init;
		
		cout << "==================================================" << endl <<endl;
		
	}
	return 0;
}

