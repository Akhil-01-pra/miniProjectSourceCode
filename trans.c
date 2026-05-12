// Bank-account transaction program with extended features.
// Adds account management, search, summary and database initialization.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ACCOUNTS 100
#define LAST_NAME_SIZE 15
#define FIRST_NAME_SIZE 10
#define BUFFER_SIZE 128

// clientData structure definition
struct clientData
{
    unsigned int acctNum;                 // account number
    char lastName[LAST_NAME_SIZE];        // account last name
    char firstName[FIRST_NAME_SIZE];      // account first name
    double balance;                       // account balance
};

// prototypes
unsigned int enterChoice(void);
void initializeDatabase(FILE *fPtr);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void listAccounts(FILE *fPtr);
void searchByAccount(FILE *fPtr);
void searchByLastName(FILE *fPtr);
void accountSummary(FILE *fPtr);
unsigned int safeReadUnsignedInt(const char *prompt, unsigned int lower, unsigned int upper);
double safeReadDouble(const char *prompt);
void readString(const char *prompt, char *buffer, size_t size);
int compareIgnoreCase(const char *a, const char *b);

int main(void)
{
    FILE *cfPtr;                        // credit.dat file pointer
    unsigned int choice;                // user's choice

    cfPtr = fopen("credit.dat", "rb+");

    if (cfPtr == NULL)
    {
        puts("credit.dat not found. Creating a new database file with blank records...");

        cfPtr = fopen("credit.dat", "wb+");
        if (cfPtr == NULL)
        {
            fputs("Unable to create credit.dat. Exiting.\n", stderr);
            return EXIT_FAILURE;
        }

        initializeDatabase(cfPtr);
        rewind(cfPtr);
    }

    while ((choice = enterChoice()) != 9)
    {
        switch (choice)
        {
            case 1:
                textFile(cfPtr);
                break;
            case 2:
                updateRecord(cfPtr);
                break;
            case 3:
                newRecord(cfPtr);
                break;
            case 4:
                deleteRecord(cfPtr);
                break;
            case 5:
                listAccounts(cfPtr);
                break;
            case 6:
                searchByAccount(cfPtr);
                break;
            case 7:
                searchByLastName(cfPtr);
                break;
            case 8:
                accountSummary(cfPtr);
                break;
            default:
                puts("Incorrect choice. Please enter a number between 1 and 9.");
                break;
        }
    }

    fclose(cfPtr);
    puts("Program ended. Goodbye!");
    return EXIT_SUCCESS;
}

void initializeDatabase(FILE *fPtr)
{
    struct clientData blankClient = {0, "", "", 0.0};
    unsigned int i;

    rewind(fPtr);
    for (i = 0; i < MAX_ACCOUNTS; ++i)
    {
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    }
    fflush(fPtr);
}

void textFile(FILE *readPtr)
{
    FILE *writePtr;
    struct clientData client = {0, "", "", 0.0};
    size_t result;
    unsigned int count = 0;
    double totalBalance = 0.0;

    writePtr = fopen("accounts.txt", "w");
    if (writePtr == NULL)
    {
        puts("Unable to open accounts.txt for writing.");
        return;
    }

    rewind(readPtr);
    fprintf(writePtr, "%-6s%-16s%-11s%12s\n", "Acct", "Last Name", "First Name", "Balance");
    fprintf(writePtr, "%-6s%-16s%-11s%12s\n", "-----", "---------", "----------", "-----------");

    while ((result = fread(&client, sizeof(struct clientData), 1, readPtr)) == 1)
    {
        if (client.acctNum != 0)
        {
            fprintf(writePtr, "%-6u%-16s%-11s%12.2f\n", client.acctNum, client.lastName, client.firstName,
                    client.balance);
            totalBalance += client.balance;
            ++count;
        }
    }

    fprintf(writePtr, "\nTotal active accounts: %u\n", count);
    fprintf(writePtr, "Total balance: %.2f\n", totalBalance);
    if (count > 0)
    {
        fprintf(writePtr, "Average balance: %.2f\n", totalBalance / count);
    }

    fclose(writePtr);
    puts("Formatted text file 'accounts.txt' has been created.");
}

