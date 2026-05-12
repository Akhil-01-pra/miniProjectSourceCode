#include "bank.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char hiddenAdminPasswordHash[] = "0e7517141fb53f21ee439b355b5a1d0a"; // hashed value for "Admin@123"

int adminLogin(void) {
    char username[MAX_USERNAME];
    char password[MAX_PASS];
    printHeader("Admin Login");
    printf("Username: ");
    scanf("%31s", username);
    getchar();
    maskInput(password, MAX_PASS, "Password: ");
    if (strcmp(username, "admin") != 0) {
        printColored("Unknown admin username.\n", 12);
        return 0;
    }
    char hash[65];
    hashString(password, hash);
    if (!secureCompare(hash, hiddenAdminPasswordHash)) {
        printColored("Invalid admin password.\n", 12);
        return 0;
    }
    return 1;
}

static void displayAccountSummary(const Customer *customer) {
    printf("%-12s %-15s %-8s %-10.2f %-8s\n", customer->account_no, customer->full_name,
           customer->account_type, customer->balance, customer->is_frozen ? "Frozen" : "Active");
}

static void listAllAccounts(void) {
    printHeader("All Customer Accounts");
    size_t count = 0;
    if (!readAllRecords(CUSTOMER_FILE, NULL, sizeof(Customer), &count) || count == 0) {
        printf("No accounts found.\n");
        return;
    }
    Customer *customers = malloc(count * sizeof(Customer));
    if (!customers) {
        printf("Memory error.\n");
        return;
    }
    if (!readAllRecords(CUSTOMER_FILE, customers, sizeof(Customer), &count)) {
        free(customers);
        printf("Unable to load accounts.\n");
        return;
    }
    printf("%-12s %-15s %-8s %-10s %-8s\n", "Account No", "Name", "Type", "Balance", "Status");
    for (size_t i = 0; i < count; i++) {
        if (customers[i].is_active) {
            displayAccountSummary(&customers[i]);
        }
    }
    free(customers);
}

static void searchCustomerAccount(void) {
    char query[MAX_USERNAME];
    printHeader("Search Customer Account");
    printf("Enter account number or username: ");
    scanf("%31s", query);
    getchar();
    size_t count = 0;
    if (!readAllRecords(CUSTOMER_FILE, NULL, sizeof(Customer), &count) || count == 0) {
        printf("No customer accounts available.\n");
        return;
    }
    Customer *customers = malloc(count * sizeof(Customer));
    if (!customers) {
        printf("Memory error.\n");
        return;
    }
    if (!readAllRecords(CUSTOMER_FILE, customers, sizeof(Customer), &count)) {
        free(customers);
        printf("Unable to load customers.\n");
        return;
    }
    int found = 0;
    for (size_t i = 0; i < count; i++) {
        if (!customers[i].is_active) continue;
        if (strcmp(customers[i].account_no, query) == 0 || strcmp(customers[i].username, query) == 0) {
            showCustomerProfile(&customers[i]);
            found = 1;
            break;
        }
    }
    if (!found) {
        printColored("Customer record not found.\n", 12);
    }
    free(customers);
}

static void toggleAccountFreeze(int freeze) {
    char account_no[MAX_ACCOUNT_NO];
    printHeader(freeze ? "Freeze Account" : "Unfreeze Account");
    printf("Enter account number: ");
    scanf("%12s", account_no);
    getchar();
    Customer customer = {0};
    size_t index;
    if (!loadCustomerByAccountNo(account_no, &customer, &index)) {
        printColored("Account not found.\n", 12);
        return;
    }
    customer.is_frozen = freeze;
    if (!updateCustomer(&customer, index)) {
        printColored("Unable to update account status.\n", 12);
        return;
    }
    printColored(freeze ? "Account frozen successfully.\n" : "Account unfrozen successfully.\n", 10);
}

static void deleteAccount(void) {
    char account_no[MAX_ACCOUNT_NO];
    printHeader("Delete Account");
    printf("Enter account number to delete: ");
    scanf("%12s", account_no);
    getchar();
    Customer customer = {0};
    size_t index;
    if (!loadCustomerByAccountNo(account_no, &customer, &index)) {
        printColored("Account not found.\n", 12);
        return;
    }
    customer.is_active = 0;
    if (!updateCustomer(&customer, index)) {
        printColored("Unable to delete account.\n", 12);
        return;
    }
    printColored("Account deleted successfully.\n", 10);
}

