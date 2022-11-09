#include <bits/stdc++.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

#define max_clients 30

int opt = 1, PORT;
struct sockaddr_in addr;
int addrlen;

const string welcome_mes = "*****Welcome to Game 1A2B*****";

map<string, pair<string, string> > user2data;
map<string, pair<string, string> > email2data;

int ingame[max_clients + 5];

string state[max_clients + 5];

string game[max_clients + 5];

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

bool is4digit(string str) {
	if(str.size() != 4) {
		return false;
	}
	for(int i = 0; i < 4; i ++) {
		if(str[i] > '9' || str[i] < '0') {
			return false;
		}
	}
	return true;
}

pair<int, int> comp(string one, string two) {
	bool vis[4] = {0};
	pair<int, int> res = {0, 0};
	for(int i = 0; i < 4; i ++) {
		if(one[i] == two[i]) {
			res.first ++;
			vis[i] = true;
		}
	}
	for(int i = 0; i < 4; i ++) {
		for(int j = 0; j < 4; j ++) {
			if(!vis[i] && !vis[j] && one[i] == two[j]) {
				vis[i] = vis[j] = true;
				res.second ++;
			}
		}
	}
	return res;
}

class Connection {
public:
	void process_register(vector<string> comm) {
		if(comm.size() != 4) {
			sendMes("Usage: register <username> <email> <password>");
			return;
		}
		if(user2data.find(comm[1]) != user2data.end()) {
			sendMes("Username is already used.");
			return;
		}
		if(email2data.find(comm[2]) != email2data.end()) {
			sendMes("Email is already used.");
			return;
		}
		email2data[comm[2]] = {comm[1], comm[3]};
		user2data[comm[1]] = {comm[2], comm[3]};
		sendMes("Register successfully.");
	}

	void process_login(vector<string> comm, int ind) {
		if(comm.size() != 3) {
			sendMes("Usage: login <username> <password>", ind);
			return;
		}
		if(user2data.find(comm[1]) == user2data.end()) {
			sendMes("Username not found.", ind);
			return;
		}
		if(user2data[comm[1]].second != comm[2]) {
			sendMes("Password not correct.", ind);
			return;
		}
		sendMes("Welcome, " + comm[1] + ".", ind);
		state[ind] = comm[1];
	}

	void process_gamerule() {
		sendMes("1. Each question is a 4-digit secret number.\n2. After each guess, you will get a hint with the following information:\n2.1 The number of \"A\", which are digits in the guess that are in the correct position.\n2.2 The number of \"B\", which are digits in the guess that are in the answer but are in the wrong position.\nThe hint will be formatted as \"xAyB\".\n3. 5 chances for each question.");
	}

	void process_logout(int ind) {
		if(state[ind] == "") {
			sendMes("Please login first.", ind);
		}
		else {
			sendMes("Bye, " + state[ind], ind);
			state[ind] = "";
		}
	}

	void process_startgame(vector<string> comm, int ind) {
		if(state[ind] == "") {
			sendMes("Please login first.", ind);
			return;
		}
		if(comm.size() > 2) {
			sendMes("Usage: start-game <4-digit number>", ind);
			return;
		}
		if(comm.size() > 1 && !is4digit(comm[1])) {
			sendMes("Usage: start-game <4-digit number>", ind);
			return;
		}
		sendMes("Please typing a 4-digit number:", ind);
		ingame[ind] = 1;

		if(comm.size() == 2) {
			game[ind] = comm[1];
		}
		else {
			int t = rand() % 10000;
			game[ind] = "";
			for(int i = 1; i < 10000; i *= 10) {
				game[ind] += (char)('0' + t / i % 10);
			}
		}
		cout << game[ind] << '\n';
	}

