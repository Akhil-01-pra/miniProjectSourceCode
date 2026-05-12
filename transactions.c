#include "bank.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void appendAudit(const char *message) {
    FILE *file = fopen(AUDIT_FILE, "a");
    if (!file) return;
    fprintf(file, "%s - %s\n", ctime(&(time_t){time(NULL)}), message);
    fclose(file);
}

void recordTransaction(const char *account_no, const char *type, double amount, const char *target_account, double balance_after) {
    Transaction record = {0};
    strcpy(record.account_no, account_no);
    strcpy(record.type, type);
    record.amount = amount;
    strncpy(record.target_account, target_account, MAX_ACCOUNT_NO - 1);
    record.balance_after = balance_after;
    record.timestamp = time(NULL);
    saveRecord(TRANSACTION_FILE, &record, sizeof(Transaction));
    char audit[256];
    snprintf(audit, sizeof(audit), "Transaction recorded: %s %s amount %.2f on %s", account_no, type, amount, target_account);
    appendAudit(audit);
}

static int loadCustomerByAccount(const char *account_no, Customer *customer, size_t *index) {
    return loadCustomerByAccountNo(account_no, customer, index);
}

static double calculateDailyWithdrawal(const char *account_no) {
    size_t count = 0;
    if (!readAllRecords(TRANSACTION_FILE, NULL, sizeof(Transaction), &count)) return 0.0;
    if (count == 0) return 0.0;
    Transaction *history = malloc(count * sizeof(Transaction));
    if (!history) return 0.0;
    if (!readAllRecords(TRANSACTION_FILE, history, sizeof(Transaction), &count)) {
        free(history);
        return 0.0;
    }
    double total = 0.0;
    time_t today = time(NULL);
    struct tm now;
#ifdef _WIN32
    localtime_s(&now, &today);
#else
    localtime_r(&today, &now);
#endif
    for (size_t i = 0; i < count; i++) {
        if (strcmp(history[i].account_no, account_no) != 0) continue;
        if (strcmp(history[i].type, "Withdrawal") != 0 && strcmp(history[i].type, "ATM Withdrawal") != 0) continue;
        struct tm txn;
#ifdef _WIN32
        localtime_s(&txn, &history[i].timestamp);
#else
        localtime_r(&history[i].timestamp, &txn);
#endif
        if (txn.tm_year == now.tm_year && txn.tm_mon == now.tm_mon && txn.tm_mday == now.tm_mday) {
            total += history[i].amount;
        }
    }
    free(history);
    return total;
}

int depositToAccount(Customer *customer, double amount) {
    if (amount <= 0.0) {
        printColored("Invalid deposit amount.\n", 12);
        return 0;
    }
    size_t index;
    if (!loadCustomerByAccountNo(customer->account_no, customer, &index)) {
        printColored("Unable to update customer record.\n", 12);
        return 0;
    }
    customer->balance += amount;
    if (!updateCustomer(customer, index)) {
        printColored("Failed to save deposit.\n", 12);
        return 0;
    }
    recordTransaction(customer->account_no, "Deposit", amount, "Self", customer->balance);
    printColored("Deposit successful.\n", 10);
    return 1;
}

int withdrawFromAccount(Customer *customer, double amount) {
    if (amount <= 0.0) {
        printColored("Invalid withdrawal amount.\n", 12);
        return 0;
    }
    size_t index;
    if (!loadCustomerByAccountNo(customer->account_no, customer, &index)) {
        printColored("Unable to update customer record.\n", 12);
        return 0;
    }
    if (amount > customer->balance) {
        printColored("Insufficient balance.\n", 12);
        return 0;
    }
    double dailyTotal = calculateDailyWithdrawal(customer->account_no);
    if (dailyTotal + amount > DAILY_WITHDRAWAL_LIMIT) {
        printColored("Daily withdrawal limit reached.\n", 12);
        return 0;
    }
    customer->balance -= amount;
    if (!updateCustomer(customer, index)) {
        printColored("Failed to save withdrawal.\n", 12);
        return 0;
    }
    recordTransaction(customer->account_no, "Withdrawal", amount, "Self", customer->balance);
    printColored("Withdrawal successful.\n", 10);
    return 1;
}

