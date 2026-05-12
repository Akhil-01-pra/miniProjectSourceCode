#include "bank.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

static void xorCipher(unsigned char *data, size_t len) {
    const char key[] = "Bank@2026Secret";
    size_t keyLen = strlen(key);
    for (size_t i = 0; i < len; ++i) {
        data[i] ^= key[i % keyLen];
    }
}

void clearConsole(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void waitEnter(void) {
    printf("\nPress ENTER to continue...");
    while (getchar() != '\n');
}

void printHeader(const char *title) {
    clearConsole();
    printf("========================================\n");
    printf("  %s\n", title);
    printf("========================================\n\n");
}

void printColored(const char *message, int color) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
    printf("%s", message);
    SetConsoleTextAttribute(hConsole, 7);
#else
    printf("%s", message);
#endif
}

static char getchPortable(void) {
#ifdef _WIN32
    return _getch();
#else
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
#endif
}

void maskInput(char *input, int size, const char *prompt) {
    printf("%s", prompt);
    int pos = 0;
    while (pos < size - 1) {
        char ch = getchPortable();
        if (ch == '\r' || ch == '\n') {
            break;
        }
        if (ch == '\b' || ch == 127) {
            if (pos > 0) {
                pos--;
                printf("\b \b");
            }
            continue;
        }
        if (isprint((unsigned char)ch)) {
            input[pos++] = ch;
            printf("*");
        }
    }
    input[pos] = '\0';
    printf("\n");
}

void hashString(const char *input, char *output) {
    unsigned long hash = 5381;
    for (size_t i = 0; i < strlen(input); i++) {
        hash = ((hash << 5) + hash) + (unsigned char)input[i];
    }
    sprintf(output, "%08lx", hash);
}

int secureCompare(const char *hash1, const char *hash2) {
    return strcmp(hash1, hash2) == 0;
}

int saveRecord(const char *filename, const void *record, size_t recSize) {
    FILE *file = fopen(filename, "ab");
    if (!file) {
        return 0;
    }
    unsigned char *buffer = malloc(recSize);
    if (!buffer) {
        fclose(file);
        return 0;
    }
    memcpy(buffer, record, recSize);
    xorCipher(buffer, recSize);
    size_t written = fwrite(buffer, recSize, 1, file);
    free(buffer);
    fclose(file);
    return written == 1;
}

int readAllRecords(const char *filename, void *buffer, size_t recSize, size_t *count) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        *count = 0;
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    if (fileSize < 0) {
        fclose(file);
        return 0;
    }
    *count = fileSize / recSize;
    if (*count == 0) {
        fclose(file);
        return 1;
    }
    if (!buffer) {
        fclose(file);
        return 1;
    }
    rewind(file);
    size_t readCount = fread(buffer, recSize, *count, file);
    fclose(file);
    if (readCount != *count) {
        return 0;
    }
    unsigned char *ptr = buffer;
    for (size_t i = 0; i < *count; ++i) {
        xorCipher(ptr + i * recSize, recSize);
    }
    return 1;
}

int updateRecordByIndex(const char *filename, size_t index, const void *record, size_t recSize) {
    FILE *file = fopen(filename, "rb+");
    if (!file) {
        return 0;
    }
    if (fseek(file, index * recSize, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    unsigned char *buffer = malloc(recSize);
    if (!buffer) {
        fclose(file);
        return 0;
    }
    memcpy(buffer, record, recSize);
    xorCipher(buffer, recSize);
    size_t written = fwrite(buffer, recSize, 1, file);
    free(buffer);
    fclose(file);
    return written == 1;
}

int validatePasswordStrength(const char *password) {
    int length = strlen(password);
    int hasUpper = 0, hasLower = 0, hasDigit = 0, hasSpecial = 0;
    for (int i = 0; i < length; i++) {
        char ch = password[i];
        if (isupper((unsigned char)ch)) hasUpper = 1;
        else if (islower((unsigned char)ch)) hasLower = 1;
        else if (isdigit((unsigned char)ch)) hasDigit = 1;
        else if (ispunct((unsigned char)ch)) hasSpecial = 1;
    }
    return length >= 8 && hasUpper && hasLower && hasDigit && hasSpecial;
}

int validPhoneNumber(const char *phone) {
    if (strlen(phone) < 10 || strlen(phone) > 13) return 0;
    for (size_t i = 0; i < strlen(phone); i++) {
        if (!isdigit((unsigned char)phone[i]) && phone[i] != '+') return 0;
    }
    return 1;
}

int validAadhaar(const char *aadhaar) {
    if (strlen(aadhaar) != 12) return 0;
    for (size_t i = 0; i < 12; i++) {
        if (!isdigit((unsigned char)aadhaar[i])) return 0;
    }
    return 1;
}

int validDate(const char *dob) {
    if (strlen(dob) != 10) return 0;
    if (dob[2] != '/' || dob[5] != '/') return 0;
    char temp[16];
    strncpy(temp, dob, sizeof(temp));
    temp[sizeof(temp) - 1] = '\0';
    char *token = strtok(temp, "/");
    if (!token) return 0;
    int day = atoi(token);
    token = strtok(NULL, "/");
    if (!token) return 0;
    int month = atoi(token);
    token = strtok(NULL, "/");
    if (!token) return 0;
    int year = atoi(token);
    if (day < 1 || day > 31) return 0;
    if (month < 1 || month > 12) return 0;
    if (year < 1900 || year > 2100) return 0;
    return 1;
}

int validAmount(const char *value) {
    char *end;
    double amount = strtod(value, &end);
    return end != value && *end == '\0' && amount >= 0.0;
}