	void process_command(string str, int ind = -1) {
		auto comm = process_string(str);
		if(ind == -1) {
			if(comm[0] == "register") {
				process_register(comm);
			}
			else if(comm[0] == "game-rule") {
				process_gamerule();
			}
			else {
				sendMes("Not a valid command");
			}
			return;
		}
		if(!ingame[ind]) {
			if(comm[0] == "login") {
				if(state[ind] != "") {
					sendMes("Please logout first.", ind);
					return;
				}
				process_login(comm, ind);
			}
			else if(comm[0] == "logout") {
				process_logout(ind);
			}
			else if(comm[0] == "start-game") {
				process_startgame(comm, ind);
			}
			else {
				sendMes("Not a valid command", ind);
			}
		}
		else {
			if(!is4digit(comm[0])) {
				sendMes("Your guess should be a 4-digit number.", ind);
			}
			else {
				ingame[ind] ++;
				pair<int, int> res = comp(comm[0], game[ind]);
				string hint = to_string(res.first) + "A" + to_string(res.second) + "B";
				if(res.first == 4) {
					sendMes("You got the answer!", ind);
					ingame[ind] = 0;
				}
				else {
					if(ingame[ind] == 6) {
						sendMes(hint + "\nYou lose the game!", ind);
						ingame[ind] = 0;
					}
					else {
						sendMes(hint, ind);
					}
				}
			}
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

	void TCP() {
		for(int i = 0; i < max_clients; i ++) {
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
			FD_SET(udp_socket, &readfds);

			max_sd = max(udp_socket, master_socket) + 1; // for select function

			for(int i = 0; i < max_clients; i ++) {
				int sd = clients[i];

				if(sd > 0) {
					FD_SET(sd, &readfds);
				}

				max_sd = max(max_sd, sd);
			}

			activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

			if ( (activity < 0) && (errno != EINTR) ) {  
	            printf("select error");  
	        }

			if(FD_ISSET(master_socket, &readfds)) {
				new_socket = accept(master_socket, (struct sockaddr *)&addr, (socklen_t*)&addrlen);
				if(new_socket < 0) {
					cout << "New socket fail to accept\n";
					exit(0);
				}

				cout << "New connection.\n";
				// cout << "FD : " << new_socket << '\n';
				// cout << "IP : " << inet_ntoa(addr.sin_addr) << '\n';
				// cout << "Port : " << ntohs(addr.sin_port) << '\n';

				if(send(new_socket, welcome_mes.c_str(), welcome_mes.size(), 0) != welcome_mes.size()) {
					cerr << "Failed to send welcome meassage\n";
				}

				for(int i = 0; i < max_clients; i ++) {
					if(clients[i] == -1) {
						clients[i] = new_socket;
						break;
					}
				}

			}

			if(FD_ISSET(udp_socket, &readfds)) {
				int n = recvfrom(udp_socket, buffer, 1024, 0, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
				buffer[n] = '\0';
				process_command(buffer);
			}


			for(int i = 0; i < max_clients; i ++) {
				int sd = clients[i];
				if(FD_ISSET(sd, &readfds)) {
					int val = read(sd, buffer, 1024);
					buffer[val] = '\0';
					if(!val) { // closing
						cout << "tcp get msg: exit\n";
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

	void sendMes(string str, int ind = -1) {
		if(ind != -1) {
			send(clients[ind], str.c_str(), str.size(), 0);
		}
		else {
			sendto(udp_socket, str.c_str(), str.size(), 0, (struct sockaddr*)&addr, sizeof(addr));
		}
	}
private:
	int master_socket, max_sd;

	int udp_socket;

	int activity, new_socket;

	fd_set readfds;
	
	int clients[max_clients];
	char buffer[1024];
};

int main(int argc, char** argv) {

	if (argc < 2) {
		cout << "Please input the port for server side !!\n";
		return 0;
	}
	PORT = atoi(argv[1]); // extract the port

	
	
	Connection lol;

	lol.UDP(); lol.TCP(); 

	lol.handle_event();

	cout << "TEST\n";
}