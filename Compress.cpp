#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <iomanip>
#include <Windows.h>
#include <conio.h>
#define FLOW_SPACE 4096 // �������ڴ�С
#define FRONT_SPACE 32 // ��ǰ��������С
using namespace std;
struct lz77 {
	short o;
	short l;
	char c;
};

bool compare(char* front, char* flow, short& o, short& l)
{
	int i, j;
	int max = 0; // �������ƥ�䳤��
	int tmp = 0; // �����ݴ泤��
	int offset = 0; // ƫ����
	for (i = 0; i < FLOW_SPACE; i++)
	{
		if (front[0] == flow[i])
		{ // ���ҵ����ַ���ͬ
			tmp = 0;
			for (j = 0; j < FRONT_SPACE && i + j < FLOW_SPACE; j++, tmp++)
			{
				if (front[j] != flow[i + j])
					break; // ���ҵ���ͬ�ַ����˳�ѭ��
			}
			//��ʱ�ѻ�ȡ����������λ��i��ƥ�����󳤶�tmp
			if (tmp > max)
			{
				max = tmp; // ���ҵ��˸����ƥ�䳤�ȣ�����Ϊmax
				offset = i;
			}
		}
	}
	o = offset;
	if (max > FRONT_SPACE - 2)
		max = 30; // ƥ�䳤�Ȳ��ܳ���30
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
	// ��ǰ��������ȫ����������sum�ֽ�

	/*�Ӻ�����������������ݽ�front*/
	for (i = 0; i < sum; i++)
		if (filelength - 32 == 0) // ֤��p���Ѿ�û�����ݿ����ṩ��front
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
	// flowԭ�в�������Ųlengthλ
	for (i = 0; i < length; i++)
		flow[FLOW_SPACE - length + i] = front[i];
	// ��front�����length����front����ʣ������compress���Ѿ��ж���������length+1��
}
int compress(char* p, char* result, int filelength, char* file_end)
{
	int result_length = 0; // ����ļ�����
	double static_filelength = filelength;
	int i, tmp = filelength / 37, times = 0;
	struct lz77 sequence = {
		0,
		0,
		'\0'
	};
	char* flow = new(nothrow)char[FLOW_SPACE] { '\0' };// ���û������ڴ�С
	if (flow == NULL)
	{
		cerr << "No Memory" << endl;
		exit(EXIT_FAILURE);
	}
	CONSOLE_CURSOR_INFO cursor_info_mode1 = { 1,0 };
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info_mode1);
	char front[FRONT_SPACE] = { '\0' }; // ������ǰ������
	// ��һ����ʼ����ǰ������
	for (i = 0; i < FRONT_SPACE && i < filelength; i++, p++)
		front[i] = *p;
	// ��ǰ�����������ϣ�pָ�����׼��������ַ�
	system("cls");
	cerr << "Begin Compressing" << endl;
	while (filelength > 0)
	{
		if ((static_filelength - filelength) / tmp > times)
		{
			times++;
			COORD position = { 0,1 };
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), position);
			cerr << "�����:";
			cerr << setiosflags(ios::fixed)
				<< setprecision(2)
				<< (1 - filelength / static_filelength) * 100
				<< "%" << endl;
		}
		if (compare(front, flow, sequence.o, sequence.l) && filelength - sequence.l > 0)
		{ // ƥ��ɹ�ʱִ�����²���     /*�˴���һ�������Ż��ռ䣬����һ����������*/
			SlipToFlow(flow, front, sequence.l + 1); // ��������ݵ���������
			sequence.c = SlipToFront(front, sequence.l + 1, p, filelength); // ��ȡ�ַ�
			CompareWirteToResult(sequence.o, sequence.l, sequence.c, result); // д�����ַ���
			result += 4; // ����λ�Ѿ�д������
			result_length += 4;
			filelength -= (sequence.l + 1);
		}
		else
		{ // ƥ��ʧ��ʱִ�����²���
			SlipToFlow(flow, front, 1);
			CharWriteToResult(front[0], result); // ����result��
			SlipToFront(front, 1, p, filelength); // ��ǰ��������Ųһλ
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
	// flowԭ�в�������Ųstepλ
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
			cerr << "�����:";
			cerr << setiosflags(ios::fixed)
				<< setprecision(2)
				<< (1 - filelength / static_filelength) * 100
				<< "%" << endl;
		}
		if (*p < 0)
		{ // ��bitΪ1,����������4�ֽ�ofc��ȡ
			GetOLC(sequence, p); // ��ȡOLC
			for (i = 0; i < sequence.l; i++, result++)
			{
				*result = flow[sequence.o + i];
				result_length++;
			}
			*result = sequence.c;
			result++;
			result_length++;
			// �ýṹд�뵽�ļ���
			MoveFlow(flow, sequence.l + 1);
			FillFlow(flow, result - i - 1, sequence.l + 1);
			filelength -= 4;
		}
		else
		{ // ��bitΪ0������������2�ֽ�char��ȡ
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
	cerr << "Compress or Uncompress now! Auther:Root" << endl; // cmd�����ʾ��
	if (argc != 4) {
		cerr << "Please make sure the number of parameters is correct." << endl;
		return -1;
	}
	// �������������ֱ���˳�0
	if (strcmp(argv[3], "zip") && strcmp(argv[3], "unzip"))
	{ // ��û������zip��unzip����
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
	int filelength = ftell(pfile); // ��ȡ�ļ���С�����ڶ�̬����
	fclose(pfile);
	ifstream fin(argv[1], ios::binary);
	if (!fin.good())
	{
		cerr << "file doesn't exist!" << endl;
		return -1;
	}
	// �ж��Ƿ���ȷ���ļ�
	char file_end[13] = { '\0' }; // �ļ���ĩβ
	ofstream fout(argv[2], ios::binary); // ������ļ�
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
	// �ر��ļ�
	/*���Ƚ��ļ����ݵ����ʮ�����ֽڵ���ȡ��*/
	int i, static_filelength = filelength;
	for (i = 0; i < 12 && static_filelength > 0; i++)
	{
		file_end[i] = p[filelength - 12 + i];
		p[filelength - 12 + i] = '\0';
		static_filelength--;
	}
	// �޸ĺ��ļ��������ʮ�����ֽ���file_end�У�p�����ݼ���12��
	int result_length = 0;
	char reaction = '\0';
	cerr << "�س��Կ�ʼ!";
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
		if (i == 12) // �ļ�����12���ַ�������ѹ��
		{
			time(&past);
			filelength -= 12;
			result_length = compress(p, result, filelength, file_end);
			output(fout, result, result_length);
			time(&now);
			system("cls");
			cerr << "��ʱ:" << now - past << "s" << endl;
			cerr << "ѹ����:"
				<< setiosflags(ios::fixed)
				<< setprecision(2)
				<< result_length / double(filelength + 12) * 100 << "%" << endl;
			cerr << "Complete!" << endl;
		}
		else // �ļ�������Ҫѹ��
			fout << p;
		fout.close();
		delete[]result;
	}
	else
	{ // ִ�н�ѹ��
		time_t now, past;
		if (i == 12)
		{
			time(&past);
			filelength -= 12;
			uncompress(p, fout, file_end, filelength);
			time(&now);
			system("cls");
			cerr << "��ʱ:" << now - past << "s" << endl;
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