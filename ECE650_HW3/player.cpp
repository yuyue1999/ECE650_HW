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
#include "potato.h"
class player{
public:
    std::string host;
    std::string port;
    int masterfd;
    int nextfd;
    int prevfd;
    int id;
    int NumPlayer;
    int socket_fd;
    int neightport;
    std::string neightip;
    player(std::string thost, std::string tport): host(thost),port(tport){
        struct addrinfo hints, *servinfo;
        int rv;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        if ((rv = getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
            std::cerr << "Cannot get address information" <<std::endl;
            exit(EXIT_FAILURE);
        }
        
        masterfd = socket(servinfo->ai_family,
                          servinfo->ai_socktype,
                          servinfo->ai_protocol);
        if (masterfd == -1) {
            std::cerr << "Cannot create socket" << std::endl;
            exit(EXIT_FAILURE);
        }
        rv = connect(masterfd, servinfo->ai_addr, servinfo->ai_addrlen);
        if(rv == -1){
            std::cerr<<"Cannot connect"<<std::endl;
            exit(EXIT_FAILURE);
        }
        freeaddrinfo(servinfo);
        recv(masterfd, (char*)&id, sizeof(int), MSG_WAITALL);
        recv(masterfd, (char*)&NumPlayer, sizeof(int), MSG_WAITALL);
    }
    void startServer(){
        int rv;
        int yes=1;
        struct addrinfo hints, *servinfo;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        
        if ((rv = getaddrinfo(NULL, "", &hints, &servinfo)) != 0) {
            std::cerr << "Cannot get address information" << std::endl;
            exit(EXIT_FAILURE);
        }
        struct sockaddr_in *addr_in = (struct sockaddr_in *)(servinfo->ai_addr);
        addr_in->sin_port = 0;
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
        struct sockaddr_in tempaddin;
        socklen_t len = sizeof(tempaddin);
        rv=getsockname(socket_fd, (struct sockaddr *)&tempaddin, &len);
        if (rv == -1) {
            std::cerr << "Cannot get socket name" << std::endl;
            exit(EXIT_FAILURE);
        }
        int  tempPort=ntohs(tempaddin.sin_port);
        send(masterfd, (char*)&tempPort, sizeof(int), 0);
    }
    void ConnectNeighbor(){
        neighbor n;
        recv(masterfd, &n, sizeof(n), MSG_WAITALL);
        std::string intToStr=std::to_string(n.port);
        struct addrinfo hints, *servinfo;
        int rv;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if ((rv = getaddrinfo(n.addr, intToStr.c_str(), &hints, &servinfo)) != 0) {
            std::cerr << "Cannot get address information" <<std::endl;
            exit(EXIT_FAILURE);
        }
        nextfd = socket(servinfo->ai_family,
                          servinfo->ai_socktype,
                          servinfo->ai_protocol);
        if (nextfd == -1) {
            std::cerr << "Cannot create socket" << std::endl;
            exit(EXIT_FAILURE);
        }
        rv = connect(nextfd, servinfo->ai_addr, servinfo->ai_addrlen);
        if(rv == -1){
            std::cerr<<"Cannot connect"<<std::endl;
            exit(EXIT_FAILURE);
        }
        freeaddrinfo(servinfo);
        
        struct sockaddr_storage connector_addr;
        socklen_t addr_len = sizeof(connector_addr);
        prevfd= accept(socket_fd, (struct sockaddr *)&connector_addr, &addr_len);
        if (prevfd == -1) {
            std::cerr << "Cannot accept connection on that socket" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    ~player() {
        close(nextfd);
        close(prevfd);
        close(socket_fd);
        close(masterfd);
    }
};
int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr<<"Invalid input!"<<std::endl;
        return EXIT_FAILURE;
    }
    std::string host(argv[1]);
    std::string port(argv[2]);
    player *p = new player(host,port);
    p->startServer();
    std::cout << "Connected as player " << p->id << " out of "
              << p->NumPlayer << " total players"<<std::endl;
    p->ConnectNeighbor();
    srand((unsigned int)time(NULL) + p->id);
    potato po;
    fd_set read_fds;
    FD_ZERO(&read_fds);
    std::vector<int> FDs;
    FDs.push_back(p->prevfd);
    FDs.push_back(p->nextfd);
    FDs.push_back(p->masterfd);
    int fd_max=-123456;
    for(int i:FDs){
        if(i>fd_max){
            fd_max=i;
        }
    }
    while(true){
        FD_ZERO(&read_fds);
        for (int i = 0; i < 3; i++){
            FD_SET(FDs[i], &read_fds);
        }
        select(fd_max+1, &read_fds, NULL, NULL, NULL);
        for (int i = 0; i < 3; i++) {
            if (FD_ISSET(FDs[i], &read_fds)) {
                recv(FDs[i], &po, sizeof(po), MSG_WAITALL);
                
            }
        }
        if(po.hops==0){
            break;
        }
        po.hops--;
        po.path[po.hops]=p->id;
        if (po.hops == 0) {
            send(p->masterfd, &po, sizeof(po), 0);
            std::cout << "I'm it"<<std::endl;
            break;
        }
        int random = rand() % 2;
        if(random==0){
            if(p->id==0){
                int num=p->NumPlayer;
                std::cout << "Sending potato to " <<num-1<< std::endl;
            }else{
                std::cout << "Sending potato to " <<p->id-1<< std::endl;
            }
            send(FDs[0], &po, sizeof(po), 0);
        }else{
            if(p->id==p->NumPlayer-1){
                std::cout << "Sending potato to " <<0<< std::endl;
            }else{
                std::cout << "Sending potato to " <<p->id+1<< std::endl;
            }
            send(FDs[1], &po, sizeof(po), 0);
        }
    }
    delete p;
    return EXIT_SUCCESS;
}
