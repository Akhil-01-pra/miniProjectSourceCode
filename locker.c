#include "bank.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void generateLockerNumber(char locker_no[MAX_LOCKER_NO]) {
    sprintf(locker_no, "LK%09u", rand() % 1000000000);
}

static int loadLockerByNumber(const char *locker_no, Locker *locker, size_t *index) {
    size_t count = 0;
    if (!readAllRecords(LOCKER_FILE, NULL, sizeof(Locker), &count)) return 0;
    if (count == 0) return 0;
    Locker *records = malloc(count * sizeof(Locker));
    if (!records) return 0;
    if (!readAllRecords(LOCKER_FILE, records, sizeof(Locker), &count)) {
        free(records);
        return 0;
    }
    int found = 0;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(records[i].locker_no, locker_no) == 0) {
            *locker = records[i];
            *index = i;
            found = 1;
            break;
        }
    }
    free(records);
    return found;
}

static int saveLocker(const Locker *locker) {
    return saveRecord(LOCKER_FILE, locker, sizeof(Locker));
}

static int updateLocker(const Locker *locker, size_t index) {
    return updateRecordByIndex(LOCKER_FILE, index, locker, sizeof(Locker));
}

int requestLocker(Customer *customer) {
    printHeader("Locker Request");
    if (customer->locker_id[0] != '\0') {
        printColored("You already have a locker assigned.\n", 12);
        return 0;
    }
    Locker locker = {0};
    generateLockerNumber(locker.locker_no);
    strcpy(locker.account_no, customer->account_no);
    locker.is_allocated = 1;
    locker.rent_months = 12;
    locker.rent_due = 500.0 * locker.rent_months;
    locker.allocated_at = time(NULL);
    char accessCode[10];
    maskInput(accessCode, 10, "Create locker access code: ");
    hashString(accessCode, locker.access_code_hash);
    if (!saveLocker(&locker)) {
        printColored("Unable to allocate locker at the moment.\n", 12);
        return 0;
    }
    strcpy(customer->locker_id, locker.locker_no);
    size_t customerIndex;
    if (!loadCustomerByAccountNo(customer->account_no, customer, &customerIndex)) {
        printColored("Customer update failed.\n", 12);
        return 0;
    }
    if (!updateCustomer(customer, customerIndex)) {
        printColored("Failed to save locker assignment.\n", 12);
        return 0;
    }
    printf("Locker assigned successfully. Locker Number: %s\n", locker.locker_no);
    printf("Locker rent due: %.2f for %d months\n", locker.rent_due, locker.rent_months);
    return 1;
}

int accessLocker(void) {
    printHeader("Locker Access");
    char locker_no[MAX_LOCKER_NO];
    printf("Enter locker number: ");
    scanf("%12s", locker_no);
    getchar();
    Locker locker = {0};
    size_t index;
    if (!loadLockerByNumber(locker_no, &locker, &index)) {
        printColored("Locker not found.\n", 12);
        return 0;
    }
    if (!locker.is_allocated) {
        printColored("Locker is not currently allocated.\n", 12);
        return 0;
    }
    char accessCode[10];
    maskInput(accessCode, 10, "Enter locker access code: ");
    char codeHash[65];
    hashString(accessCode, codeHash);
    if (!secureCompare(codeHash, locker.access_code_hash)) {
        printColored("Invalid locker access code.\n", 12);
        return 0;
    }
    printf("Locker %s is allocated to account %s.\n", locker.locker_no, locker.account_no);
    printf("Rent due: %.2f\n", locker.rent_due);
    printf("Allocated on: %s", ctime(&locker.allocated_at));
    return 1;
}

int manageLockerAvailability(void) {
    printHeader("Locker Availability Report");
    size_t count = 0;
    if (!readAllRecords(LOCKER_FILE, NULL, sizeof(Locker), &count) || count == 0) {
        printf("No locker records available.\n");
        return 0;
    }
    Locker *records = malloc(count * sizeof(Locker));
    if (!records) {
        printf("Memory allocation failed.\n");
        return 0;
    }
    if (!readAllRecords(LOCKER_FILE, records, sizeof(Locker), &count)) {
        free(records);
        printf("Unable to read locker records.\n");
        return 0;
    }
    printf("%-12s %-12s %-10s %-10s\n", "Locker No", "Account", "Allocated", "RentDue");
    for (size_t i = 0; i < count; i++) {
        printf("%-12s %-12s %-10s %-10.2f\n", records[i].locker_no,
               records[i].account_no[0] ? records[i].account_no : "None",
               records[i].is_allocated ? "Yes" : "No",
               records[i].rent_due);
    }
    free(records);
    return 1;
}
