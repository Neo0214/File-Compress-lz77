#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <iomanip>
#include <Windows.h>
#include <conio.h>
#define FLOW_SPACE 4096 // 滑动窗口大小
#define FRONT_SPACE 32 // 向前缓冲区大小
using namespace std;
struct lz77 {
	short o;
	short l;
	char c;
};

bool compare(char* front, char* flow, short& o, short& l)
{
	int i, j;
	int max = 0; // 定义最大匹配长度
	int tmp = 0; // 定义暂存长度
	int offset = 0; // 偏移量
	for (i = 0; i < FLOW_SPACE; i++)
	{
		if (front[0] == flow[i])
		{ // 查找到首字符相同
			tmp = 0;
			for (j = 0; j < FRONT_SPACE && i + j < FLOW_SPACE; j++, tmp++)
			{
				if (front[j] != flow[i + j])
					break; // 查找到不同字符，退出循环
			}
			//此时已获取滑动窗口中位置i可匹配的最大长度tmp
			if (tmp > max)
			{
				max = tmp; // 若找到了更大可匹配长度，保存为max
				offset = i;
			}
		}
	}
	o = offset;
	if (max > FRONT_SPACE - 2)
		max = 30; // 匹配长度不能超过30
	l = max;
	if (max >= 2)
		return true;
	return false;
}
char SlipToFront(char front[FRONT_SPACE], int sum, char*& p, int filelength)
{
	int i;
	char rechar = front[sum - 1];
	for (i = sum; i < FRONT_SPACE; i++)
		front[i - sum] = front[i];
	// 向前缓冲区内全部数据左移sum字节

	/*从后续内容中填充新内容进front*/
	for (i = 0; i < sum; i++)
		if (filelength - 32 == 0) // 证明p里已经没有内容可以提供给front
			front[FRONT_SPACE - sum + i] = '\0';
		else
		{
			front[FRONT_SPACE - sum + i] = *p;
			p++;
		}
	return rechar;
}
void CompareWirteToResult(short o, short l, char c, char* result)
{
	*result = -128;
	*result = *result | (o >> 5);
	result++;
	*result = (o << 3);
	result++;
	*result = char(l);
	result++;
	*result = c;
}
void CharWriteToResult(char ch, char* result)
{
	*result = 0;
	result++;
	*result = ch;
}
void SlipToFlow(char*& flow, char* front, int length)
{
	int i;
	for (i = 0; i < FLOW_SPACE - length; i++)
		flow[i] = flow[i + length];
	// flow原有部分向左挪length位
	for (i = 0; i < length; i++)
		flow[FLOW_SPACE - length + i] = front[i];
	// 从front里放入length个，front里所剩内容在compress里已经判定过至少有length+1个
}
int compress(char* p, char* result, int filelength, char* file_end)
{
	int result_length = 0; // 结果文件长度
	double static_filelength = filelength;
	int i, tmp = filelength / 37, times = 0;
	struct lz77 sequence = {
		0,
		0,
		'\0'
	};
	char* flow = new(nothrow)char[FLOW_SPACE] { '\0' };// 设置滑动窗口大小
	if (flow == NULL)
	{
		cerr << "No Memory" << endl;
		exit(EXIT_FAILURE);
	}
	CONSOLE_CURSOR_INFO cursor_info_mode1 = { 1,0 };
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info_mode1);
	char front[FRONT_SPACE] = { '\0' }; // 设置向前缓冲区
	// 第一步初始化向前缓冲区
	for (i = 0; i < FRONT_SPACE && i < filelength; i++, p++)
		front[i] = *p;
	// 向前缓冲区填充完毕，p指向后续准备填入的字符
	system("cls");
	cerr << "Begin Compressing" << endl;
	while (filelength > 0)
	{
		if ((static_filelength - filelength) / tmp > times)
		{
			times++;
			COORD position = { 0,1 };
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), position);
			cerr << "已完成:";
			cerr << setiosflags(ios::fixed)
				<< setprecision(2)
				<< (1 - filelength / static_filelength) * 100
				<< "%" << endl;
		}
		if (compare(front, flow, sequence.o, sequence.l) && filelength - sequence.l > 0)
		{ // 匹配成功时执行以下操作     /*此处后一条件有优化空间，缩短一个长度再判*/
			SlipToFlow(flow, front, sequence.l + 1); // 填充新内容到滑动窗口
			sequence.c = SlipToFront(front, sequence.l + 1, p, filelength); // 获取字符
			CompareWirteToResult(sequence.o, sequence.l, sequence.c, result); // 写入结果字符串
			result += 4; // 这四位已经写入内容
			result_length += 4;
			filelength -= (sequence.l + 1);
		}
		else
		{ // 匹配失败时执行以下操作
			SlipToFlow(flow, front, 1);
			CharWriteToResult(front[0], result); // 放入result里
			SlipToFront(front, 1, p, filelength); // 向前缓冲区左挪一位
			result += 2;
			result_length += 2;
			filelength--;
		}
	}
	for (i = 0; i < 12; i++)
		result[i] = file_end[i];
	delete[]flow;
	return result_length + 12;
}
void output(ofstream& fout, char* result, int filelength)
{
	fout.write(result, filelength);
}
void GetOLC(struct lz77& sequence, char*& p)
{
	sequence.o = 0;
	sequence.o = sequence.o + ((*p & 127) << 5);
	p++;
	sequence.o = sequence.o + ((*p >> 3) & 31);
	p++;
	sequence.l = short(*p);
	p++;
	sequence.c = *p;
	p++;
}
void MoveFlow(char* flow, short step)
{
	int i;
	for (i = 0; i < FLOW_SPACE - step; i++)
		flow[i] = flow[i + step];
	// flow原有部分向左挪step位
}
void FillFlow(char* flow, char* writein, short length)
{
	int i;
	for (i = 0; i < length; i++)
		flow[FLOW_SPACE - length + i] = writein[i];
}
void uncompress(char* p, ofstream& fout, char* file_end, int filelength)
{
	CONSOLE_CURSOR_INFO cursor_info_mode1 = { 1,0 };
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info_mode1);
	char* result = new(nothrow) char[filelength * 10]{ '\0' };
	if (result == NULL)
	{
		cerr << "No Memory" << endl;
		exit(EXIT_FAILURE);
	}
	char* head_result = result;
	int result_length = 0;
	double static_filelength = filelength;
	char* flow = new(nothrow)char[FLOW_SPACE] {'\0'};
	if (flow == NULL)
	{
		cerr << "No Memory" << endl;
		exit(EXIT_FAILURE);
	}
	int i, tmp = filelength / 37, times = 0;
	char ch = '\0';
	struct lz77 sequence {
		0,
			0,
			'0'
	};
	system("cls");
	cerr << "Begin Uncompressing" << endl;
	while (filelength > 0)
	{
		if ((static_filelength - filelength) / tmp > times)
		{
			times++;
			COORD position = { 0,1 };
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), position);
			cerr << "已完成:";
			cerr << setiosflags(ios::fixed)
				<< setprecision(2)
				<< (1 - filelength / static_filelength) * 100
				<< "%" << endl;
		}
		if (*p < 0)
		{ // 首bit为1,接下来按照4字节ofc读取
			GetOLC(sequence, p); // 获取OLC
			for (i = 0; i < sequence.l; i++, result++)
			{
				*result = flow[sequence.o + i];
				result_length++;
			}
			*result = sequence.c;
			result++;
			result_length++;
			// 该结构写入到文件内
			MoveFlow(flow, sequence.l + 1);
			FillFlow(flow, result - i - 1, sequence.l + 1);
			filelength -= 4;
		}
		else
		{ // 首bit为0，接下来按照2字节char读取
			p++;
			ch = *p;
			p++;
			MoveFlow(flow, 1);
			flow[FLOW_SPACE - 1] = ch;
			*result = ch;
			result++;
			result_length++;
			filelength -= 2;
		}
	}
	fout.write(head_result, result_length);
	fout.write(file_end, 12);
	delete[]flow;
	delete[]head_result;
}

