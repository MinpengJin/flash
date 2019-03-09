#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

int read_disk(char *diskfile,unsigned long int &disk_read,unsigned long int &disk_write)
{
	FILE *fp;
	char buf1[20],buf2[20];
	fp = fopen(diskfile,"r");
	if(!fp)
	{
		printf("disk error1!\n");
		return -1;
	}
	if(fscanf(fp,"%[0-9:] %[a-zA-Z] %lu\n",buf1,buf2,&disk_read)==EOF)//???????8:0 Resd 3350528
	{
		printf("disk error2!\n");
		return -1;
	}
	if(fscanf(fp,"%[0-9:] %[a-zA-Z] %lu\n",buf1,buf2,&disk_write)==EOF)//???????8:0 Write 0
	{
		printf("disk error3!\n");
		return -1;
	}
	fclose(fp);
	return 0;
}

int read_net(char *netfile,unsigned long int &net_rcv,unsigned long int &net_snd)
{
	FILE *fp;
	unsigned long int t;
	char buf1[1024],buf2[20];
	fp = fopen(netfile,"r");
	if(!fp)
	{
		printf("net error1!\n");
		return -1;
	}
	for(int i = 0; i < 3;i++)//????????§Ò????????????eth0??????????????loopback??????????§Ó??????????§Ø???????????????
	{
		if(fgets(buf1,1024,fp)==NULL)
		{
			printf("net error2!\n");
			return -1;
		}
	}
	if(fscanf(fp,"%s",buf2)==EOF)//????????
	{
		printf("net error3!\n");
		return -1;
	}
	if(fscanf(fp,"%lu",&net_rcv)==EOF)//????bytes
	{
		printf("net error4!\n");
		return -1;
	}
	for(int i = 0;i < 7;i++)//?????packets??receive???
	{
		if(fscanf(fp,"%lu",&net_rcv)==EOF)
		{
		    printf("net error5!\n");
		    return -1;
		}
	}
	if(fscanf(fp,"%lu",&net_snd)==EOF)//????bytes
	{
		printf("net error6!\n");
		return -1;
	}
	fclose(fp);
	return 0;
}

int read_cpu(char *cpufile,unsigned long int &cpu_container,unsigned long int &cpu_linux)
{
	FILE *fp;
	cpu_linux = 0;
	unsigned long int t[9];
	char buf[20];
	fp = fopen(cpufile,"r");
	if(!fp)
	{
		printf("cpu error1!\n");
		return -1;
	}
	if(fscanf(fp,"%lu",&cpu_container)==EOF)//??????§ß???????????
	{
		printf("cpu error2!\n");
		return -1;
	}
	fclose(fp);
	fp = fopen("/proc/stat","r");
	if(!fp)
	{
		printf("cpu error3!\n");
		return -1;
	}
	if(fscanf(fp,"%s",buf)==EOF)//????§Þ????????????????????cpu
	{
		printf("cpu error4!\n");
		return -1;
	}
	for(int i = 0;i < 10;i++)//?????????????????7???user,nice,system,idle,iowait,irq,softirq
	{
		if(fscanf(fp,"%lu",&t[i])==EOF)
		{
			printf("cpu error5!\n");
		    return -1;
		}
	}
	for(int i = 0;i < 10;i++)
	{
		cpu_linux += t[i];
	}
	fclose(fp);
	return 0;
}

int read_mem(char *memfile1,char *memfile2,unsigned long int &mem_used,unsigned long int &mem_limit)
{
	FILE *fp;
	fp = fopen(memfile1,"r");
	if(!fp)
	{
		printf("mem1 error1!\n");
		return -1;
	}
	if(fscanf(fp,"%lu",&mem_used)==EOF)
	{
		printf("mem1 error2!\n");
		return -1;
	}
	fclose(fp);

	fp = fopen(memfile2,"r");
	if(!fp)
	{
		printf("mem2 error1!\n");
		return -1;
	}
	if(fscanf(fp,"%lu",&mem_limit)==EOF)//???????????????
	{
		printf("mem2 error2!\n");
		return -1;
	}
	fclose(fp);
	return 0;
}

int max_feature(float features_ratio[6])
{ 
	int max = 0;
	float temp = 0;
	for(int i = 0;i < 6;i++)
	{
		if(features_ratio[i] > temp)
		{
			temp = features_ratio[i];
			max = i;
		}
	}
	return max;
}
