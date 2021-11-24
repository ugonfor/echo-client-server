#include <string.h>
#include <unistd.h>
#ifdef __linux__
#include <arpa/inet.h>
#include <sys/socket.h>
#endif // __linux
#ifdef WIN32
#include <winsock2.h>
#include "../mingw_net.h"
#endif // WIN32
#include <iostream>
#include <thread>

#include <set>
#include <queue>

using namespace std;

#ifdef WIN32
void perror(const char* msg) { fprintf(stderr, "%s %ld\n", msg, GetLastError()); }
#endif // WIN32

// for broadcast
struct Msg
{
	int fd;
	char* message = nullptr;
	ssize_t len = 0;
};
set<int> cli_sds;
queue<Msg> msg_queue;
uint8_t killThread[BUFSIZ];


void usage() {
	cout << "syntax : tcp-server <port> [-e[-b]]\n";
	cout << "    -e : echo\n";
	cout << "    -b : broadcast\n";
	cout << "sample : tcp-server 1234 -e -b\n";

}

struct Param {
	bool echo{false};
	// broadcast option
	bool broadcast{false};
	uint16_t port{0};

	bool parse(int argc, char* argv[]) {
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-e") == 0) {
				echo = true;
				continue;
			}
			// broadcast option
			else if (strcmp(argv[i], "-b") == 0){
				broadcast = true;
				continue;
			}
			port = stoi(argv[i]);
		}
		return port != 0;
	}
} param;

void sendThread(){
	cout << "echo : " << param.echo << "\n";
	cout << "broadcast : " << param.broadcast << "\n";
	while (true)
	{
		if (msg_queue.empty()){
			// if there is no sleep... this thread doesn't work correctly
			this_thread::sleep_for(chrono::milliseconds(1));
			continue;
		}
		
		Msg msg = msg_queue.front();
		msg_queue.pop();

		if (param.broadcast)
		{
			for (set<int>::iterator it = cli_sds.begin(); it != cli_sds.end(); it++)
			{
				ssize_t res = send(*it, msg.message, msg.len, 0);
				if (res == 0 || res == -1) {
					cerr << "send return " << res << " at client sd(" << *it << ")";
					perror(" ");
					// kill thread
					killThread[*it] = 1;
				}
			}
		}
		else{
			ssize_t res = send(msg.fd, msg.message, msg.len, 0);
			if (res == 0 || res == -1) {
				cerr << "send return " << res << " at client sd(" << msg.fd << ")";
				perror(" ");
				// kill thread
				killThread[msg.fd] = 1;
			}
		}
		// free dynamic allocation
		delete[] msg.message;
	}
	

}


void recvThread(int sd) {
	cout << "connected\n";

	// insert to client fd list
	cli_sds.insert(sd);

	static const int BUFSIZE = 65536;

	while (true) {
		// kill thread check
		if (killThread[sd]) {
			killThread[sd] = 0;
			break;
		}

		// Dynamic Allocation
		char* buf = new char[BUFSIZE];

		ssize_t res = recv(sd, buf, BUFSIZE - 1, 0);
		if (res == 0 || res == -1) {
			cerr << "recv return " << res;
			perror(" ");
			break;
		}
		buf[res] = '\0';
		cout << buf;
		cout.flush();

		if (param.echo) {
			// push to msg queue
			msg_queue.push({sd, buf, res});
		}
	}
	cout << "disconnected\n";
	close(sd);
	
	// erase client fd
	cli_sds.erase(sd);
}

int main(int argc, char* argv[]) {
	if (!param.parse(argc, argv)) {
		usage();
		return -1;
	}

#ifdef WIN32
	WSAData wsaData;
	WSAStartup(0x0202, &wsaData);
#endif // WIN32

	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1) {
		perror("socket");
		return -1;
	}

	int res;
#ifdef __linux__
	int optval = 1;
	res = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (res == -1) {
		perror("setsockopt");
		return -1;
	}
#endif // __linux

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(param.port);

	ssize_t res2 = ::bind(sd, (struct sockaddr *)&addr, sizeof(addr));
	if (res2 == -1) {
		perror("bind");
		return -1;
	}

	res = listen(sd, 5);
	if (res == -1) {
		perror("listen");
		return -1;
	}

	// sendThread
	thread* send_th = new thread(sendThread);
	send_th->detach();

	while (true) {
		struct sockaddr_in cli_addr;
		socklen_t len = sizeof(cli_addr);
		int cli_sd = accept(sd, (struct sockaddr *)&cli_addr, &len);
		if (cli_sd == -1) {
			perror("accept");
			break;
		}

		if( static_cast<uint32_t>(cli_sd) > sizeof(killThread)){ 
			fprintf(stderr,"Too many clients\n");
			break;
		}

		thread* t = new thread(recvThread, cli_sd);
		t->detach();
	}
	close(sd);
}
