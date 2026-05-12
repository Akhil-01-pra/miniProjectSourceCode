#include "bank.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void generateRandomNumber(char *buffer, int length) {
    const char digits[] = "0123456789";
    for (int i = 0; i < length; i++) {
        buffer[i] = digits[rand() % 10];
    }
    buffer[length] = '\0';
}

void generateAccountNumber(char account_no[MAX_ACCOUNT_NO]) {
    srand((unsigned int)time(NULL));
    sprintf(account_no, "AC%010u", rand() % 1000000000);
}

int accountExistsByUsername(const char *username) {
    size_t count = 0;
    Customer *customers = NULL;
    if (!readAllRecords(CUSTOMER_FILE, NULL, sizeof(Customer), &count)) return 0;
    if (count == 0) return 0;
    customers = malloc(count * sizeof(Customer));
    if (!customers) return 0;
    if (!readAllRecords(CUSTOMER_FILE, customers, sizeof(Customer), &count)) {
        free(customers);
        return 0;
    }
    int found = 0;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(customers[i].username, username) == 0 && customers[i].is_active) {
            found = 1;
            break;
        }
    }
    free(customers);
    return found;
}

int accountExistsByAadhaar(const char *aadhaar) {
    size_t count = 0;
    Customer *customers = NULL;
    if (!readAllRecords(CUSTOMER_FILE, NULL, sizeof(Customer), &count)) return 0;
    if (count == 0) return 0;
    customers = malloc(count * sizeof(Customer));
    if (!customers) return 0;
    if (!readAllRecords(CUSTOMER_FILE, customers, sizeof(Customer), &count)) {
        free(customers);
        return 0;
    }
    int found = 0;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(customers[i].aadhaar, aadhaar) == 0 && customers[i].is_active) {
            found = 1;
            break;
        }
    }
    free(customers);
    return found;
}

int accountExistsByPhone(const char *phone) {
    size_t count = 0;
    Customer *customers = NULL;
    if (!readAllRecords(CUSTOMER_FILE, NULL, sizeof(Customer), &count)) return 0;
    if (count == 0) return 0;
    customers = malloc(count * sizeof(Customer));
    if (!customers) return 0;
    if (!readAllRecords(CUSTOMER_FILE, customers, sizeof(Customer), &count)) {
        free(customers);
        return 0;
    }
    int found = 0;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(customers[i].phone, phone) == 0 && customers[i].is_active) {
            found = 1;
            break;
        }
    }
    free(customers);
    return found;
}

int saveCustomer(const Customer *customer) {
    return saveRecord(CUSTOMER_FILE, customer, sizeof(Customer));
}

int updateCustomer(const Customer *customer, size_t index) {
    return updateRecordByIndex(CUSTOMER_FILE, index, customer, sizeof(Customer));
}

int loadCustomerByUsername(const char *username, Customer *customer, size_t *index) {
    size_t count = 0;
    if (!readAllRecords(CUSTOMER_FILE, NULL, sizeof(Customer), &count)) return 0;
    if (count == 0) return 0;
    Customer *customers = malloc(count * sizeof(Customer));
    if (!customers) return 0;
    if (!readAllRecords(CUSTOMER_FILE, customers, sizeof(Customer), &count)) {
        free(customers);
        return 0;
    }
    int found = 0;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(customers[i].username, username) == 0 && customers[i].is_active) {
            *customer = customers[i];
            *index = i;
            found = 1;
            break;
        }
    }
    free(customers);
    return found;
}

int loadCustomerByAccountNo(const char *account_no, Customer *customer, size_t *index) {
    size_t count = 0;
    if (!readAllRecords(CUSTOMER_FILE, NULL, sizeof(Customer), &count)) return 0;
    if (count == 0) return 0;
    Customer *customers = malloc(count * sizeof(Customer));
    if (!customers) return 0;
    if (!readAllRecords(CUSTOMER_FILE, customers, sizeof(Customer), &count)) {
        free(customers);
        return 0;
    }
    int found = 0;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(customers[i].account_no, account_no) == 0 && customers[i].is_active) {
            *customer = customers[i];
            *index = i;
            found = 1;
            break;
        }
    }
    free(customers);
    return found;
}

void showCustomerProfile(const Customer *customer) {
    printHeader("Customer Profile");
    printf("Account Number : %s\n", customer->account_no);
    printf("Full Name      : %s\n", customer->full_name);
    printf("DOB            : %s\n", customer->dob);
    printf("Address        : %s\n", customer->address);
    printf("Phone Number   : %s\n", customer->phone);
    printf("Aadhaar Number : %s\n", customer->aadhaar);
    printf("Account Type   : %s\n", customer->account_type);
    printf("Balance        : %.2f\n", customer->balance);
    printf("Account Status : %s\n", customer->is_frozen ? "Frozen" : "Active");
    printf("Locker ID      : %s\n", customer->locker_id[0] ? customer->locker_id : "None");
    printf("Created On     : %s", ctime(&customer->created_at));
}

