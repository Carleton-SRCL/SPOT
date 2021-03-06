#define PORT 12345
#define BUFFERLEN 4096

#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>
#include <string>
#include <strings.h>
#include <arpa/inet.h>
#include <sstream>


//your variables that your code uses goes here
int server_socket_fd(-1), client_socket_fd(-1), port_number(PORT), num_chars(0), conncectionAvail(0), writeOK(0), check(0);
char buffer[4096];
char message[BUFFERLEN];
double guidance_data[3];
socklen_t client_len;
struct sockaddr_in server_address, client_address;
struct timeval tv = {0, 5000};
std::string serverOutput;
bool zed_init_ret_state(false), zed_start_ret_state(false), zed_stop_ret_state(false);

void createServerNewSocket()
{
    if(server_socket_fd >= 0)
        close(server_socket_fd);
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    int enable = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        std::cout << "setsockopt failed" << std::endl;
        exit(1);
    }
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_number);
    server_address.sin_addr.s_addr = INADDR_ANY;
    //TODO:: ADD ERROR LOGIC IN CODE TO MAKE SURE PORT IS NOT IN USE ETC
    bind(server_socket_fd, (struct sockaddr *) &server_address, sizeof(server_address));
    listen(server_socket_fd,5);
   
}

void run_guidance(const double input_1, const double input_2, 
                const double input_3, const double input_4,
                const double input_5, const double input_6,
                const double input_7, const double input_8,
                const double input_9, const double input_10,
                const double input_11, const double input_12,
                const double input_13, double* output_1, 
                double* output_2, double* output_3)
{
    
    if (client_socket_fd < 0)
    {
        client_len = sizeof(client_address);
        client_socket_fd = accept(server_socket_fd,
                                  (struct sockaddr *) &client_address,
                                  &client_len);
        setsockopt(client_socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    }
    else {
       std::string stringMessageOut = "";
       
       stringMessageOut = std::to_string(input_1) + "\n" + 
               std::to_string(input_2) + "\n" + 
               std::to_string(input_3) + "\n" + 
               std::to_string(input_4) + "\n" +
               std::to_string(input_5) + "\n" + 
               std::to_string(input_6) + "\n" + 
               std::to_string(input_7) + "\n" +
               std::to_string(input_8) + "\n" +
               std::to_string(input_9) + "\n" + 
               std::to_string(input_10) + "\n" +
               std::to_string(input_11) + "\n" +
               std::to_string(input_12) + "\n" +
               std::to_string(input_13) + "\n";

       check = write(client_socket_fd, stringMessageOut.c_str(), stringMessageOut.length());
       if (check <= 0)
       {
           close(client_socket_fd);
           client_socket_fd = -1;
           return;
       }
       recv(client_socket_fd, &message, BUFFERLEN, 0);
       std::stringstream ss(message);
       std::string out;
       
       std::getline(ss, out, '\n'); 
       *output_1 = atof(out.c_str());
       
       std::getline(ss, out, '\n'); 
       *output_2 = atof(out.c_str());
       
       std::getline(ss, out, '\n'); 
       *output_3 = atof(out.c_str());
               
       //*output_2 = guidance_data[1];
       //*output_3 = guidance_data[2];
    }
}

void terminate()
{
    if(client_socket_fd >= 0)
        close(client_socket_fd);
    if(server_socket_fd >= 0)
        close(server_socket_fd);
}
