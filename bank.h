#ifndef BANK_H
#define BANK_H

#include <time.h>

#define CUSTOMER_FILE "customers.dat"
#define TRANSACTION_FILE "transactions.dat"
#define ATM_FILE "atm.dat"
#define LOCKER_FILE "lockers.dat"
#define CHEQUE_FILE "cheque_requests.dat"
#define AUDIT_FILE "admin_audit.log"

#define MAX_NAME 100
#define MAX_ADDR 150
#define MAX_USERNAME 32
#define MAX_PASS 32
#define MAX_QUESTION 128
#define MAX_ACCOUNT_TYPE 10
#define MAX_STATUS 16
#define MAX_ACCOUNT_NO 13
#define MAX_LOCKER_NO 13
#define MAX_PHONE 16
#define MAX_AADHAAR 16
#define SESSION_TIMEOUT_SECONDS 300
#define DAILY_WITHDRAWAL_LIMIT 20000.0
#define MIN_SAVINGS_DEPOSIT 500.0
#define MIN_CURRENT_DEPOSIT 1000.0
#define INTEREST_RATE 4.0

typedef struct {
    char account_no[MAX_ACCOUNT_NO];
    char username[MAX_USERNAME];
    char password_hash[65];
    char full_name[MAX_NAME];
    char dob[12];
    char address[MAX_ADDR];
    char phone[MAX_PHONE];
    char aadhaar[MAX_AADHAAR];
    char account_type[MAX_ACCOUNT_TYPE];
    double balance;
    int is_active;
    int is_frozen;
    int login_attempts;
    time_t created_at;
    char security_question[MAX_QUESTION];
    char security_answer_hash[65];
    char atm_pin_hash[65];
    int atm_attempts;
    char locker_id[MAX_LOCKER_NO];
} Customer;

typedef struct {
    char account_no[MAX_ACCOUNT_NO];
    char type[20];
    double amount;
    char target_account[MAX_ACCOUNT_NO];
    double balance_after;
    time_t timestamp;
} Transaction;

typedef struct {
    char account_no[MAX_ACCOUNT_NO];
    char pin_hash[65];
    int failed_attempts;
    int is_locked;
    time_t last_change;
} ATMRecord;

typedef struct {
    char locker_no[MAX_LOCKER_NO];
    char account_no[MAX_ACCOUNT_NO];
    int is_allocated;
    char access_code_hash[65];
    double rent_due;
    time_t allocated_at;
    int rent_months;
} Locker;

typedef struct {
    char request_id[MAX_LOCKER_NO];
    char account_no[MAX_ACCOUNT_NO];
    char status[MAX_STATUS];
    time_t request_time;
} ChequeRequest;

typedef struct {
    time_t last_action;
} Session;

// utility functions
void clearConsole(void);
void waitEnter(void);
void printHeader(const char *title);
void printColored(const char *message, int color);
int validatePasswordStrength(const char *password);
int validPhoneNumber(const char *phone);
int validAadhaar(const char *aadhaar);
int validDate(const char *dob);
int validAmount(const char *value);
void maskInput(char *input, int size, const char *prompt);
void hashString(const char *input, char *output);
int secureCompare(const char *hash1, const char *hash2);
int saveRecord(const char *filename, const void *record, size_t recSize);
int readAllRecords(const char *filename, void *buffer, size_t recSize, size_t *count);
int updateRecordByIndex(const char *filename, size_t index, const void *record, size_t recSize);

// account module
void generateAccountNumber(char account_no[MAX_ACCOUNT_NO]);
int accountExistsByUsername(const char *username);
int accountExistsByAadhaar(const char *aadhaar);
int accountExistsByPhone(const char *phone);
int saveCustomer(const Customer *customer);
int updateCustomer(const Customer *customer, size_t index);
int loadCustomerByUsername(const char *username, Customer *customer, size_t *index);
int loadCustomerByAccountNo(const char *account_no, Customer *customer, size_t *index);
int registerCustomer(void);
void showCustomerProfile(const Customer *customer);

// transaction module
void recordTransaction(const char *account_no, const char *type, double amount, const char *target_account, double balance_after);
void showMiniStatement(const char *account_no);
void showFullStatement(const char *account_no);
int depositToAccount(Customer *customer, double amount);
int withdrawFromAccount(Customer *customer, double amount);
int transferMoney(Customer *customer, const char *target_account, double amount);

// ATM module
int initializeATMForCustomer(const Customer *customer);
int loadATMRecord(const char *account_no, ATMRecord *record, size_t *index);
int updateATMRecord(const ATMRecord *record, size_t index);
int changeATMPin(Customer *customer);
int recoverATMPin(Customer *customer);
int withdrawFromATM(Customer *customer);

// locker module
int requestLocker(Customer *customer);
int accessLocker(void);
int manageLockerAvailability(void);

// cheque module
int requestChequeBook(const Customer *customer);
void showChequeStatus(const char *account_no);

// admin module
int adminLogin(void);
void adminMenu(void);

#endif // BANK_H