int registerCustomer(void) {
    Customer customer = {0};
    char password[MAX_PASS];
    char confirmPassword[MAX_PASS];
    char depositString[32];
    printHeader("Register New Bank Account");
    printf("Enter username:\n");
    scanf("%31s", customer.username);
    getchar();
    if (accountExistsByUsername(customer.username)) {
        printColored("Username already exists. Please try another.\n", 12);
        return 0;
    }
    maskInput(password, MAX_PASS, "Enter password: ");
    maskInput(confirmPassword, MAX_PASS, "Confirm password: ");
    if (strcmp(password, confirmPassword) != 0) {
        printColored("Passwords do not match.\n", 12);
        return 0;
    }
    if (!validatePasswordStrength(password)) {
        printColored("Password must be at least 8 characters, include upper, lower, digit, and special symbol.\n", 12);
        return 0;
    }
    hashString(password, customer.password_hash);
    printf("Enter full name: ");
    fgets(customer.full_name, MAX_NAME, stdin);
    customer.full_name[strcspn(customer.full_name, "\n")] = '\0';
    printf("Enter date of birth (DD/MM/YYYY): ");
    scanf("%11s", customer.dob);
    getchar();
    if (!validDate(customer.dob)) {
        printColored("Invalid DOB format. Use DD/MM/YYYY.\n", 12);
        return 0;
    }
    printf("Enter address: ");
    fgets(customer.address, MAX_ADDR, stdin);
    customer.address[strcspn(customer.address, "\n")] = '\0';
    printf("Enter phone number: ");
    scanf("%15s", customer.phone);
    getchar();
    if (!validPhoneNumber(customer.phone) || accountExistsByPhone(customer.phone)) {
        printColored("Invalid or duplicate phone number.\n", 12);
        return 0;
    }
    printf("Enter Aadhaar/ID Number: ");
    scanf("%15s", customer.aadhaar);
    getchar();
    if (!validAadhaar(customer.aadhaar) || accountExistsByAadhaar(customer.aadhaar)) {
        printColored("Invalid or duplicate Aadhaar number.\n", 12);
        return 0;
    }
    printf("Choose account type (Savings/Current): ");
    scanf("%9s", customer.account_type);
    getchar();
    if (strcmp(customer.account_type, "Savings") != 0 && strcmp(customer.account_type, "Current") != 0) {
        printColored("Only Savings or Current allowed.\n", 12);
        return 0;
    }
    printf("Enter initial deposit amount: ");
    scanf("%31s", depositString);
    getchar();
    if (!validAmount(depositString)) {
        printColored("Invalid amount.\n", 12);
        return 0;
    }
    customer.balance = atof(depositString);
    if ((strcmp(customer.account_type, "Savings") == 0 && customer.balance < MIN_SAVINGS_DEPOSIT) ||
        (strcmp(customer.account_type, "Current") == 0 && customer.balance < MIN_CURRENT_DEPOSIT)) {
        printColored("Minimum deposit not met for selected account type.\n", 12);
        return 0;
    }
    printf("Enter security question: ");
    fgets(customer.security_question, MAX_QUESTION, stdin);
    customer.security_question[strcspn(customer.security_question, "\n")] = '\0';
    char security_answer[MAX_PASS];
    maskInput(security_answer, MAX_PASS, "Enter answer to security question: ");
    hashString(security_answer, customer.security_answer_hash);
    char atmPin[8];
    maskInput(atmPin, 8, "Set 4-digit ATM PIN: ");
    if (strlen(atmPin) != 4) {
        printColored("ATM PIN must be exactly 4 digits.\n", 12);
        return 0;
    }
    hashString(atmPin, customer.atm_pin_hash);
    generateAccountNumber(customer.account_no);
    customer.is_active = 1;
    customer.is_frozen = 0;
    customer.login_attempts = 0;
    customer.atm_attempts = 0;
    customer.created_at = time(NULL);
    customer.locker_id[0] = '\0';
    if (!saveCustomer(&customer)) {
        printColored("Unable to save account. Try again.\n", 12);
        return 0;
    }
    ATMRecord atmRecord = {0};
    strcpy(atmRecord.account_no, customer.account_no);
    strcpy(atmRecord.pin_hash, customer.atm_pin_hash);
    atmRecord.failed_attempts = 0;
    atmRecord.is_locked = 0;
    atmRecord.last_change = customer.created_at;
    saveRecord(ATM_FILE, &atmRecord, sizeof(ATMRecord));
    printColored("Account created successfully.\n", 10);
    printf("Your account number is %s\n", customer.account_no);
    return 1;
}
