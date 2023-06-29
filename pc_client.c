#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>

#define PORT 8080

struct interface_struct {
  uint8_t enabled;
  uint8_t local_ip[4];
  uint8_t local_mask[4];
  uint8_t mac_addr[6];
  uint8_t name[64];
  uint8_t ifname[64];
};

int main(int argc, char *argv[]) {
  if(argc != 2){
    printf("-> Request Error\n");
    return -1;
  }
  // Create a socket:
  int client_fd_tcp = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server_addr;
  struct interface_struct network;

  // Set port and IP the same as server-side:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = inet_addr("192.168.10.1");

  // Send connection request to server:
  if (connect(client_fd_tcp, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    printf("Unable to connect\n");
    return -1;
  }
  printf("Connected with server successfully!\n");
  printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
  
  char buffer[1024];
  char request[32];
  char found_message[32];
  int found_flag;
  strcpy(request, argv[1]);

  memset(buffer, 0, sizeof(buffer));

  printf("Request <- \"%s\"\n", request);
  
  // Send the message to server:
  if(send(client_fd_tcp, request, strlen(request), 0) < 0) {
    printf("Unable to send message\n");
    return -1;
  }

  if(recv(client_fd_tcp, &found_flag, sizeof(found_flag), 0) < 0) {
    printf("Unable to send message\n");
    return -1;
  }
  
  // Receive the server's response.
  printf("Sent from server:\n");
  if(found_flag == 1){
    if(recv(client_fd_tcp, &network, sizeof(network), 0) < 0) {
      printf("Unable to send message\n");
      return -1;
    }
    printf("-> enabled: %hhu\n", network.enabled);
    printf("-> ipaddr: %hhu.%hhu.%hhu.%hhu\n", 
           network.local_ip[0], 
           network.local_ip[1], 
           network.local_ip[2], 
           network.local_ip[3]);
    printf("-> netmask: %hhu.%hhu.%hhu.%hhu\n", 
           network.local_mask[0], 
           network.local_mask[1], 
           network.local_mask[2], 
           network.local_mask[3]);
    printf("-> macaddr: %hhx:%hhx:%hhx:%hhx:%hhx:%hhx\n", 
           network.mac_addr[0], 
           network.mac_addr[1], 
           network.mac_addr[2], 
           network.mac_addr[3], 
           network.mac_addr[4], 
           network.mac_addr[5]);
    printf("-> name: %s\n", network.name);
    printf("-> ifname: %s\n", network.ifname);
  }
  else if(found_flag == 0){
    if(recv(client_fd_tcp, buffer, sizeof(buffer), 0) < 0) {
      printf("Unable to send message\n");
      return -1;
    }
    printf("-> %s\n", buffer);
  }
  // Print the server's response.
  printf("Message received!\n");

  // Close the sockets.
  close(client_fd_tcp);

  return 0;
}

