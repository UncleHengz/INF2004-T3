#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

void hexToText(const char *hexString, char *output) {
    int len = strlen(hexString);
    
    // Ensure the length is even
    if (len % 2 != 0) {
        printf("Error: Hexadecimal string must have an even number of characters.\n");
        return;
    }

    for (int i = 0; i < len; i += 2) {
        // Extract two characters from the hex string
        char hex[3] = {hexString[i], hexString[i + 1], '\0'};
        
        // Convert the hex string to an integer
        int value;
        sscanf(hex, "%x", &value);

        // Convert the integer to a character and append to the output string
        output[i / 2] = (char)value;
    }

    // Null-terminate the output string
    output[len / 2] = '\0';
}

int main() {
    stdio_init_all();
    sleep_ms(7000);
    // Example usage
    const char *hexString = "7569643d61646d696e2670617373773d61646d696e2662746e5375626d69743d4c6f67696e"; // Hex representation of "Hello World"
    char text[100]; // Adjust the size based on your needs

    hexToText(hexString, text);

    // Print the result
    printf("Hexadecimal: %s\nText: %s\n", hexString, text);

    while (1)
    {
        tight_loop_contents();
    }
    return 0;
}
