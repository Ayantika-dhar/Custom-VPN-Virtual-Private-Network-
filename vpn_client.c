#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>
#include <openssl/aes.h>

#define SERVER_IP "192.168.1.1"  // Change this to your VPN server IP
#define PORT 12345
#define BUFFER_SIZE 4096
#define AES_KEY_SIZE 128

unsigned char aes_key[AES_BLOCK_SIZE] = "0123456789abcdef"; // 16-byte key

int tun_alloc(char *dev) {
    struct ifreq ifr;
    int fd = open("/dev/net/tun", O_RDWR);
    if (fd < 0) { perror("Open TUN"); exit(1); }
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    if (ioctl(fd, TUNSETIFF, (void *)&ifr) < 0) { perror("TUNSETIFF"); exit(1); }
    strcpy(dev, ifr.ifr_name);
    return fd;
}

void encrypt_data(unsigned char *input, unsigned char *output, int len) {
    AES_KEY enc_key;
    AES_set_encrypt_key(aes_key, AES_KEY_SIZE, &enc_key);
    AES_encrypt(input, output, &enc_key);
}

int main() {
    char tun_name[IFNAMSIZ] = "tun0";
    int tun_fd = tun_alloc(tun_name);
    printf("VPN Client: TUN device %s allocated\n", tun_name);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        exit(1);
    }
    printf("Connected to VPN server\n");

    unsigned char buffer[BUFFER_SIZE], encrypted[BUFFER_SIZE];
    while (1) {
        int bytes = read(tun_fd, buffer, AES_BLOCK_SIZE);
        if (bytes <= 0) break;

        encrypt_data(buffer, encrypted, AES_BLOCK_SIZE);
        write(sockfd, encrypted, AES_BLOCK_SIZE);
    }

    close(sockfd);
    close(tun_fd);
    return 0;
}
