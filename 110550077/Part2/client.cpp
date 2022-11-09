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


class TCP {
public:
	TCP() {
		TCP_fd = socket(AF_INET, SOCK_STREAM, 0);
		// file descriptor of TCP

		client_fd = connect(TCP_fd, (struct sockaddr*)&addr, sizeof(addr));
		if(client_fd < 0) {
			cout << "connection failed.\n";
			exit(0);
		}
		// connect to server

	}
	void readMes() {
		int val = read(TCP_fd, buffer, 1024);
		buffer[val] = '\0';
		cout << buffer << '\n';
	}

	void close_connection() {
		close(client_fd);
	}

	void sendMes(string str) {
		send(TCP_fd, str.c_str(), str.size(), 0);
	}
private:
	int TCP_fd, client_fd;
};

int main(int argc, char** argv) {

	if (argc < 3) {
		cout << "Please input the port and IP for client side !!\n";
		return 0;
	}
	PORT = atoi(argv[2]); // extract the port

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	if(inet_pton(AF_INET, argv[1], &addr.sin_addr) <= 0) {
		cout << "\nInvalid address/ Address not supported\n";
		return 0;
	}
	addrlen = sizeof(addr);
	// setup address
	
	TCP tcp;
	tcp.readMes();
	string str;
	while(getline(cin, str)) {
		auto comm = process_string(str);
		if(comm[0] != "%") {
			cout << "Invalid command!\n";
			continue;
		}
		if(comm[1] == "exit") {
			tcp.sendMes(str);
			tcp.readMes();
			tcp.close_connection();
			return 0;
		}
		else {
			tcp.sendMes(str);
			tcp.readMes();
		}
	}
	return 0;
}