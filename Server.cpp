#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <ctime>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;

#define PORT "10061"

enum {
	SPADES = 0,HEARTS = 1,DAIMONDS = 2,CLUBS = 3
};
class Card {
public:
	int point;
	int color;
	class Card* next;
	class Card* pre;
	Card operator=(Card c) {
		point = c.point;
		color = c.color;
		next = c.next;
		pre = c.pre;
	}
};
class CardPile {
public:
	int RemainCard;
	Card *top;
	Card *bottom;

	CardPile() {
		RemainCard = 0;
		top = bottom = NULL;
	}
	~CardPile() {
		RemainCard = 0;
		delete[] top;
		delete[] bottom;
	}
	//====================================
	void CreatePlieAndWash() {
		int i, index[52];
		RemainCard = 52;
		for (i = 0; i < 52; i++)
			index[i] = i;

		for (i = 0; i < 52; i++) //random swap 52 times
			swap(index[rand() % 52], index[rand() % 52]);
		
		for (i = 0; i < 52; i++)
			AddCard(top, index[i] % 13 + 1, index[i] / 13);
	}
	void ShowCardPile(Card* p) {
		Card* temp = p->next;
		while (p != NULL) {
			cout << p->point << ", ";
			p = p->next;
		}
		cout << endl;
	}
	void AddCard(Card *to, int p, int c) {//插入到 to 上面,傳入NULL 重底部
		Card *temp;
		temp = new Card;
		temp->point = p;
		temp->color = c;
		temp->next = to;

		if (to != NULL) {
			temp->pre = to->pre;
			if (to->pre != NULL) {
				(to->pre)->next = temp;
			}
			to->pre = temp;
			if (to == top)
				top = temp;
		}
		else {//add to bottom
			if (bottom != NULL) {
				bottom->next = temp;
				temp->pre = bottom;
				temp->next = NULL;
			}
			else {
				top = bottom = temp;
				temp->next = temp->pre = NULL;
			}

			bottom = temp;
			if (top == NULL)//first add
				top = temp;
		}
	}
	Card* PopCard() {
		Card *temp;
		temp = top;
		if (top != NULL) {
			top->pre = NULL;
			top = top->next;
		}
		temp->next = NULL;
		temp->pre = NULL;
		RemainCard--;
		return temp;
	}

	/*void Wash(int HowMany) {
	int i, a, b;
	Card *A,*B;
	for (i = 0; i < HowMany; i++) {
	a = rand() % RemainCard;
	b = rand() % RemainCard;
	A = GetCard(a);
	B = GetCard(b);
	if(A!=NULL && B!=NULL)
	swap(*A, *B);//!
	}
	}
	Card* GetCard(int index) {//return NULL while fail
		int i;
		Card *temp = top;
		for (i = 0; i < index; i++) {
			if (temp == NULL)
				return NULL;
			temp = temp->next;
		}
		return temp;
	}*/
};
int ProcessClientData(char* buf);
CardPile CP;
int main() {
	WSADATA w;
	SOCKET Server, Client;
	addrinfo hints, *res;
	sockaddr_storage addr;
	int iResult, addr_len = sizeof addr;
	char buf[100],ipstr[INET_ADDRSTRLEN];
	
	srand(time(NULL));
	WSAStartup(MAKEWORD(2, 2), &w);
	Server = socket(PF_INET, SOCK_STREAM, 0);
	if (Server == INVALID_SOCKET) {
		return 1;
	}

	//set hints
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;


	// Resolve the server address and port
	iResult = getaddrinfo(NULL, PORT, &hints, &res);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		return 1;
	}
	iResult = bind(Server, res->ai_addr, (int)res->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(Server);
		freeaddrinfo(res);
		return 1;
	}
	freeaddrinfo(res);
	iResult = listen(Server, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(Server);
		return 1;
	}
	//CardPile Initial
	CP.CreatePlieAndWash();
	while (1) {
		cout << "waiting for new connection..." << endl;
		Client = accept(Server, (struct sockaddr*)&addr, &addr_len);
		if (Client == INVALID_SOCKET) {
			cout << "accpet fail : error code = " << WSAGetLastError() << endl;
			break;
		}
		inet_ntop(AF_INET, &((struct sockaddr_in*)&addr)->sin_addr, ipstr, sizeof ipstr);
		cout << "Accept connect form(" << ipstr << ")" << endl;
		
		do {
			iResult = recv(Client, buf, sizeof buf, 0);
			if (iResult>0) {
				cout << "Server recv " << iResult << "bytes" << endl;
				cout << buf << endl;
				cout << "==========================================" << endl;
				iResult = send(Client, buf, ProcessClientData(buf), 0);
				cout << "Server send " << iResult << "bytes" << endl;
				cout << buf << endl;
				cout << "牌堆 : ";
				CP.ShowCardPile(CP.top);
				cout << "==========================================" << endl;
			}
			else if (iResult == 0) {
				cout << "connection closing ..." << endl;
			}
			else {
				cout << "Server recv error (error code=" << WSAGetLastError() << ") " << endl;
			}
		} while (iResult>0);

		cout << "==========================================================" << endl;
	}

}
char Color2Char(int color) {
	switch (color){
	case SPADES:
		return 'S';
	case HEARTS:
		return 'H';
	case DAIMONDS:
		return 'D';
	case CLUBS:
		return 'C';
	}
	return 0;
}

int ProcessClientData(char* buf) {
	Card *p;
	char p1, c1, p2, c2;
	if (buf[0] == 'f') {//first-time
		p = CP.PopCard();
		p1 = (char)p->point + '0';
		c1 = Color2Char(p->color);
		p = CP.PopCard();
		p2 = (char)p->point + '0';
		c2 = Color2Char(p->color);
		buf[0] = p1;
		buf[1] = ' ';
		buf[2] = c1;
		buf[3] = ' ';
		buf[4] = p2;
		buf[5] = ' ';
		buf[6] = c2;
		buf[7] = '\0';
		return 8;
	}
	else {
		p = CP.PopCard();
		p1 = p->point + '0';
		c1 = Color2Char(p->color);
		buf[0] = p1;
		buf[1] = ' ';
		buf[2] = c1;
		buf[3] = '\0';
		return 4;
	}
}