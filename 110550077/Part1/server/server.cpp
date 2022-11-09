#include <bits/stdc++.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fstream>
#include <unistd.h>
#include <filesystem>

using namespace std::filesystem;
using namespace std;


int opt = 1, PORT;
struct sockaddr_in addr;
int addrlen;

string getFiles() {
	path str("./");
	directory_iterator list(str);
	string res = " . ..";
	for(auto& it : list) {
		res += ' ' + (string)(it.path().filename());
	}
	return res;
}

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

string retFile(string str) {
	ifstream f("./" + str);
	string res, tmp;
	while(f >> tmp) {
		res += tmp + ' ';
	}
	res.pop_back();
	f.close();
	return res;
}

class Connection {
public:

	void process_command(string str) {
		auto comm = process_string(str);
		if(comm[1] == "get-file") {
			for(int i = 2; i < comm.size(); i ++) {
				string res = retFile(comm[i]);
				sendMes(res);
			}
		}
		else if(comm[1] == "get-file-list") {
			string res = getFiles();
			sendMes("Files:" + res);
		}
	}

	Connection() {
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(PORT);
		addrlen = sizeof(addr);
		// setup address
	}

	void UDP() {
		udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

		if( bind(udp_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			cout << "Udp socket fail to bind\n";
		}

		cout << "UDP server is running\n";

	}

	void handle_event() {
		while(true) {
			FD_ZERO(&readfds); // clear out

			FD_SET(udp_socket, &readfds);

			max_sd = udp_socket + 1; // for select function

			activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

			if ( (activity < 0) && (errno != EINTR) ) {  
	            printf("select error");  
	        }

			if(FD_ISSET(udp_socket, &readfds)) {
				int n = recvfrom(udp_socket, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
				buffer[n] = '\0';
				process_command(buffer);
			}

		}
	}

	void sendMes(string str) {
		sendto(udp_socket, str.c_str(), str.size(), 0, (struct sockaddr*)&addr, sizeof(addr));
	}
private:
	int max_sd;

	int udp_socket;

	int activity;

	fd_set readfds;

	char buffer[1024];
};

int main(int argc, char** argv) {

	if (argc < 2) {
		cout << "Please input the port for server side !!\n";
		return 0;
	}
	PORT = atoi(argv[1]); // extract the port
	
	Connection lol;

	lol.UDP();

	lol.handle_event();
}