static void viewAllTransactions(void) {
    printHeader("All Transaction Records");
    size_t count = 0;
    if (!readAllRecords(TRANSACTION_FILE, NULL, sizeof(Transaction), &count) || count == 0) {
        printf("No transactions found.\n");
        return;
    }
    Transaction *history = malloc(count * sizeof(Transaction));
    if (!history) {
        printf("Memory allocation failed.\n");
        return;
    }
    if (!readAllRecords(TRANSACTION_FILE, history, sizeof(Transaction), &count)) {
        free(history);
        printf("Unable to read transactions.\n");
        return;
    }
    printf("%-12s %-15s %-10s %-10s %-10s\n", "Account", "Type", "Amount", "Balance", "Target");
    for (size_t i = 0; i < count; i++) {
        printf("%-12s %-15s %-10.2f %-10.2f %-10s\n", history[i].account_no,
               history[i].type, history[i].amount, history[i].balance_after,
               history[i].target_account[0] ? history[i].target_account : "Self");
    }
    free(history);
}

static void approveChequeRequests(void) {
    printHeader("Cheque Book Requests");
    size_t count = 0;
    if (!readAllRecords(CHEQUE_FILE, NULL, sizeof(ChequeRequest), &count) || count == 0) {
        printf("No cheque requests pending.\n");
        return;
    }
    ChequeRequest *requests = malloc(count * sizeof(ChequeRequest));
    if (!requests) {
        printf("Memory error.\n");
        return;
    }
    if (!readAllRecords(CHEQUE_FILE, requests, sizeof(ChequeRequest), &count)) {
        free(requests);
        printf("Unable to load cheque requests.\n");
        return;
    }
    int pendingCount = 0;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(requests[i].status, "Pending") == 0) {
            pendingCount++;
            printf("Request %s for %s created at %s", requests[i].request_id, requests[i].account_no,
                   ctime(&requests[i].request_time));
            printf("Approve (A) or Reject (R)? ");
            char choice = getchar();
            getchar();
            if (choice == 'A' || choice == 'a') {
                strcpy(requests[i].status, "Approved");
            } else {
                strcpy(requests[i].status, "Rejected");
            }
            updateRecordByIndex(CHEQUE_FILE, i, &requests[i], sizeof(ChequeRequest));
        }
    }
    if (pendingCount == 0) {
        printf("No pending cheque requests.\n");
    }
    free(requests);
}

static void generateBankReports(void) {
    printHeader("Bank Reports");
    size_t custCount = 0, txnCount = 0, lockerCount = 0;
    readAllRecords(CUSTOMER_FILE, NULL, sizeof(Customer), &custCount);
    readAllRecords(TRANSACTION_FILE, NULL, sizeof(Transaction), &txnCount);
    readAllRecords(LOCKER_FILE, NULL, sizeof(Locker), &lockerCount);
    printf("Total customer records: %zu\n", custCount);
    printf("Total transactions: %zu\n", txnCount);
    printf("Locker records: %zu\n", lockerCount);
}

void adminMenu(void) {
    if (!adminLogin()) {
        waitEnter();
        return;
    }
    Session session = {time(NULL)};
    while (1) {
        if (time(NULL) - session.last_action > SESSION_TIMEOUT_SECONDS) {
            printColored("Session timed out. Returning to main menu.\n", 12);
            waitEnter();
            break;
        }
        printHeader("Bank Admin Panel");
        printf("1. View all accounts\n");
        printf("2. Search customer account\n");
        printf("3. Freeze account\n");
        printf("4. Unfreeze account\n");
        printf("5. Delete account\n");
        printf("6. View all transactions\n");
        printf("7. Approve cheque requests\n");
        printf("8. Manage locker availability\n");
        printf("9. Generate reports\n");
        printf("0. Logout\n");
        printf("Select option: ");
        int choice;
        scanf("%d", &choice);
        getchar();
        switch (choice) {
            case 1: listAllAccounts(); break;
            case 2: searchCustomerAccount(); break;
            case 3: toggleAccountFreeze(1); break;
            case 4: toggleAccountFreeze(0); break;
            case 5: deleteAccount(); break;
            case 6: viewAllTransactions(); break;
            case 7: approveChequeRequests(); break;
            case 8: manageLockerAvailability(); break;
            case 9: generateBankReports(); break;
            case 0: return;
            default: printColored("Invalid choice.\n", 12); break;
        }
        session.last_action = time(NULL);
        waitEnter();
    }
}
