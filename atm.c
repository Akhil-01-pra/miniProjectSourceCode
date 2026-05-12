#include "bank.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int loadATMRecord(const char *account_no, ATMRecord *record, size_t *index) {
    size_t count = 0;
    if (!readAllRecords(ATM_FILE, NULL, sizeof(ATMRecord), &count)) return 0;
    if (count == 0) return 0;
    ATMRecord *records = malloc(count * sizeof(ATMRecord));
    if (!records) return 0;
    if (!readAllRecords(ATM_FILE, records, sizeof(ATMRecord), &count)) {
        free(records);
        return 0;
    }
    int found = 0;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(records[i].account_no, account_no) == 0) {
            *record = records[i];
            *index = i;
            found = 1;
            break;
        }
    }
    free(records);
    return found;
}

int updateATMRecord(const ATMRecord *record, size_t index) {
    return updateRecordByIndex(ATM_FILE, index, record, sizeof(ATMRecord));
}

int initializeATMForCustomer(const Customer *customer) {
    ATMRecord atmRecord = {0};
    strcpy(atmRecord.account_no, customer->account_no);
    strcpy(atmRecord.pin_hash, customer->atm_pin_hash);
    atmRecord.failed_attempts = 0;
    atmRecord.is_locked = 0;
    atmRecord.last_change = time(NULL);
    return saveRecord(ATM_FILE, &atmRecord, sizeof(ATMRecord));
}

static int verifyATMPin(const char *account_no) {
    ATMRecord record = {0};
    size_t index;
    if (!loadATMRecord(account_no, &record, &index)) {
        printColored("ATM record not found.\n", 12);
        return 0;
    }
    if (record.is_locked) {
        printColored("ATM features locked due to repeated failed PIN attempts.\n", 12);
        return 0;
    }
    char pin[8];
    maskInput(pin, 8, "Enter ATM PIN: ");
    char pinHash[65];
    hashString(pin, pinHash);
    if (!secureCompare(pinHash, record.pin_hash)) {
        record.failed_attempts++;
        if (record.failed_attempts >= 3) {
            record.is_locked = 1;
            printColored("ATM PIN locked after 3 failed attempts. Contact bank support.\n", 12);
        } else {
            printColored("Invalid ATM PIN.\n", 12);
        }
        updateATMRecord(&record, index);
        return 0;
    }
    record.failed_attempts = 0;
    updateATMRecord(&record, index);
    return 1;
}

int changeATMPin(Customer *customer) {
    printHeader("Change ATM PIN");
    if (!verifyATMPin(customer->account_no)) return 0;
    char currentPin[8];
    maskInput(currentPin, 8, "Enter current PIN again: ");
    char currentHash[65];
    hashString(currentPin, currentHash);
    if (!secureCompare(currentHash, customer->atm_pin_hash)) {
        printColored("Current ATM PIN mismatch.\n", 12);
        return 0;
    }
    char newPin[8];
    char confirmPin[8];
    maskInput(newPin, 8, "Enter new 4-digit PIN: ");
    if (strlen(newPin) != 4) {
        printColored("PIN must be 4 digits.\n", 12);
        return 0;
    }
    maskInput(confirmPin, 8, "Confirm new PIN: ");
    if (strcmp(newPin, confirmPin) != 0) {
        printColored("PIN entries do not match.\n", 12);
        return 0;
    }
    char pinHash[65];
    hashString(newPin, pinHash);
    strcpy(customer->atm_pin_hash, pinHash);
    size_t index;
    if (!loadCustomerByAccountNo(customer->account_no, customer, &index)) {
        printColored("Failed to locate customer.\n", 12);
        return 0;
    }
    if (!updateCustomer(customer, index)) {
        printColored("Failed to update customer ATM PIN.\n", 12);
        return 0;
    }
    ATMRecord record = {0};
    size_t atmIndex;
    if (!loadATMRecord(customer->account_no, &record, &atmIndex)) {
        printColored("Failed to update ATM record.\n", 12);
        return 0;
    }
    strcpy(record.pin_hash, pinHash);
    record.failed_attempts = 0;
    record.last_change = time(NULL);
    updateATMRecord(&record, atmIndex);
    printColored("ATM PIN changed successfully.\n", 10);
    return 1;
}

int recoverATMPin(Customer *customer) {
    printHeader("Recover ATM PIN");
    char answer[MAX_PASS];
    printf("Security question: %s\n", customer->security_question);
    maskInput(answer, MAX_PASS, "Enter answer: ");
    char answerHash[65];
    hashString(answer, answerHash);
    if (!secureCompare(answerHash, customer->security_answer_hash)) {
        printColored("Security answer does not match.\n", 12);
        return 0;
    }
    int otp = rand() % 9000 + 1000;
    printf("Simulated OTP sent to registered phone number: %d\n", otp);
    int enteredOtp;
    printf("Enter OTP: ");
    scanf("%d", &enteredOtp);
    getchar();
    if (enteredOtp != otp) {
        printColored("OTP verification failed.\n", 12);
        return 0;
    }
    char newPin[8];
    char confirmPin[8];
    maskInput(newPin, 8, "Enter new 4-digit PIN: ");
    if (strlen(newPin) != 4) {
        printColored("PIN must be 4 digits.\n", 12);
        return 0;
    }
    maskInput(confirmPin, 8, "Confirm new PIN: ");
    if (strcmp(newPin, confirmPin) != 0) {
        printColored("PIN entries do not match.\n", 12);
        return 0;
    }
    char pinHash[65];
    hashString(newPin, pinHash);
    strcpy(customer->atm_pin_hash, pinHash);
    size_t index;
    if (!loadCustomerByAccountNo(customer->account_no, customer, &index)) {
        printColored("Failed to locate customer record.\n", 12);
        return 0;
    }
    if (!updateCustomer(customer, index)) {
        printColored("Failed to update customer ATM PIN.\n", 12);
        return 0;
    }
    ATMRecord record = {0};
    size_t atmIndex;
    if (!loadATMRecord(customer->account_no, &record, &atmIndex)) {
        printColored("Failed to locate ATM record.\n", 12);
        return 0;
    }
    strcpy(record.pin_hash, pinHash);
    record.failed_attempts = 0;
    record.is_locked = 0;
    record.last_change = time(NULL);
    updateATMRecord(&record, atmIndex);
    printColored("ATM PIN recovered and reset successfully.\n", 10);
    return 1;
}

int withdrawFromATM(Customer *customer) {
    printHeader("ATM Cash Withdrawal");
    if (!verifyATMPin(customer->account_no)) return 0;
    double amount = 0.0;
    printf("Enter amount to withdraw: ");
    scanf("%lf", &amount);
    getchar();
    if (amount <= 0.0 || amount > customer->balance) {
        printColored("Invalid amount or insufficient funds.\n", 12);
        return 0;
    }
    if (amount > 10000.0) {
        printColored("ATM withdrawal limit is 10,000 per transaction.\n", 12);
        return 0;
    }
    if (!withdrawFromAccount(customer, amount)) {
        return 0;
    }
    printColored("Please collect your cash.\n", 10);
    return 1;
}
