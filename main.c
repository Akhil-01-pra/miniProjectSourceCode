#include "bank.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern int registerCustomer(void);
extern int loadCustomerByUsername(const char *, Customer *, size_t *);
extern int loadCustomerByAccountNo(const char *, Customer *, size_t *);
extern int depositToAccount(Customer *, double);
extern int withdrawFromAccount(Customer *, double);
extern int transferMoney(Customer *, const char *, double);
extern void showCustomerProfile(const Customer *);
extern void showMiniStatement(const char *);
extern void showFullStatement(const char *);
extern int changeATMPin(Customer *);
extern int recoverATMPin(Customer *);
extern int withdrawFromATM(Customer *);
extern int requestLocker(Customer *);
extern int accessLocker(void);
extern int requestChequeBook(const Customer *);
extern void showChequeStatus(const char *);
extern void adminMenu(void);
extern void printHeader(const char *);

static int authenticateCustomer(Customer *customer) {
    char username[MAX_USERNAME];
    char password[MAX_PASS];
    printHeader("Customer Login");
    printf("Username: ");
    scanf("%31s", username);
    getchar();
    maskInput(password, MAX_PASS, "Password: ");
    size_t index;
    if (!loadCustomerByUsername(username, customer, &index)) {
        printColored("User not found.\n", 12);
        return 0;
    }
    if (!customer->is_active) {
        printColored("This account is not active.\n", 12);
        return 0;
    }
    if (customer->is_frozen) {
        printColored("This account is frozen. Contact admin.\n", 12);
        return 0;
    }
    char hash[65];
    hashString(password, hash);
    if (!secureCompare(hash, customer->password_hash)) {
        customer->login_attempts++;
        if (customer->login_attempts >= 3) {
            customer->is_frozen = 1;
            printColored("Too many failed login attempts. Account frozen.\n", 12);
        } else {
            printColored("Invalid password.\n", 12);
        }
        updateCustomer(customer, index);
        return 0;
    }
    customer->login_attempts = 0;
    updateCustomer(customer, index);
    return 1;
}

static void customerMenu(Customer *customer) {
    Session session = {time(NULL)};
    while (1) {
        if (time(NULL) - session.last_action > SESSION_TIMEOUT_SECONDS) {
            printColored("Session timed out. Logging out.\n", 12);
            waitEnter();
            break;
        }
        printHeader("Bank Customer Panel");
        printf("Welcome, %s\n\n", customer->full_name);
        printf("1. Balance enquiry\n");
        printf("2. Deposit money\n");
        printf("3. Withdraw money\n");
        printf("4. Transfer money\n");
        printf("5. Mini statement\n");
        printf("6. Full transaction history\n");
        printf("7. ATM services\n");
        printf("8. Request cheque book\n");
        printf("9. Locker services\n");
        printf("0. Logout\n");
        printf("Select option: ");
        int choice;
        scanf("%d", &choice);
        getchar();
        switch (choice) {
            case 1:
                showCustomerProfile(customer);
                break;
            case 2: {
                double amount;
                printf("Enter deposit amount: ");
                scanf("%lf", &amount);
                getchar();
                depositToAccount(customer, amount);
                break;
            }
            case 3: {
                double amount;
                printf("Enter withdrawal amount: ");
                scanf("%lf", &amount);
                getchar();
                withdrawFromAccount(customer, amount);
                break;
            }
            case 4: {
                char target[MAX_ACCOUNT_NO];
                double amount;
                printf("Enter target account number: ");
                scanf("%12s", target);
                getchar();
                printf("Enter transfer amount: ");
                scanf("%lf", &amount);
                getchar();
                transferMoney(customer, target, amount);
                break;
            }
            case 5:
                showMiniStatement(customer->account_no);
                break;
            case 6:
                showFullStatement(customer->account_no);
                break;
            case 7: {
                printHeader("ATM Services");
                printf("1. Withdraw from ATM\n");
                printf("2. Change ATM PIN\n");
                printf("3. Forgot ATM PIN\n");
                printf("0. Back\n");
                int atmChoice;
                scanf("%d", &atmChoice);
                getchar();
                if (atmChoice == 1) withdrawFromATM(customer);
                else if (atmChoice == 2) changeATMPin(customer);
                else if (atmChoice == 3) recoverATMPin(customer);
                break;
            }
            case 8:
                requestChequeBook(customer);
                break;
            case 9:
                requestLocker(customer);
                break;
            case 0:
                return;
            default:
                printColored("Invalid choice.\n", 12);
                break;
        }
        session.last_action = time(NULL);
        waitEnter();
    }
}

int requestChequeBook(const Customer *customer) {
    ChequeRequest request = {0};
    sprintf(request.request_id, "CQ%09u", rand() % 1000000000);
    strcpy(request.account_no, customer->account_no);
    strcpy(request.status, "Pending");
    request.request_time = time(NULL);
    if (!saveRecord(CHEQUE_FILE, &request, sizeof(ChequeRequest))) {
        printColored("Unable to submit cheque request.\n", 12);
        return 0;
    }
    printColored("Cheque book request submitted successfully.\n", 10);
    return 1;
}

void showChequeStatus(const char *account_no) {
    printHeader("Cheque Book Status");
    size_t count = 0;
    if (!readAllRecords(CHEQUE_FILE, NULL, sizeof(ChequeRequest), &count) || count == 0) {
        printf("No cheque requests found.\n");
        return;
    }
    ChequeRequest *requests = malloc(count * sizeof(ChequeRequest));
    if (!requests) {
        printf("Unable to allocate memory.\n");
        return;
    }
    if (!readAllRecords(CHEQUE_FILE, requests, sizeof(ChequeRequest), &count)) {
        free(requests);
        printf("Unable to read cheque records.\n");
        return;
    }
    int found = 0;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(requests[i].account_no, account_no) == 0) {
            printf("Request %s created on %sStatus: %s\n", requests[i].request_id,
                   ctime(&requests[i].request_time), requests[i].status);
            found = 1;
        }
    }
    if (!found) {
        printf("No cheque book requests for your account.\n");
    }
    free(requests);
}

static void showWelcomeScreen(void) {
    printHeader("Bank Management System");
    printf("Professional terminal-based banking software built in C.\n");
    printf("Secure login, account management, ATM services, cheque book and locker features.\n\n");
    printf("Developed for mini-project demonstration.\n");
    waitEnter();
}

int main(void) {
    srand((unsigned int)time(NULL));
    showWelcomeScreen();
    while (1) {
        printHeader("Bank Management System");
        printf("1. Customer Registration\n");
        printf("2. Customer Login\n");
        printf("3. Admin Login\n");
        printf("0. Exit\n");
        printf("Select option: ");
        int option;
        scanf("%d", &option);
        getchar();
        if (option == 1) {
            registerCustomer();
        } else if (option == 2) {
            Customer customer = {0};
            if (authenticateCustomer(&customer)) {
                customerMenu(&customer);
            }
        } else if (option == 3) {
            adminMenu();
        } else if (option == 0) {
            printColored("Thank you for using the Bank Management System.\n", 10);
            break;
        } else {
            printColored("Invalid menu choice.\n", 12);
        }
        waitEnter();
    }
    return 0;
}
