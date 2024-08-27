#include <stdio.h>

int main(int argc, char* argv[]) {
    
    // Checking if there are enough arguments
    if (argc < 2) {
        printf("Not enough arguments.\n");
        return 1;
    }
    
    // Taking the character from the command line
    char character = argv[1][0];

    // Checking if the character is capital 
    if (character >= 'A' && character <= 'Z') {
        printf("%c is a capital letter.\n", character);
    } 
    // Checking if the character is lowercase 
    else if (character >= 'a' && character <= 'z') {
        printf("%c is not a capital letter.\n", character);
    } 
    // If it's not a letter at all print error
    else {
        printf("%c is not a letter.\n", character);
    }

    return 0;
}
