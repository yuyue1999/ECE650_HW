#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <vector>
#include "potato.h"

class ringmaster{
public:
    std::string port;
    int NumPlayers;
    int hops;
    int socket_fd;
    std::vector<int> FDs;
    std::vector<std::string> IPs;
    std::vector<int> ports;
    
    ringmaster(std::string tport, int tNumPlayers,int thops):port(tport),NumPlayers(tNumPlayers),hops(thops){
        int rv;
        int yes=1;
        struct addrinfo hints, *servinfo;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        
        if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &servinfo)) != 0) {
            std::cerr << "Cannot get address information" << std::endl;
            exit(EXIT_FAILURE);
        }
        socket_fd = socket(servinfo->ai_family,
                                servinfo->ai_socktype,
                                servinfo->ai_protocol);
        if (socket_fd == -1) {
            std::cerr << "Cannot create socket" << std::endl;
            exit(EXIT_FAILURE);
        }
        rv = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if (rv == -1) {
            std::cerr << "setsockopt" << std::endl;
            exit(EXIT_FAILURE);
        }
        rv = bind(socket_fd, servinfo->ai_addr, servinfo->ai_addrlen);
        if (rv == -1) {
            close(socket_fd);
            std::cerr << "Cannot bind" << std::endl;
            exit(EXIT_FAILURE);
        }
        rv= listen(socket_fd, 150);
        if (rv == -1) {
            std::cerr << "Cannot listen on socket" << std::endl;
            exit(EXIT_FAILURE);
        }
        freeaddrinfo(servinfo);
        std::cout << "Potato Ringmaster"<<std::endl;
        std::cout << "Players = " << NumPlayers << std::endl;
        std::cout << "Hops = " << hops << std::endl;
    }
    int acceptwith(){
        struct sockaddr_storage connector_addr;
        socklen_t addr_len = sizeof(connector_addr);
        int player_fd= accept(socket_fd, (struct sockaddr *)&connector_addr, &addr_len);
        if (player_fd == -1) {
            std::cerr << "Cannot accept connection on that socket" << std::endl;
            exit(EXIT_FAILURE);
        }
        struct sockaddr_in *trans = (struct sockaddr_in *)&connector_addr;
        std::string ip = inet_ntoa(trans->sin_addr);
        IPs.push_back(ip);
        return player_fd;
    }
    void Send_and_Recv(){
        for(int i=0;i<NumPlayers;i++){
            FDs.push_back(acceptwith());
            send(FDs[i],(char*)&i,sizeof(int),0);
            send(FDs[i],(char*)&NumPlayers,sizeof(int),0);
            int tempport;
            recv(FDs[i], &tempport, sizeof(int), MSG_WAITALL);
            ports.push_back(tempport);
            std::cout << "Player " << i << " is ready to play\n";
        }
        for(int i=0;i<NumPlayers;i++){
            neighbor n;
            if(i==NumPlayers-1){
                n.port=ports[0];
                strcpy(n.addr, IPs[0].c_str());
            }else{
                n.port=ports[i+1];
                strcpy(n.addr, IPs[i+1].c_str());
            }
              send(FDs[i], &n, sizeof(n), 0);
          }
    }
    ~ringmaster() {
      for (int i = 0; i < NumPlayers; i++) {
        close(FDs[i]);
      }
        close(socket_fd);
    }
};



int main(int argc, const char * argv[]) {
    if(argc<4){
        std::cerr<<"Wrong input!"<<std::endl;
        return EXIT_FAILURE;
    }
    std::string port(argv[1]);
    int NumPlayers=std::stoi(argv[2]);
    int hops=std::stoi(argv[3]);
    if(hops<=0 || hops>512){
        std::cerr<<"Invalid hops input!"<<std::endl;
        exit(EXIT_FAILURE);
    }
    if(NumPlayers<2){
        std::cerr<<"Invalid hops input!"<<std::endl;
        exit(EXIT_FAILURE);
    }
    ringmaster *r=new ringmaster(port,NumPlayers,hops);
    r->Send_and_Recv();
    potato p(hops);
    int random=rand()%NumPlayers;
    std::cout << "Ready to start the game, sending potato to player " <<random<< std::endl;
    send(r->FDs[random],&p,sizeof(p),0);
    fd_set read_fds;
    FD_ZERO(&read_fds);
    int fd_max=-123456;
    for(int i:r->FDs){
        if(i>fd_max){
            fd_max=i;
        }
    }
    for (int i = 0; i < NumPlayers; ++i) {
        FD_SET(r->FDs[i], &read_fds);
    }
    int select_status=select(fd_max+1,&read_fds, NULL, NULL, NULL);
    if(select_status == -1) {
        std::cerr<<"Cannot select"<<std::endl;
        return EXIT_FAILURE;
    }
    for(int i=0;i<NumPlayers;i++){
        if(FD_ISSET(r->FDs[i], &read_fds)){
            recv(r->FDs[i], &p, sizeof(p), 0);
            break;
        }
    }
    for (int j = 0; j < NumPlayers; j++) {
        send(r->FDs[j], &p, sizeof(p), 0);
    }
    std::cout << "Trace of potato:" << std::endl;
    for (int i = 0; i <hops; i++) {
      if (i == hops-1) {
        std::cout << p.path[0] << std::endl;
      } else {
        std::cout << p.path[hops-1-i] << "," ;
      }
    }
    delete r;
    return 0;
}
