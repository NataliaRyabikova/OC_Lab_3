#include <iostream>
#include <Windows.h>
using namespace std;
CRITICAL_SECTION cs;
int* arr;

struct params
{
	int i;
	int n;
	HANDLE hEndEvent;
	HANDLE hStartEvent;
	HANDLE hCantEvent;
};

DWORD WINAPI marker(LPVOID lpParam)
{
	params* p = (params*)lpParam;
	int num = 0;
	srand(p->i);
	HANDLE hEvents[2] = { p->hStartEvent,p->hEndEvent };
	while (WaitForMultipleObjects(2, hEvents, FALSE, INFINITE) != WAIT_OBJECT_0 + 1)
	{
		while (true) {
			int r = rand();
			int mod = r % p->n;
			EnterCriticalSection(&cs);
			if (arr[mod] == 0) {
				Sleep(5);
				arr[mod] = p->i;
				LeaveCriticalSection(&cs);
				num++;
				Sleep(5);
			}
			else {
				cout << "Thread: " << p->i - 1 << " " << num << " " << mod << endl;
				LeaveCriticalSection(&cs);
				SetEvent(p->hCantEvent);
				break;
			}
		}
	}
	for (int i = 0; i < p->n; i++)
		if (arr[i] == p->i)
			arr[i] = 0;
	return 0;
}

int main()
{
	cout << "size of array:" << endl;
	int n;
	cin >> n;
	arr = new int[n];
	memset(arr, 0, sizeof(int) * n);
	InitializeCriticalSection(&cs);
	cout << "number of threads:" << endl;
	int max;
	cin >> max;
	HANDLE* hThreads = new HANDLE[max];
	params* p = new params[max];
	for (int i = 0; i < max; i++)
	{
		p[i].n = n;
		p[i].i = i + 1;
		p[i].hStartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (p[i].hStartEvent == NULL)
		{
			cerr << "Failed to create start event!\n";
			return 1;
		};
		p[i].hEndEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (p[i].hEndEvent == NULL)
		{
			cerr << "Failed to create end event!\n";
			return 1;
		}
		p[i].hCantEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (p[i].hCantEvent == NULL)
		{
			cerr << "Failed to create cant event!\n";
			return 1;
		}
		hThreads[i] = CreateThread(NULL, 0, marker, &p[i], 0, NULL);
		if (hThreads[i] == NULL)
		{
			cerr << "Failed to create a thread!\n";
			return 2;
		}
	}
	for (int i = 0; i < max; i++) {
		SetEvent(p[i].hStartEvent);
	}
	bool* a = new bool[max];
	memset(a, false, sizeof(bool) * max);
	while (true) {
		for (int i = 0; i < max; i++) {
			if (a[i] == false)
				WaitForSingleObject(p[i].hCantEvent, INFINITE);
		}
		cout << "array after marks:" << endl;
		for (int i = 0; i < n; i++) {
			cout << arr[i] << " ";
		}
		cout << endl << "the thread you want to end: ";
		int x;
		cin >> x;
		a[x] = 1;
		SetEvent(p[x].hEndEvent);
		WaitForSingleObject(hThreads[x], INFINITE);
		cout << "array after ends:" << endl;
		for (int i = 0; i < n; i++) {
			cout << arr[i] << " ";
		}
		cout << endl;
		bool allThreadEnd = 1;
		for (int i = 0; i < max; i++) {
			if (a[i] == 0) {
				SetEvent(p[i].hStartEvent);
				allThreadEnd = 0;
			}
		}
		if (allThreadEnd == 1) {
			cout << "all threads ended"; break;
		}
	}

	for (int i = 0; i < max; i++)
	{
		CloseHandle(hThreads[i]);
		CloseHandle(p[i].hStartEvent);
		CloseHandle(p[i].hEndEvent);
		CloseHandle(p[i].hCantEvent);
	}
	delete[] arr;
	delete[] p;
	delete[] hThreads;
	delete[] a;
	DeleteCriticalSection(&cs);
	return 0;
}
