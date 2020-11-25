#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct __attribute__((__packed__)) {
    uint16_t ID;
    uint16_t FLAGS;
    uint16_t QDCOUNT;
    uint16_t ANCOUNT;
    uint16_t NSCOUNT;
    uint16_t ARCOUNT;
} dns_header_t;

int build_dns_qname(char* name, uint8_t* buffer)
{
    uint8_t* pos = buffer;
    for (char* token = strtok(name, "."); token; token = strtok(NULL, ".")) {
        uint8_t len = strlen(token);
        *(pos++) = len;
        memcpy(pos, token, len);
        pos += len;
    }
    *(pos++) = 0;

    return pos - buffer;
}

int build_dns_request(char* name, uint8_t* buffer)
{
    size_t size = sizeof(dns_header_t);

    dns_header_t* header = (dns_header_t*)buffer;
    memset(header, 0, size);

    header->ID = htons(1);
    // HERE I DIDNT GET BYTE ORDER
    uint16_t flags = (0LL << 15) | // QUESTION
                     (0LL << 11) | // STANDARD REQUEST
                     (0LL << 9) |  // NOT TRUNCATED
                     (1LL << 8);   // RECURSION DESIRED
    header->FLAGS = htons(flags);
    header->QDCOUNT = htons(1); // ONE QUESTION

    size += build_dns_qname(name, buffer + size);

    uint16_t QTYPE = htons(1);  // A RECORD
    uint16_t QCLASS = htons(1); // CLASS IN - INTERNET

    memcpy(buffer + size, &QTYPE, sizeof(QTYPE));
    size += sizeof(QTYPE);
    memcpy(buffer + size, &QCLASS, sizeof(QCLASS));
    size += sizeof(QCLASS);

    return size;
}

typedef struct {
    uint8_t octets[4];
} ip_t;

uint8_t* skip_dns_name(uint8_t* buffer)
{
    uint8_t* pos = buffer;
    if (*pos & (0x3 << 6)) // COMPRESSED FORM
        return pos + 2;

    while (*pos)
        pos += *pos + 1;

    return pos + 1;
}

int parse_dns_response(uint8_t* buffer, ip_t* ip)
{
    dns_header_t* header = (dns_header_t*)buffer;

    header->ID = ntohs(header->ID);
    header->FLAGS = ntohs(header->FLAGS);
    header->QDCOUNT = ntohs(header->QDCOUNT);
    header->ANCOUNT = ntohs(header->ANCOUNT);
    header->NSCOUNT = ntohs(header->QDCOUNT);
    header->ARCOUNT = ntohs(header->QDCOUNT);

    if (!(header->FLAGS & (1 << 15))) // ITS RESPONSE
        return -1;

    if (header->FLAGS & (0xF << 0)) // NO ERRORS OCCURED
        return -1;

    uint8_t* pos = buffer + sizeof(dns_header_t);
    for (uint16_t i = 0; i < header->QDCOUNT; ++i) {
        pos = skip_dns_name(pos);
        pos += 2 * sizeof(uint16_t); // SKIP QTYPE AND QCLASS
    }

    for (uint16_t i = 0; i < header->ANCOUNT; ++i) {
        pos = skip_dns_name(pos);

        uint16_t type = 0;
        memcpy(&type, pos, sizeof(type));
        type = ntohs(type);

        pos += 4 * sizeof(uint16_t); // SKIP SOME FIELDS

        uint16_t rdlength = 0;
        memcpy(&rdlength, pos, sizeof(rdlength));
        rdlength = ntohs(rdlength);

        pos += sizeof(rdlength);

        if (type == 0x0001 && rdlength == 4) { // ARECORD WITH IP
            memcpy(ip->octets, pos, rdlength);
            return 0;
        }

        pos += rdlength;
    }

    return -1;
}

int main()
{
    const int port = 53;
    const char* address = "8.8.8.8";

    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(port)};
    if (inet_aton(address, &addr.sin_addr) != 1)
        return 1;

    int sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd == -1)
        return 1;

    if (connect(sfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        close(sfd);
        return 1;
    }

    // result - 200KB, lol
    /*
    int max_len = 2048;
    socklen_t size = sizeof(max_len);
    if (getsockopt(sfd, SOL_SOCKET, SO_SNDBUF, &max_len, &size) == -1) {
        close(sfd);
        return 1;
    }
    */

    uint8_t buffer[2048];
    char* name = NULL;
    size_t name_size = 0;

    int res = 0;
    while ((res = getline(&name, &name_size, stdin)) != -1) {
        if (res > 256) {
            free(name);
            close(sfd);
            return 1;
        }

        if (res == 0)
            continue;

        if (name[res - 1] == '\n')
            name[res - 1] = 0;

        int size = build_dns_request(name, buffer);
        if (write(sfd, buffer, size) == -1) {
            free(name);
            close(sfd);
            return 1;
        }

        if (read(sfd, buffer, sizeof(buffer)) == -1) {
            free(name);
            close(sfd);
            return 1;
        }

        ip_t ip;
        if (parse_dns_response(buffer, &ip) == -1) {
            free(name);
            close(sfd);
            return 1;
        }

        printf(
            "%d.%d.%d.%d\n",
            ip.octets[0],
            ip.octets[1],
            ip.octets[2],
            ip.octets[3]);
    }

    free(name);
    close(sfd);

    return 0;
}