void updateRecord(FILE *fPtr)
{
    unsigned int account = safeReadUnsignedInt("Enter account to update (1-100): ", 1, MAX_ACCOUNTS);
    struct clientData client = {0, "", "", 0.0};
    double transaction;
    long offset = (long)(account - 1) * sizeof(struct clientData);

    fseek(fPtr, offset, SEEK_SET);
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        puts("Unable to read the requested account.");
        return;
    }

    if (client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
        return;
    }

    printf("Current account details:\n");
    printf("%-6s%-16s%-11s%12s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("%-6u%-16s%-11s%12.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

    transaction = safeReadDouble("Enter transaction amount (positive deposit, negative withdrawal): ");
    client.balance += transaction;

    fseek(fPtr, offset, SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);
    fflush(fPtr);

    printf("Updated balance for account #%u: %.2f\n", client.acctNum, client.balance);
}

void deleteRecord(FILE *fPtr)
{
    unsigned int accountNum = safeReadUnsignedInt("Enter account number to delete (1-100): ", 1, MAX_ACCOUNTS);
    struct clientData client = {0, "", "", 0.0};
    struct clientData blankClient = {0, "", "", 0.0};
    long offset = (long)(accountNum - 1) * sizeof(struct clientData);
    char input[BUFFER_SIZE];

    fseek(fPtr, offset, SEEK_SET);
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1 || client.acctNum == 0)
    {
        printf("Account #%u does not exist.\n", accountNum);
        return;
    }

    printf("Account found: %u %s %s %.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
    readString("Type YES to confirm deletion: ", input, sizeof(input));

    if (compareIgnoreCase(input, "YES") != 0)
    {
        puts("Deletion cancelled.");
        return;
    }

    fseek(fPtr, offset, SEEK_SET);
    fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    fflush(fPtr);
    printf("Account #%u has been deleted.\n", accountNum);
}

void newRecord(FILE *fPtr)
{
    unsigned int accountNum = safeReadUnsignedInt("Enter new account number (1-100): ", 1, MAX_ACCOUNTS);
    struct clientData client = {0, "", "", 0.0};
    long offset = (long)(accountNum - 1) * sizeof(struct clientData);

    fseek(fPtr, offset, SEEK_SET);
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        puts("Unable to read the requested record.");
        return;
    }

    if (client.acctNum != 0)
    {
        printf("Account #%u already contains information.\n", client.acctNum);
        return;
    }

    readString("Enter last name: ", client.lastName, sizeof(client.lastName));
    readString("Enter first name: ", client.firstName, sizeof(client.firstName));
    client.balance = safeReadDouble("Enter opening balance: ");
    client.acctNum = accountNum;

    fseek(fPtr, offset, SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);
    fflush(fPtr);

    printf("Account #%u created for %s %s with balance %.2f\n", client.acctNum, client.firstName,
           client.lastName, client.balance);
}

void listAccounts(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int count = 0;
    double totalBalance = 0.0;

    rewind(fPtr);
    printf("%-6s%-16s%-11s%12s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("%-6s%-16s%-11s%12s\n", "-----", "---------", "----------", "-----------");

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            printf("%-6u%-16s%-11s%12.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
            totalBalance += client.balance;
            ++count;
        }
    }

    if (count == 0)
    {
        puts("No active accounts found.");
    }
    else
    {
        printf("\nActive accounts: %u\n", count);
        printf("Total balance: %.2f\n", totalBalance);
    }
}

void searchByAccount(FILE *fPtr)
{
    unsigned int accountNum = safeReadUnsignedInt("Enter account number to search (1-100): ", 1, MAX_ACCOUNTS);
    struct clientData client = {0, "", "", 0.0};
    long offset = (long)(accountNum - 1) * sizeof(struct clientData);

    fseek(fPtr, offset, SEEK_SET);
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1 || client.acctNum == 0)
    {
        printf("Account #%u not found.\n", accountNum);
        return;
    }

    printf("Account details:\n");
    printf("%-6s%-16s%-11s%12s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("%-6u%-16s%-11s%12.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
}

