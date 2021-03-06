#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/socket.h> 
#include <unistd.h>
#include <sstream>
#include <vector>

#define SRV_IP  "192.168.0.105"
#define PORT    2000
#define BUFLEN1 4096
#define BUFLEN2 4096

struct sockaddr_in si_other;
struct sockaddr_in client;
socklen_t slen; 
int s;
char buf1[BUFLEN1];
socklen_t client_length;
using namespace std;
double data[13];		/* Create array for data storage */

void Initialize_Client()
{   
  if ((s=socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP))==-1);
   
  memset((char *) &si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET;
  si_other.sin_port = htons(PORT);
  slen = sizeof(si_other);
  if (inet_aton(SRV_IP, &si_other.sin_addr)==0) { 
    exit(1);
  }
}

void Handshake_ServerClient()
{
    sendto(s, &buf1, BUFLEN1, 0, (struct sockaddr *)&si_other, slen); 
}

void Receive_UDP_Packet(double* time, double* xpos_r, double* ypos_r, double* atti_r,
                        double* xpos_b, double* ypos_b, double* atti_b,
                        double* xpos_el,double* ypos_el, double* xpos_wr, 
                        double* ypos_wr, double* xpos_ed, double* ypos_ed)
{	
    Handshake_ServerClient();
    client_length = (int)sizeof(struct sockaddr_in);
    
    
	/* Receive bytes from client */
	recvfrom(s, &data, BUFLEN2, 0, (struct sockaddr *)&client, &client_length);
    
    *time    = data[0];
    *xpos_r  = data[1];
    *ypos_r  = data[2];
    *atti_r  = data[3];
    *xpos_b  = data[4];
    *ypos_b  = data[5];
    *atti_b  = data[6];
    *xpos_el = data[7];
    *ypos_el = data[8];
    *xpos_wr = data[9];
    *ypos_wr = data[10];
    *xpos_ed = data[11];
    *ypos_ed = data[12];
           
    
}

void Terminate_SocketConnection()
{
     close(s);    
}