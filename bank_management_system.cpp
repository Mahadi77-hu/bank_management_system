#include <iostream>
#include <iomanip>
#include <cstring>
#include <limits>

using namespace std;

const int STACK_CAPACITY = 100;
const int QUEUE_CAPACITY = 50;

struct AccountNode {
    int accountNumber;
    char holderName[51];
    char accountType[10];
    double balance;
    AccountNode* next;
};

struct Transaction {
    char type[20];
    int accountNumber;
    int senderAccount;
    int receiverAccount;
    double amount;
    char description[101];
};

struct TransactionStack {
    Transaction items[STACK_CAPACITY];
    int top;
};

struct Customer {
    char name[51];
    int accountNumber;
    int queueNumber;
};

struct CustomerQueue {
    Customer items[QUEUE_CAPACITY];
    int front;
    int rear;
    int size;
};

void clearInput()
{
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int readInteger(const char* message)
{
    int value;
    cout << message;
    while (!(cin >> value)) {
        clearInput();
        cout << "Invalid input. Please enter a whole number: ";
    }
    clearInput();
    return value;
}

long double readMoney(const char* message)
{
    long double value;
    cout << message;
    while (!(cin >> value)) {
        clearInput();
        cout << "Invalid input. Please enter a valid amount: ";
    }
    clearInput();
    return value;
}

void readText(const char* message, char* destination, int destinationSize)
{
    cout << message;
    cin.getline(destination, destinationSize);

    while (strlen(destination) == 0) {
        cout << "This field cannot be empty. Please try again: ";
        cin.getline(destination, destinationSize);
    }
}

void initializeStack(TransactionStack& transactionStack)
{
    transactionStack.top = -1;
}

bool pushTransaction(TransactionStack& transactionStack, const Transaction& transaction)
{
    if (transactionStack.top == STACK_CAPACITY - 1) {
        cout << "Transaction history is full. This transaction was not recorded.\n";
        return false;
    }

    transactionStack.top++;
    transactionStack.items[transactionStack.top] = transaction;
    return true;
}

void initializeQueue(CustomerQueue& queue)
{
    queue.front = -1;
    queue.rear = -1;
    queue.size = 0;
}

bool isQueueFull(const CustomerQueue& queue)
{
    return queue.size == QUEUE_CAPACITY;
}

bool isQueueEmpty(const CustomerQueue& queue)
{
    return queue.size == 0;
}

bool enqueueCustomer(CustomerQueue& queue, const Customer& customer)
{
    if (isQueueFull(queue)) {
        cout << "Waiting queue is full. The customer could not be added.\n";
        return false;
    }

    if (isQueueEmpty(queue)) {
        queue.front = 0;
    }

    queue.rear = (queue.rear + 1) % QUEUE_CAPACITY;
    queue.items[queue.rear] = customer;
    queue.size++;
    return true;
}

bool dequeueCustomer(CustomerQueue& queue, Customer& removedCustomer)
{
    if (isQueueEmpty(queue)) {
        cout << "Waiting queue is empty. No customer to serve.\n";
        return false;
    }

    removedCustomer = queue.items[queue.front];

    if (queue.size == 1) {
        queue.front = -1;
        queue.rear = -1;
    } else {
        queue.front = (queue.front + 1) % QUEUE_CAPACITY;
    }

    queue.size--;
    return true;
}

void displayQueue(const CustomerQueue& queue)
{
    if (isQueueEmpty(queue)) {
        cout << "Waiting queue is empty.\n";
        return;
    }

    cout << "\nCustomers waiting (next to be served first):\n";
    int index = queue.front;
    for (int i = 0; i < queue.size; i++) {
        cout << i + 1 << ". Queue #" << queue.items[index].queueNumber
             << " | " << queue.items[index].name;
        if (queue.items[index].accountNumber != 0) {
            cout << " | Account #" << queue.items[index].accountNumber;
        } else {
            cout << " | No account";
        }
        cout << '\n';
        index = (index + 1) % QUEUE_CAPACITY;
    }
}

AccountNode* findAccount(AccountNode* head, int accountNumber)
{
    AccountNode* current = head;
    while (current != NULL) {
        if (current->accountNumber == accountNumber) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void createAccount(AccountNode*& head, int& nextAccountNumber)
{
    char name[51];
    char accountType[10];

    readText("Enter account holder name: ", name, 51);

    cout << "Select account type:\n";
    cout << "1. Savings\n";
    cout << "2. Current\n";
    int typeChoice = readInteger("Enter choice: ");

    if (typeChoice == 1) {
        strcpy(accountType, "Savings");
    } else if (typeChoice == 2) {
        strcpy(accountType, "Current");
    } else {
        cout << "Invalid account type. Account was not created.\n";
        return;
    }

    long double initialBalance = readMoney("Enter initial balance: ");
    if (initialBalance < 0) {
        cout << "Initial balance cannot be negative. Account was not created.\n";
        return;
    }

    AccountNode* newAccount = new AccountNode;
    newAccount->accountNumber = nextAccountNumber;
    nextAccountNumber++;
    strcpy(newAccount->holderName, name);
    strcpy(newAccount->accountType, accountType);
    newAccount->balance = static_cast<double>(initialBalance);
    newAccount->next = NULL;

    if (head == NULL) {
        head = newAccount;
    } else {
        AccountNode* current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newAccount;
    }

    cout << "Account created successfully. Account number: "
         << newAccount->accountNumber << '\n';
}

void displayAllAccounts(AccountNode* head)
{
    if (head == NULL) {
        cout << "No accounts exist.\n";
        return;
    }

    cout << fixed << setprecision(2);
    cout << "\nAll bank accounts:\n";
    cout << left << setw(16) << "Account No."
         << setw(22) << "Holder Name"
         << setw(12) << "Type"
         << setw(15) << "Balance" << '\n';

    AccountNode* current = head;
    while (current != NULL) {
        cout << left << setw(16) << current->accountNumber
             << setw(22) << current->holderName
             << setw(12) << current->accountType
             << setw(15) << current->balance << '\n';
        current = current->next;
    }
}

void searchAccount(AccountNode* head)
{
    int accountNumber = readInteger("Enter account number to search: ");
    AccountNode* account = findAccount(head, accountNumber);

    if (account == NULL) {
        cout << "Account not found.\n";
        return;
    }

    cout << fixed << setprecision(2);
    cout << "Account found:\n";
    cout << "Account number: " << account->accountNumber << '\n';
    cout << "Holder name: " << account->holderName << '\n';
    cout << "Account type: " << account->accountType << '\n';
    cout << "Balance: " << account->balance << '\n';
}

void depositMoney(AccountNode* head, TransactionStack& transactionStack)
{
    int accountNumber = readInteger("Enter account number: ");
    AccountNode* account = findAccount(head, accountNumber);

    if (account == NULL) {
        cout << "Account not found. Deposit cancelled.\n";
        return;
    }

    long double amount = readMoney("Enter deposit amount: ");
    if (amount <= 0) {
        cout << "Deposit amount must be greater than zero.\n";
        return;
    }

    account->balance += static_cast<double>(amount);

    Transaction transaction;
    strcpy(transaction.type, "DEPOSIT");
    transaction.accountNumber = accountNumber;
    transaction.senderAccount = 0;
    transaction.receiverAccount = accountNumber;
    transaction.amount = static_cast<double>(amount);
    strcpy(transaction.description, "Money deposited into account");
    pushTransaction(transactionStack, transaction);

    cout << fixed << setprecision(2);
    cout << "Deposit successful. New balance: " << account->balance << '\n';
}

void withdrawMoney(AccountNode* head, TransactionStack& transactionStack)
{
    int accountNumber = readInteger("Enter account number: ");
    AccountNode* account = findAccount(head, accountNumber);

    if (account == NULL) {
        cout << "Account not found. Withdrawal cancelled.\n";
        return;
    }

    long double amount = readMoney("Enter withdrawal amount: ");
    if (amount <= 0) {
        cout << "Withdrawal amount must be greater than zero.\n";
        return;
    }

    if (static_cast<double>(amount) > account->balance) {
        cout << "Insufficient balance. Withdrawal cancelled.\n";
        return;
    }

    account->balance -= static_cast<double>(amount);

    Transaction transaction;
    strcpy(transaction.type, "WITHDRAWAL");
    transaction.accountNumber = accountNumber;
    transaction.senderAccount = accountNumber;
    transaction.receiverAccount = 0;
    transaction.amount = static_cast<double>(amount);
    strcpy(transaction.description, "Money withdrawn from account");
    pushTransaction(transactionStack, transaction);

    cout << fixed << setprecision(2);
    cout << "Withdrawal successful. New balance: " << account->balance << '\n';
}

void transferMoney(AccountNode* head, TransactionStack& transactionStack)
{
    int senderNumber = readInteger("Enter sender account number: ");
    int receiverNumber = readInteger("Enter receiver account number: ");

    if (senderNumber == receiverNumber) {
        cout << "Sender and receiver cannot be the same account.\n";
        return;
    }

    AccountNode* sender = findAccount(head, senderNumber);
    AccountNode* receiver = findAccount(head, receiverNumber);

    if (sender == NULL) {
        cout << "Sender account not found. Transfer cancelled.\n";
        return;
    }
    if (receiver == NULL) {
        cout << "Receiver account not found. Transfer cancelled.\n";
        return;
    }

    long double amount = readMoney("Enter transfer amount: ");
    if (amount <= 0) {
        cout << "Transfer amount must be greater than zero.\n";
        return;
    }

    if (static_cast<double>(amount) > sender->balance) {
        cout << "Insufficient balance. Transfer cancelled.\n";
        return;
    }

    sender->balance -= static_cast<double>(amount);
    receiver->balance += static_cast<double>(amount);

    Transaction transaction;
    strcpy(transaction.type, "TRANSFER");
    transaction.accountNumber = senderNumber;
    transaction.senderAccount = senderNumber;
    transaction.receiverAccount = receiverNumber;
    transaction.amount = static_cast<double>(amount);
    strcpy(transaction.description, "Money transferred between accounts");
    pushTransaction(transactionStack, transaction);

    cout << fixed << setprecision(2);
    cout << "Transfer successful. Sender balance: " << sender->balance
         << " | Receiver balance: " << receiver->balance << '\n';
}

void deleteAccount(AccountNode*& head)
{
    int accountNumber = readInteger("Enter account number to delete: ");

    if (head == NULL) {
        cout << "Account not found.\n";
        return;
    }

    AccountNode* current = head;
    AccountNode* previous = NULL;

    while (current != NULL && current->accountNumber != accountNumber) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        cout << "Account not found. Deletion cancelled.\n";
        return;
    }

    if (previous == NULL) {
        head = current->next;
    } else {
        previous->next = current->next;
    }

    delete current;
    cout << "Account deleted successfully.\n";
}

void viewTransactionHistory(const TransactionStack& transactionStack)
{
    if (transactionStack.top == -1) {
        cout << "No transactions have been recorded.\n";
        return;
    }

    cout << fixed << setprecision(2);
    cout << "\nRecent transaction history (newest first):\n";

    for (int i = transactionStack.top; i >= 0; i--) {
        const Transaction& transaction = transactionStack.items[i];
        cout << "Type: " << transaction.type;

        if (strcmp(transaction.type, "TRANSFER") == 0) {
            cout << " | Sender: " << transaction.senderAccount
                 << " | Receiver: " << transaction.receiverAccount;
        } else {
            cout << " | Account: " << transaction.accountNumber;
        }

        cout << " | Amount: " << transaction.amount
             << " | " << transaction.description << '\n';
    }
}

void addCustomerToQueue(CustomerQueue& queue, int& nextQueueNumber)
{
    Customer customer;

    readText("Enter customer name: ", customer.name, 51);
    customer.accountNumber = readInteger("Enter account number (enter 0 if none): ");
    customer.queueNumber = nextQueueNumber;

    if (enqueueCustomer(queue, customer)) {
        nextQueueNumber++;
        cout << "Customer added successfully. Queue number: "
             << customer.queueNumber << '\n';
    }
}

void serveNextCustomer(CustomerQueue& queue)
{
    Customer customer;
    if (dequeueCustomer(queue, customer)) {
        cout << "Serving customer: " << customer.name
             << " | Queue #" << customer.queueNumber;
        if (customer.accountNumber != 0) {
            cout << " | Account #" << customer.accountNumber;
        }
        cout << '\n';
    }
}

void displayMenu()
{
    cout << "\n========================================\n";
    cout << "       BANK MANAGEMENT SYSTEM\n";
    cout << "========================================\n";
    cout << "1. Create a new account\n";
    cout << "2. Display all accounts\n";
    cout << "3. Search for an account\n";
    cout << "4. Deposit money\n";
    cout << "5. Withdraw money\n";
    cout << "6. Transfer money between accounts\n";
    cout << "7. Delete an account\n";
    cout << "8. View recent transaction history\n";
    cout << "9. Add a customer to the waiting queue\n";
    cout << "10. Serve the next customer\n";
    cout << "11. Display the waiting queue\n";
    cout << "12. Exit\n";
}

int main()
{
    AccountNode* head = NULL;
    TransactionStack transactionStack;
    CustomerQueue customerQueue;

    initializeStack(transactionStack);
    initializeQueue(customerQueue);

    int nextAccountNumber = 1001;
    int nextQueueNumber = 1;
    int choice;

    do {
        displayMenu();
        choice = readInteger("Enter your choice (1-12): ");

        switch (choice) {
            case 1:
                createAccount(head, nextAccountNumber);
                break;
            case 2:
                displayAllAccounts(head);
                break;
            case 3:
                searchAccount(head);
                break;
            case 4:
                depositMoney(head, transactionStack);
                break;
            case 5:
                withdrawMoney(head, transactionStack);
                break;
            case 6:
                transferMoney(head, transactionStack);
                break;
            case 7:
                deleteAccount(head);
                break;
            case 8:
                viewTransactionHistory(transactionStack);
                break;
            case 9:
                addCustomerToQueue(customerQueue, nextQueueNumber);
                break;
            case 10:
                serveNextCustomer(customerQueue);
                break;
            case 11:
                displayQueue(customerQueue);
                break;
            case 12:
                cout << "Thank you for using the Bank Management System.\n";
                break;
            default:
                cout << "Invalid menu choice. Please enter a number from 1 to 12.\n";
        }
    } while (choice != 12);

    while (head != NULL) {
        AccountNode* temporary = head;
        head = head->next;
        delete temporary;
    }

    return 0;
}