int transferMoney(Customer *customer, const char *target_account, double amount) {
    if (strcmp(customer->account_no, target_account) == 0) {
        printColored("Cannot transfer to same account.\n", 12);
        return 0;
    }
    if (amount <= 0.0) {
        printColored("Invalid transfer amount.\n", 12);
        return 0;
    }
    Customer receiver = {0};
    size_t targetIndex;
    if (!loadCustomerByAccountNo(target_account, &receiver, &targetIndex)) {
        printColored("Target account not found.\n", 12);
        return 0;
    }
    if (!receiver.is_active || receiver.is_frozen) {
        printColored("Target account is not available for transfer.\n", 12);
        return 0;
    }
    size_t sourceIndex;
    if (!loadCustomerByAccountNo(customer->account_no, customer, &sourceIndex)) {
        printColored("Unable to update source account.\n", 12);
        return 0;
    }
    if (amount > customer->balance) {
        printColored("Insufficient balance.\n", 12);
        return 0;
    }
    customer->balance -= amount;
    receiver.balance += amount;
    if (!updateCustomer(customer, sourceIndex) || !updateCustomer(&receiver, targetIndex)) {
        printColored("Transfer update failed.\n", 12);
        return 0;
    }
    recordTransaction(customer->account_no, "Transfer Sent", amount, target_account, customer->balance);
    recordTransaction(target_account, "Transfer Received", amount, customer->account_no, receiver.balance);
    printColored("Transfer completed successfully.\n", 10);
    return 1;
}

void showMiniStatement(const char *account_no) {
    printHeader("Mini Statement");
    size_t count = 0;
    if (!readAllRecords(TRANSACTION_FILE, NULL, sizeof(Transaction), &count) || count == 0) {
        printf("No transactions found.\n");
        return;
    }
    Transaction *history = malloc(count * sizeof(Transaction));
    if (!history) {
        printf("Unable to allocate memory.\n");
        return;
    }
    if (!readAllRecords(TRANSACTION_FILE, history, sizeof(Transaction), &count)) {
        free(history);
        printf("Unable to read transactions.\n");
        return;
    }
    int shown = 0;
    printf("%-20s %-15s %-12s %-12s %-10s\n", "Date", "Type", "Amount", "Balance", "Target");
    for (long i = count - 1; i >= 0 && shown < 5; i--) {
        if (strcmp(history[i].account_no, account_no) != 0) continue;
        struct tm txn;
#ifdef _WIN32
        localtime_s(&txn, &history[i].timestamp);
#else
        localtime_r(&history[i].timestamp, &txn);
#endif
        char date[20];
        strftime(date, sizeof(date), "%d-%b-%Y", &txn);
        printf("%-20s %-15s %-12.2f %-12.2f %-10s\n", date, history[i].type, history[i].amount, history[i].balance_after,
               history[i].target_account[0] ? history[i].target_account : "Self");
        shown++;
    }
    if (shown == 0) {
        printf("No recent transactions available.\n");
    }
    free(history);
}

void showFullStatement(const char *account_no) {
    printHeader("Full Transaction History");
    size_t count = 0;
    if (!readAllRecords(TRANSACTION_FILE, NULL, sizeof(Transaction), &count) || count == 0) {
        printf("No transactions found.\n");
        return;
    }
    Transaction *history = malloc(count * sizeof(Transaction));
    if (!history) {
        printf("Unable to allocate memory.\n");
        return;
    }
    if (!readAllRecords(TRANSACTION_FILE, history, sizeof(Transaction), &count)) {
        free(history);
        printf("Unable to read transactions.\n");
        return;
    }
    printf("%-20s %-15s %-12s %-12s %-10s\n", "Date", "Type", "Amount", "Balance", "Target");
    for (size_t i = 0; i < count; i++) {
        if (strcmp(history[i].account_no, account_no) != 0) continue;
        struct tm txn;
#ifdef _WIN32
        localtime_s(&txn, &history[i].timestamp);
#else
        localtime_r(&history[i].timestamp, &txn);
#endif
        char date[20];
        strftime(date, sizeof(date), "%d-%b-%Y %H:%M", &txn);
        printf("%-20s %-15s %-12.2f %-12.2f %-10s\n", date, history[i].type, history[i].amount,
               history[i].balance_after, history[i].target_account[0] ? history[i].target_account : "Self");
    }
    free(history);
}
