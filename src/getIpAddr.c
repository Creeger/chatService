#include <sys/types.h>
#include <ifaddrs.h>
#include <stddef.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>







char* getIpAddr() {
    struct ifaddrs *ifaddr, *ifa;
    char *ip = NULL;

    if (getifaddrs(&ifaddr) == -1) {
        return NULL;
    }

    // Loop through the local interfaces
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) {
            continue;
        }
        
        // Check if interface has a IPv4 address or if the interface is the loopback interface
        if (ifa->ifa_addr->sa_family == AF_INET && strcmp(ifa->ifa_name, "lo") != 0) {
            
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;

            ip = malloc(INET_ADDRSTRLEN); // Put the ip value from the stack and to the heap
            if (!ip) {
                break;
            }

            if (inet_ntop(AF_INET,&sa->sin_addr, ip, INET_ADDRSTRLEN)) {
                freeifaddrs(ifaddr);
                return ip;
            }

        free(ip);
        ip = NULL;
        }
    }
    freeifaddrs(ifaddr);
    return NULL;
}



/* Usage
int main() {
    char *ip = get_local_ip();

    if (ip) {
        printf("Local IP: %s\n", ip);
        free(ip);   // VERY IMPORTANT
    } else {
        printf("Could not get IP\n");
    }

    return 0;
}
*/
