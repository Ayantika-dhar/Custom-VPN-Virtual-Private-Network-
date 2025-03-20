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

void decrypt_data(unsigned char *input, unsigned char *output, int len) {
    AES_KEY dec_key;
    AES_set_decrypt_key(aes_key, AES_KEY_SIZE, &dec_key);
    AES_decrypt(input, output, &dec_key);
}

int main() {
    char tun_name[IFNAMSIZ] = "tun0";
    int tun_fd = tun_alloc(tun_name);
    printf("VPN Server: TUN device %s allocated\n", tun_name);

    int sockfd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(sockfd, 5);
    printf("VPN Server: Listening on port %d...\n", PORT);
    
    client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
    printf("Client connected!\n");

    unsigned char buffer[BUFFER_SIZE], decrypted[BUFFER_SIZE];
    while (1) {
        int bytes = read(client_fd, buffer, AES_BLOCK_SIZE);
        if (bytes <= 0) break;

        decrypt_data(buffer, decrypted, AES_BLOCK_SIZE);
        write(tun_fd, decrypted, AES_BLOCK_SIZE);
    }

    close(client_fd);
    close(sockfd);
    close(tun_fd);
    return 0;
}
