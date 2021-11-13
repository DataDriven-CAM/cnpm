#include <unistd.h>
#include "npm/WebGetter.h"

#include "Url.h"

namespace sylvanmats::reading{

    bool WebGetter::operator()(std::string& urlStr, std::ostream& ss, std::function<void(std::istream& content)> apply){
    url::Url url(urlStr);
    struct addrinfo hints = {}, *infoptr;
    char port_str[16] = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    sprintf(port_str, "%d", port);
    int result = getaddrinfo(url.host().c_str(), port_str, &hints, &infoptr);
    if (result) {
        printf("getaddrinfo: %s\n", gai_strerror(result));
        return false;
    }
    struct addrinfo *ptr;
    struct sockaddr_in  *sockaddr_ipv4;
        struct sockaddr_in6 *sockaddr_ipv6;
    char host[256];
    int sd = -1;
    for (ptr = infoptr; ptr != NULL; ptr = ptr->ai_next) {

        getnameinfo(ptr->ai_addr, ptr->ai_addrlen, host, sizeof (host), NULL, 0, NI_NUMERICHOST);
        //puts(host);
    //std::cout<<"host: "<<host<<" "<<(ptr->ai_family==AF_INET)<<std::endl;
        sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sd < 0) {
            printf("Error creating socket.\n");
            break;
        }
        switch (ptr->ai_family) {
        case AF_UNSPEC:
            printf("Unspecified\n");
            break;
        case AF_INET:
            //printf("AF_INET (IPv4)\n");
            sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
            //printf("\tIPv4 address %s\n",
            //    inet_ntoa(sockaddr_ipv4->sin_addr) );
            break;
        case AF_INET6:
            printf("AF_INET6 (IPv6)\n");
            // the InetNtop function is available on Windows Vista and later
            // sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
            // printf("\tIPv6 address %s\n",
            //    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );

            // We use WSAAddressToString since it is supported on Windows XP and later
            sockaddr_ipv6 = (sockaddr_in6*) ptr->ai_addr;
            // The buffer length is changed by each call to WSAAddresstoString
            // So we need to set it for each iteration through the loop for safety
            //ipbufferlength = 46;
            /*iRetval = WSAAddressToString(sockaddr_ip, (DWORD) ptr->ai_addrlen, NULL, 
                ipstringbuffer, &ipbufferlength );
            if (iRetval)
                printf("WSAAddressToString failed with %u\n", WSAGetLastError() );
            else    
                printf("\tIPv6 address %s\n", ipstringbuffer);*/
            break;
        /*case AF_NETBIOS:
            printf("AF_NETBIOS (NetBIOS)\n");
            break;*/
        default:
            printf("Other %ld\n", ptr->ai_family);
            break;
    }
    /*printf("\tSocket type: ");
    switch (ptr->ai_socktype) {
        case 0:
            printf("Unspecified\n");
            break;
        case SOCK_STREAM:
            printf("SOCK_STREAM (stream)\n");
            break;
        case SOCK_DGRAM:
            printf("SOCK_DGRAM (datagram) \n");
            break;
        case SOCK_RAW:
            printf("SOCK_RAW (raw) \n");
            break;
        case SOCK_RDM:
            printf("SOCK_RDM (reliable message datagram)\n");
            break;
        case SOCK_SEQPACKET:
            printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
            break;
        default:
            printf("Other %ld\n", ptr->ai_socktype);
            break;
    }
    printf("\tProtocol: ");
    switch (ptr->ai_protocol) {
        case 0:
            printf("Unspecified\n");
            break;
        case IPPROTO_TCP:
            printf("IPPROTO_TCP (TCP)\n");
            break;
        case IPPROTO_UDP:
            printf("IPPROTO_UDP (UDP) \n");
            break;
        default:
            printf("Other %ld\n", ptr->ai_protocol);
            break;
    }*/
        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0) {
            break;
        }
        close(sd);
        printf("Error connecting to server.\n");
        return false;
    }

    freeaddrinfo(infoptr);
    //int s = socket(AF_UNSPEC, SOCK_STREAM, 0);
    if (sd < 0) {
        //printf("Error creating socket.\n");
        return false;
    }

    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD *meth = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new (meth);
    ssl = SSL_new (ctx);
    if (!ssl) {
        printf("Error creating SSL.\n");
        log_ssl();
        return false;
    }
    sock = SSL_get_fd(ssl);
    SSL_set_fd(ssl, sd);
    int err = SSL_connect(ssl);
    if (err <= 0) {
        printf("Error creating SSL connection.  err=%x\n", err);
        log_ssl();
        fflush(stdout);
        return false;
    }
    //printf ("SSL connection using %s\n", SSL_get_cipher (ssl));
    std::string escapedUrl=url::Url::escape_reserved_unsafe(urlStr);
//std::cout<<"escapedUrl "<<escapedUrl<<std::endl;
    std::string request = "GET "+url.path()+" HTTP/1.1\r\nUser-Agent: cnpm/1.0.0\r\nAccept: */*\r\nAccept-Encoding: identity\r\nHost: "+url.host()+"\r\nConnection: Keep-Alive\r\n\r\n\r\n";
//    std::cout<<"request "<<request<<std::endl;
    int status=SendPacket(request.c_str());
//    std::cout<<"SentPacket "<<status<<std::endl;
    status=RecvPacket(ss);
//    std::cout<<"RecvPacket "<<status<<std::endl;
    return true;
    };

}

