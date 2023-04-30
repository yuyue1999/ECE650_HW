#include <vector>
#ifndef potato_h
#define potato_h


#endif /* potato_h */
class potato{
public:
    int hops;
    int path[512];
    potato(int thops):hops(thops){
    }
    potato():hops(0){
    }
};
class neighbor {
public:
  char addr[512];
  int port;
  neighbor(){
    memset(addr, 0, sizeof(addr));
  }
};
