# Bank Management System in C

## Overview
This project is a professional terminal-based Bank Management System built in C. It supports customer registration, secure login, deposit/withdrawal, money transfer, ATM services, cheque book requests, locker management, and admin operations.

## Folder Structure
- `main.c` - application entry point and menu flow
- `account.c` - account registration and customer record management
- `security.c` - password hashing, input masking, encrypted file handling, validation helpers
- `transactions.c` - deposit, withdrawal, transfer, transaction logging, mini-statement
- `atm.c` - ATM PIN management and ATM withdrawal simulation
- `locker.c` - safe locker request, access, and admin locker report
- `admin.c` - admin login and bank management operations
- `bank.h` - shared structures, constants, and function prototypes
- `customers.dat` - encrypted customer data storage (created at runtime)
- `transactions.dat` - encrypted transaction log storage
- `atm.dat` - encrypted ATM record storage
- `lockers.dat` - encrypted locker record storage
- `cheque_requests.dat` - encrypted cheque request storage
- `admin_audit.log` - audit trail for admin and transaction actions

## Features
- Customer registration and account creation
- Unique automatic account number generation
- Secure username/password login with masking
- Password hashing and encrypted file storage
- Admin login with hidden credentials
- Deposit, withdraw, transfer, balance enquiry
- Mini statement and full transaction history
- ATM PIN management, recovery, and ATM cash simulation
- Cheque book request submission and admin approval
- Locker application, allocation, and access authentication
- Admin tools for account search, freeze/unfreeze, delete, and reports
- Professional terminal UI with clear menus

## Setup Instructions for VS Code
1. Open this folder in VS Code.
2. Ensure GCC is installed and available in your PATH.
3. Open the integrated terminal.
4. Compile the project using:

```powershell
cd "c:\Users\KiTE\Saved Games\miniProjectSourceCode"
gcc main.c account.c security.c transactions.c atm.c locker.c admin.c -o bank.exe
```

5. Run the program:

```powershell
./bank.exe
```

## Admin Login
- Username: `admin`
- Password: `Admin@123`

## Notes
- Data files are created automatically on first run.
- Keep the generated `.dat` files in the executable directory for persistence.
- The system uses modular C code with structures, functions, file handling, and validation.

## Advanced Concepts Implemented
- File encryption with XOR cipher
- Password and PIN hashing
- Session timeout logic
- OTP simulation for PIN recovery
- Audit log system
- Secure input masking
- Daily withdrawal limit enforcement

## Recommended Usage
1. Register a customer account.
2. Login as a customer to perform banking operations.
3. Use ATM services and apply for cheque books or locker services.
4. Login as admin to manage accounts, transactions, and requests.

## Compilation Tips
- Use GCC or MinGW on Windows.
- If compile warnings appear, ensure `bank.h` is included in each module.
- Run the executable from the same folder where the data files are stored.