void searchByLastName(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    char searchName[LAST_NAME_SIZE];
    unsigned int count = 0;

    readString("Enter last name to search: ", searchName, sizeof(searchName));
    rewind(fPtr);

    printf("%-6s%-16s%-11s%12s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("%-6s%-16s%-11s%12s\n", "-----", "---------", "----------", "-----------");

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (client.acctNum != 0 && compareIgnoreCase(client.lastName, searchName) == 0)
        {
            printf("%-6u%-16s%-11s%12.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
            ++count;
        }
    }

    if (count == 0)
    {
        printf("No accounts found with last name '%s'.\n", searchName);
    }
    else
    {
        printf("\nFound %u account(s) matching '%s'.\n", count, searchName);
    }
}

void accountSummary(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int count = 0;
    double totalBalance = 0.0;
    double minBalance = 0.0;
    double maxBalance = 0.0;

    rewind(fPtr);
    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            if (count == 0 || client.balance < minBalance)
            {
                minBalance = client.balance;
            }
            if (count == 0 || client.balance > maxBalance)
            {
                maxBalance = client.balance;
            }
            totalBalance += client.balance;
            ++count;
        }
    }

    puts("Account summary:");
    printf("Active accounts: %u\n", count);
    printf("Total balance: %.2f\n", totalBalance);
    if (count > 0)
    {
        printf("Average balance: %.2f\n", totalBalance / count);
        printf("Lowest balance: %.2f\n", minBalance);
        printf("Highest balance: %.2f\n", maxBalance);
    }
}

unsigned int safeReadUnsignedInt(const char *prompt, unsigned int lower, unsigned int upper)
{
    char buffer[BUFFER_SIZE];
    unsigned int value;

    while (1)
    {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
            clearerr(stdin);
            continue;
        }

        if (sscanf(buffer, "%u", &value) == 1 && value >= lower && value <= upper)
        {
            return value;
        }

        printf("Invalid entry. Please enter a number between %u and %u.\n", lower, upper);
    }
}

double safeReadDouble(const char *prompt)
{
    char buffer[BUFFER_SIZE];
    double value;

    while (1)
    {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
            clearerr(stdin);
            continue;
        }

        if (sscanf(buffer, "%lf", &value) == 1)
        {
            return value;
        }

        puts("Invalid number. Please enter a valid decimal value.");
    }
}

void readString(const char *prompt, char *buffer, size_t size)
{
    char temp[BUFFER_SIZE];

    while (1)
    {
        printf("%s", prompt);
        if (fgets(temp, sizeof(temp), stdin) == NULL)
        {
            clearerr(stdin);
            continue;
        }

        if (temp[0] == '\n')
        {
            puts("Input cannot be empty.");
            continue;
        }

        temp[strcspn(temp, "\n")] = '\0';
        strncpy(buffer, temp, size - 1);
        buffer[size - 1] = '\0';
        return;
    }
}

int compareIgnoreCase(const char *a, const char *b)
{
    while (*a && *b)
    {
        char ca = (char)tolower((unsigned char)*a);
        char cb = (char)tolower((unsigned char)*b);
        if (ca != cb)
        {
            return ca - cb;
        }
        a++;
        b++;
    }
    return (int)((unsigned char)tolower((unsigned char)*a) - (unsigned char)tolower((unsigned char)*b));
}

unsigned int enterChoice(void)
{
    puts("\nBank Account Transaction Menu");
    puts("1 - Export accounts to accounts.txt");
    puts("2 - Update an account");
    puts("3 - Add a new account");
    puts("4 - Delete an account");
    puts("5 - List all active accounts");
    puts("6 - Search account by number");
    puts("7 - Search accounts by last name");
    puts("8 - Show account summary");
    puts("9 - Exit program");

    return safeReadUnsignedInt("Enter your choice: ", 1, 9);
}
