#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
int main(int argc, char *argv[]) {
    int fd;
    uint8_t plop = 0x0B; // The byte to send
    struct termios options;
    plop = (unsigned char)strtol(argv[1], NULL, 16);
    // Open the serial port for writing
    fd = open("/dev/ttyd2", O_WRONLY | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("Failed to open /dev/ttyd2");
        return 1;
    }

    // Get the current options for the port
    tcgetattr(fd, &options);

    // Set the baud rates to 9600 (you may need to adjust this)
    cfsetispeed(&options, B38400);
    cfsetospeed(&options, B38400);

    // Set 8N1 mode: 8 data bits, no parity, 1 stop bit
    options.c_cflag &= ~PARENB; // No parity
    options.c_cflag &= ~CSTOPB; // 1 stop bit
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;     // 8 data bits
// Disable canonical mode, echo, and signal characters
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    // Disable input and output processing
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;

    // Disable onlcr (do not translate newline to carriage return + newline)
    options.c_oflag &= ~ONLCR;
    // Apply the options
	    tcsetattr(fd, TCSANOW, &options);

    // Write the byte to the serial port
    if (write(fd, &plop, sizeof(plop)) != 1) {
        perror("Failed to write byte to /dev/ttyd2");
        close(fd);
        return 1;
    }

    printf("Byte 0x%02X sent to /dev/ttyd2\n", plop);
    // Close the serial port
    close(fd);

    return 0;
}

