#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER 1024

struct interface_struct {
  uint8_t enabled;
  uint8_t local_ip[4];
  uint8_t local_mask[4];
  uint8_t mac_addr[6];
  uint8_t name[64];
  uint8_t ifname[64];
};

int main() {
  int new_socket;
  // Create a socket.
  int server_fd_tcp = socket(AF_INET, SOCK_STREAM, 0);

  const int enable = 1;
  if (setsockopt(server_fd_tcp, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
    perror("setsockopt(SO_REUSEADDR) failed");
  }

  // Prepare struct with address for server
  // INADDR_ANY == 0.0.0.0
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  address.sin_family = AF_INET;
  address.sin_port = htons(PORT);
  address.sin_addr.s_addr = INADDR_ANY;

  // Bind socket to IP-address
  if(bind(server_fd_tcp, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // Listen and start accpeting TCP connections
  if(listen(server_fd_tcp, 3) < 0){
    perror("listen");
    exit(EXIT_FAILURE);
  }

  // Run server in infinite loop
  while(1){
    // Create new socket after accepting TCP connection
    if((new_socket = accept(server_fd_tcp, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0){
      perror("accept");
      exit(EXIT_FAILURE);
    }

    char buffer[BUFFER];
    char for_client[BUFFER];
    char *error_message = "Request Error";
    char find_config[64];
    memset(buffer, 0, sizeof(buffer));

    if(recv(new_socket, buffer, sizeof(buffer), 0) < 0) {
      printf("Unable to send message\n");
      return -1;
    }
    printf("Client's request <- \"%s\"\n", buffer);
    
    struct interface_struct network;
    memset(&network, 0, sizeof(network));

    int start_line;
    int found_flag = 0;
    int start_flag = 0;
    int end_flag = 0;
    int match = 0;
    int counter = 1;

    FILE *network_lan = fopen("/etc/config/network", "r");
    while(fgets(for_client, BUFFER, network_lan) != NULL){
      if(strstr(for_client, "config")){
        sscanf(for_client, "config %*s \'%63[^']", find_config);
        if(strcmp(buffer, find_config) == 0){
          match = 1;
          send(new_socket, &match, sizeof(match), 0);
          start_line = counter;
          start_flag = 1;
          found_flag = 1;
          continue;
        }
      }

      if(start_flag == 1 && end_flag == 0 && found_flag == 1){
        if(strstr(for_client, "option disabled")){
          if(strstr(for_client, "\'0\'")){
            network.enabled = 1;
          }
          else{
            network.enabled = 0;
          }
        }
        if(strstr(for_client, "option enable")){
          sscanf(for_client, "%*[^']\'%hhu", &network.enabled);
        }
        if(strstr(for_client, "option ipaddr")){
          sscanf(for_client, "%*[^']\'%hhu.%hhu.%hhu.%hhu", 
                 &network.local_ip[0], 
                 &network.local_ip[1], 
                 &network.local_ip[2], 
                 &network.local_ip[3]);
        }
        if(strstr(for_client, "option netmask")){
          sscanf(for_client, "%*[^']\'%hhu.%hhu.%hhu.%hhu", 
                 &network.local_mask[0], 
                 &network.local_mask[1], 
                 &network.local_mask[2], 
                 &network.local_mask[3]);
        }
        if(strstr(for_client, "option macaddr")){
          sscanf(for_client, "%*[^']\'%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", 
                 &network.mac_addr[0],
                 &network.mac_addr[1], 
                 &network.mac_addr[2], 
                 &network.mac_addr[3], 
                 &network.mac_addr[4], 
                 &network.mac_addr[5]);
        }
        if(strstr(for_client, "option name")){
          sscanf(for_client, "%*[^']\'%63[^']", network.name);
        }
        if(strstr(for_client, "option ifname")){
          sscanf(for_client, "%*[^']\'%63[^']", network.ifname);
        }
        start_line++;
        if(strstr(for_client, "config")){
          end_flag = 1;
        }
      }
      counter++;
    }
    if(match == 0){
      send(new_socket, &match, sizeof(match), 0);
    }
    fclose(network_lan);

    printf("Sending the following to client:\n");
    if(found_flag == 1){
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

      if(send(new_socket, &network, sizeof(network), 0) < 0) {
        printf("Unable to send message\n");
        return -1;
      }
    }
    else{
      printf("-> %s\n", error_message);
      if(send(new_socket, error_message, strlen(error_message), 0) < 0) {
        printf("Unable to send message\n");
        return -1;
      }
    }
    printf("Message sent!\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");

    // Close the connected socket
    close(new_socket);
  } 
  // Close the listening socket
  close(server_fd_tcp);

  return 0;
}

