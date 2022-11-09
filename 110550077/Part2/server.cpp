#include <bits/stdc++.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

#define max_clients 1000

int opt = 1, PORT;
struct sockaddr_in addr;
int addrlen, m = 1;
string userIp[max_clients + 5];
int userPort[max_clients + 5];

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

class Connection {
public:

	void process_command(string str, int ind) {
		auto comm = process_string(str);
		if(comm[1] == "list-users") {
			string res = "";
			for(int i = 1; i <= max_clients; i ++) {
				if(clients[i] != -1) {
					if(res.size() > 0) {
						res += '\n';	
					}
					res += "user" + to_string(i);
				}
			}
			sendMes(res, ind);
		}
		else if(comm[1] == "get-ip") {
			sendMes("IP: " + userIp[ind] + ":" + to_string(userPort[ind]), ind);
		}
		else if(comm[1] == "exit") {
			cout << "user" << ind << " " << userIp[ind] << ":" << userPort[ind] << " disconnected\n";
			sendMes("Bye user" + to_string(ind) + '.', ind);
			close(clients[ind]);
			clients[ind] = -1;
		}
	}

	Connection() {
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(PORT);
		addrlen = sizeof(addr);
		// setup address
	}

	void TCP() {
		for(int i = 1; i <= max_clients; i ++) {
			clients[i] = -1;
		}
		master_socket = socket(AF_INET, SOCK_STREAM, 0);
		// file descriptor of master socket

		if(!master_socket) {
			cout << "Master socket failed to establish.\n";
			exit(-1);
		} // handle error event


		if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
			cout << "Master socket fail to be configured.\n";
			exit(0);
		}
		// configure master socket

		if( bind(master_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			cout << "Master socket fail to bind\n";
		}
		// bind the master socket to the address struct

		if( listen(master_socket, max_clients) < 0) {
			cout << "Failed to start listening.\n";
			exit(0);
		}
		// starts listening at PORT

		cout << "TCP server is running\n";
	}

	void handle_event() {
		while(true) {
			FD_ZERO(&readfds); // clear out

			FD_SET(master_socket, &readfds);

			max_sd = master_socket + 1; // for select function

			for(int i = 1; i <= max_clients; i ++) {
				int sd = clients[i];

				if(sd > 0) {
					FD_SET(sd, &readfds);
				}

				max_sd = max(max_sd, sd);
			}

			activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

			if(FD_ISSET(master_socket, &readfds)) {
				new_socket = accept(master_socket, (struct sockaddr *)&addr, (socklen_t*)&addrlen);
				if(new_socket < 0) {
					cout << "New socket fail to accept\n";
					exit(0);
				}

				userIp[m] = inet_ntoa(addr.sin_addr);
				userPort[m] = ntohs(addr.sin_port);
				cout << "New connection from " << userIp[m] << ":" << userPort[m] << ' ' << "user" << m << '\n';

				string welcome_mes = "Welcome, you are user" + to_string(m);

				clients[m ++] = new_socket;

				if(send(new_socket, welcome_mes.c_str(), welcome_mes.size(), 0) != welcome_mes.size()) {
					cerr << "Failed to send welcome meassage\n";
				}

			}

			for(int i = 1; i <= max_clients; i ++) {
				int sd = clients[i];
				if(FD_ISSET(sd, &readfds)) {
					int val = read(sd, buffer, 1024);
					buffer[val] = '\0';
					if(!val) { // closing
						cout << "user" << i << " " << userIp[i] << ":" << userPort[i] << " disconnected\n";
						sendMes("Bye user" + to_string(i) + '.', i);
						close(sd);
						clients[i] = -1;
					}
					else {
						process_command(buffer, i);
					}
				}
			}

		}
	}

	void sendMes(string str, int ind) {
		send(clients[ind], str.c_str(), str.size(), 0);
	}
private:
	int master_socket, max_sd;

	int activity, new_socket;

	fd_set readfds;
	
	int clients[max_clients + 5];
	char buffer[1024];
};

int main(int argc, char** argv) {

	if (argc < 2) {
		cout << "Please input the port for server side !!\n";
		return 0;
	}
	PORT = atoi(argv[1]); // extract the port

	
	
	Connection lol;

	lol.TCP(); 

	lol.handle_event();

	cout << "TEST\n";
}