int main(int argc, char* argv[])
{
	cerr << "Compress or Uncompress now! Auther:Root" << endl; // cmd输出提示句
	if (argc != 4) {
		cerr << "Please make sure the number of parameters is correct." << endl;
		return -1;
	}
	// 命令行输入错误，直接退出0
	if (strcmp(argv[3], "zip") && strcmp(argv[3], "unzip"))
	{ // 若没有输入zip或unzip操作
		cerr << "Unknown parameter!\nCommand list:\nzip\nunzip" << endl;
		return -1;
	}
	FILE* pfile = fopen(argv[1], "r");
	if (pfile == NULL)
	{
		cerr << "file doesn't exist!" << endl;
		return -1;
	}
	fseek(pfile, 0L, SEEK_END);
	int filelength = ftell(pfile); // 获取文件大小，用于动态申请
	fclose(pfile);
	ifstream fin(argv[1], ios::binary);
	if (!fin.good())
	{
		cerr << "file doesn't exist!" << endl;
		return -1;
	}
	// 判断是否正确打开文件
	char file_end[13] = { '\0' }; // 文件的末尾
	ofstream fout(argv[2], ios::binary); // 打开输出文件
	if (!fout) {
		cerr << "Can not open the output file!" << endl;
		return -1;
	}
	char* p = new(nothrow) char[filelength + 1]{ '\0' };
	if (p == NULL)
	{
		cerr << "No Memory" << endl;
		exit(EXIT_FAILURE);
	}
	fin.seekg(0, ios::beg);
	fin.read(p, filelength);
	fin.close();
	// 关闭文件
	/*首先将文件内容的最后十二个字节单独取出*/
	int i, static_filelength = filelength;
	for (i = 0; i < 12 && static_filelength > 0; i++)
	{
		file_end[i] = p[filelength - 12 + i];
		p[filelength - 12 + i] = '\0';
		static_filelength--;
	}
	// 修改后，文件内容最后十二个字节在file_end中，p内内容减少12个
	int result_length = 0;
	char reaction = '\0';
	cerr << "回车以开始!";
	while (true)
	{
		reaction = _getch();
		if (reaction == '\r')
			break;
	}

	if (!strcmp(argv[3], "zip"))
	{
		char* result = new(nothrow) char[filelength + filelength + 1]{ '\0' };
		if (result == NULL)
		{
			cerr << "No Memory" << endl;
			exit(EXIT_FAILURE);
		}
		time_t now, past;
		if (i == 12) // 文件大于12个字符，可以压缩
		{
			time(&past);
			filelength -= 12;
			result_length = compress(p, result, filelength, file_end);
			output(fout, result, result_length);
			time(&now);
			system("cls");
			cerr << "耗时:" << now - past << "s" << endl;
			cerr << "压缩比:"
				<< setiosflags(ios::fixed)
				<< setprecision(2)
				<< result_length / double(filelength + 12) * 100 << "%" << endl;
			cerr << "Complete!" << endl;
		}
		else // 文件并不需要压缩
			fout << p;
		fout.close();
		delete[]result;
	}
	else
	{ // 执行解压缩
		time_t now, past;
		if (i == 12)
		{
			time(&past);
			filelength -= 12;
			uncompress(p, fout, file_end, filelength);
			time(&now);
			system("cls");
			cerr << "耗时:" << now - past << "s" << endl;
			cerr << "Complete!" << endl;
		}
		else
			fout << file_end;
		fout.close();

	}
	delete[]p;
	CONSOLE_CURSOR_INFO cursor_info_mode2 = { 25,1 };
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info_mode2);
	return 0;
}