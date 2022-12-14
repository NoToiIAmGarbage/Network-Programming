#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;

int opt = 1, PORT;
const int mx_q = 100;
struct sockaddr_in addr;
int addrlen;

char buffer[1024];

class UDP {
public:
	UDP() {
		UDP_fd = socket(AF_INET, SOCK_DGRAM, 0);
		if(UDP_fd < 0) {
			cout << "UDP fd failed creating\n";
			exit(0);
		}
	}

	void close_connection() {
		close(UDP_fd);
	}

	void sendMes(string str) {
		sendto(UDP_fd, str.c_str(), str.size(), 0, (struct sockaddr*)&addr, sizeof(addr));
	}

	void readMes() {
		int val = recvfrom(UDP_fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
		buffer[val] = '\0';
		cout << buffer << '\n';
	}

	string getMes() {
		int val = recvfrom(UDP_fd, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
		buffer[val] = '\0';
		return (string)buffer;	
	}

private:
	int UDP_fd;
};

vector<string> process_string(string str) {
	vector<string> res;
	string tmp = "";
	for(char x : str) {
		if(x == ' ') {
			res.push_back(tmp);
			tmp = "";
		}
		else {
			if((int)x <= 31) {
				continue;
			}
			tmp += x;
		}
	}
	res.push_back(tmp);
	return res;
}

void buildFile(string name, string content) {
	ofstream f("./" + name);
	f << content;
	f.close();
}

int main(int argc, char** argv) {

	if (argc < 3) {
		cout << "Please input the port and IP for client side !!\n";
		return 0;
	}
	PORT = atoi(argv[2]); // extract the port

	cout << argv[1] << '\n';

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	// if(inet_pton(AF_INET, argv[1], &addr.sin_addr) <= 0) {
	// 	cout << "\nInvalid address/ Address not supported\n";
	// 	return 0;
	// }
	addrlen = sizeof(addr);
	// setup address
	
	UDP udp;
	string str;
	while(getline(cin, str)) {
		auto comm = process_string(str);
		if(comm[0] != "%") {
			cout << "Not a valid command!\n";
			continue;
		}
		udp.sendMes(str);
		if(comm[1] == "get-file-list") {
			udp.readMes();
		}
		else if(comm[1] == "exit") {
			break;
		}
		else if(comm[1] == "get-file") {
			for(int i = 1; i < comm.size(); i ++) {
				string res = udp.getMes();
				buildFile(comm[i], res);
			}
		}
	}
	return 0;
}