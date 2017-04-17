#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define PORT "10061"
class Socket {//TCP
public:
	SOCKET s;

	Socket() {
		s = INVALID_SOCKET;
	}

	~Socket() {
		closesocket(s);
		s = INVALID_SOCKET;
	}

	bool WSAInit(WSADATA* w) {
		int iResult = WSAStartup(MAKEWORD(2, 2), w);
		if (iResult != 0) {
			printf("WSAStartup failed with error: %d\n", iResult);
			return false;
		}
		return true;
	}

	Socket operator=(SOCKET S) {
		s = S;
		return *this;
	}
	bool operator==(SOCKET S) {
		return s == S;
	}
	bool operator==(Socket S) {
		return s == S.s;
	}
	//usage: s = CreateSocket(AF_INET);
	bool CreateSocket(int af) {//IPV4 or IPv6 (PF_INET/PF_INET6)
		struct addrinfo hints, *res;
		int iResult;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = af;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Resolve the server address and port
		iResult = getaddrinfo(NULL, NULL, &hints, &res);
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);
			freeaddrinfo(res);
			return false;
		}

		// Create a SOCKET for connecting to server
		s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		freeaddrinfo(res);
		if (s == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			return false;
		}
		return true;
	}
	bool CreateSocket() {
		s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		return !(s == INVALID_SOCKET || s == SOCKET_ERROR);
	}
	//===============================================		
	bool Connect(char* ip, char* port) {
		struct addrinfo hints, *res, *ptr;
		int iResult;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		iResult = getaddrinfo(ip, port, &hints, &res);
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);
			return false;
		}

		for (ptr = res; ptr != NULL; ptr = ptr->ai_next) {

			// Create a SOCKET for connecting to server
			s = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (s == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				return false;
			}

			// Connect to server.
			iResult = connect(s, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				closesocket(s);
				s = INVALID_SOCKET;
				continue;
			}
			cout << "Connect to port " << ntohs((((struct sockaddr_in*)ptr->ai_addr)->sin_port)) << endl;
			break;
		}

		freeaddrinfo(res);

		if (s == INVALID_SOCKET) {
			printf("Unable to connect to server!\n");

			return false;
		}
		return true;
	}
	//===============================================
	bool BindAndListen(char* port) {
		struct addrinfo *res, hints;
		int iResult;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		iResult = getaddrinfo(NULL, port, &hints, &res);
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);
			return false;
		}
		iResult = bind(s, res->ai_addr, (int)res->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			printf("bind failed with error: %d\n", WSAGetLastError());
			freeaddrinfo(res);
			return false;
		}
		freeaddrinfo(res);

		iResult = listen(s, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(s);
			return false;
		}
		return true;
	}
	//==================================================
	bool Bind(char* port) {//IPv4
		struct addrinfo *res, hints;
		int iResult;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		iResult = getaddrinfo(NULL, port, &hints, &res);
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);
			return false;
		}
		iResult = bind(s, res->ai_addr, (int)res->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			printf("bind failed with error: %d\n", WSAGetLastError());
			freeaddrinfo(res);
			return false;
		}
		freeaddrinfo(res);
		return true;
	}
	//====================================================================================================
	SOCKET Accept(struct sockaddr_storage *their_addr) {
		socklen_t addr_size = sizeof their_addr;

		SOCKET Client = accept(s, (struct sockaddr*) their_addr, &addr_size);
		if (Client == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			return Client;
		}
		return Client;
	}
	SOCKET Accept2() {
		struct sockaddr_storage addr;
		char ipstr[INET_ADDRSTRLEN];
		int addr_size = sizeof addr;
		SOCKET Client = accept(s, (struct sockaddr*) &addr, &addr_size);
		if (Client == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			return Client;
		}
		inet_ntop(AF_INET, &((struct sockaddr_in*)&addr)->sin_addr, ipstr, sizeof ipstr);
		cout << "Accept Success form(" << ipstr << ")" << endl;
		return Client;
	}
	size_t Send(char* buf, size_t len, int flags) {
		return send(s, buf, len, flags);
	}

	//usage: S.Receive(buf,0); 
	size_t Receive(char* buf, int flags) {
		return recv(s, buf, sizeof buf, flags);
	}

};
enum {
	SPADES = 0,
	HEARTS = 1,
	DAIMONDS = 2,
	CLUBS = 3
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
		RemainCard++;
		temp = new Card;
		temp->point = p;
		temp->color = c;
		temp->next = to;
		if (to != NULL) {
			temp->pre = to->pre;
			if (to->pre != NULL)
				(to->pre)->next = temp;
			to->pre = temp;
			if (top == to)
				top = temp;
		}
		else {//add to bottom
			if (bottom != NULL) {
				bottom->next = temp;
				temp->pre = bottom;
				temp->next = NULL;
			}
			else {//first add
				top = bottom = temp;
				temp->next = temp->pre = NULL;
			}
			bottom = temp;
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
	void Delete(Card* p) {
		Card *temp = p, *t;
		while (temp != NULL) {
			t = temp->next;
			delete temp;
			temp = t;
		}
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
	}*/
	Card* GetCard(int index) {//return NULL while fail
		int i;
		Card *temp = top;
		for (i = 0; i < index; i++) {
			if (temp == NULL)
				return NULL;
			temp = temp->next;
		}
		return temp;
	}
};

CardPile CP;
char* str2char(string str, char* buf);
void Display();
int CountPoint();
string I2CardPoint(int i);
void ProcessData(char* buf);
int main() {
	int i;
	Socket Client;
	WSADATA w;
	char buf[512];
	string str;
	Client.WSAInit(&w);
	while (!Client.CreateSocket()) {
		cout << "create fail" << endl;
		system("pause");
	}
		

	while (!Client.Connect("127.0.0.1", PORT)) {
		cout << "connect fail" << endl;
		system("pause");
	}
	cout << "connect success" << endl;

	cout << "================================================" << endl << endl;
	while (true) {
		cout << "Start New Game?(y/n)" << endl;
		cin >> buf;
		if (buf[0] != 'y' && buf[0] != 'Y') {
			cout << "game end " << endl;
			system("pause");
			return 0;
		}
		//Initial
		CP.Delete(CP.top);
		CP.top = CP.bottom = NULL;
		i = Client.Send(str2char("f", buf), 2, 0);
		if (i > 0) {
			i = recv(Client.s, buf, sizeof buf, 0);
			//i = Client.Receive(buf, 0);//ex: 12 S 5 C (S = SPADES,C=CLUBS)
			if (i > 0) {
				cout << "Client recv " << i << "bytes" << endl;
				cout << buf << endl;
				//system("pause");
				ProcessData(buf);
				Display();
				cout << "====================================" << endl;
			}
			else
				return 1;

		}
		else {
			//cout << "Initial send error..." << endl;
			break;
		}
		while (true) {
			cout << "==================================================" << endl;
			cout << "還要?(y/n)" << endl;
			cin >> buf;
			if (buf[0] != 'y' && buf[0] != 'Y') {
				cout << "你贏了 " << CountPoint() << "點" << endl;
				break;
			}
			//...
			i = Client.Send(str2char("y", buf), 5, 0);
			if (i > 0) {

			}
			else {
				//error
				return 1;
			}
			//wait
			i = Client.Receive(buf, 0);
			//system("pause");
			if (i > 0) {
				cout << "Client recv " << i << "bytes" << endl;
				cout << buf << endl;
				ProcessData(buf);
				Display();
				
			}
			else {
				//connction error
				break;
			}
			i = CountPoint();
			if (i > 21) {
				cout << "You lose (" << i << " points)" << endl;
				break;
			}
		}
		//cout << "==================================================" << endl << endl;

	}

	system("pause");
	return 0;
}

void ProcessData(char* buf) {
	string str;
	str = buf;
	int point, color, f;
	while (true) {
		point = (int)(((int)str.c_str()[0]) - (int)'0');
		str = str.substr(str.find(" ", 0) + 1);// "12 spades" => "spades"
		switch (str[0]) {
		case'S':
			color = SPADES;
			break;
		case'H':
			color = HEARTS;
			break;
		case'D':
			color = DAIMONDS;
			break;
		case'C':
			color = CLUBS;
			break;
		}
		CP.AddCard(CP.top, point, color);
		f = str.find(" ", 0);
		if (f == str.npos)	break;
		str = str.substr(f + 1);
	}
}

int CountPoint() {
	int ans = 0, aceCount = 0;
	Card *p = CP.top;
	while (p != NULL) {
		if (p->point > 1) {
			if (p->point < 10)
				ans += p->point;
			else
				ans += 10;
		}
		else
			aceCount++;
		p = p->next;
	}
	while (aceCount--) {
		if (ans <= 10)
			ans += 11;
		else
			ans += 1;
	}
	return ans;
}
void Display() {
	Card *p = CP.top;
	cout << "你現在的牌:" << endl;
	while (p != NULL) {
		cout << I2CardPoint(p->point) << " ,";
		p = p->next;
	}
	cout /*<< endl << "=======================================" */<< endl;
}
char* str2char(string str, char* buf) {
	memcpy(buf, str.c_str(), str.length());
	return buf;
}
string I2CardPoint(int i) {
	char s[2];
	if (i > 1) {
		if (i < 10) {
			s[0] = (char)'0' + i;
			s[1] = 0;
			return s;
		}
		else {
			switch (i) {
			case 10:
				return "10";
			case 11:
				return "J";
			case 12:
				return "Q";
			case 13:
				return "K";
			}
		}
	}
	else
		return "A";
}