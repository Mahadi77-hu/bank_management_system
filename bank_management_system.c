#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_CAPACITY 100
#define QUEUE_CAPACITY 50

struct AccountNode {
    int accountNumber;
    char holderName[51];
    char accountType[10];
    double balance;
    struct AccountNode *next;
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
    struct Transaction items[STACK_CAPACITY];
    int top;
};

struct Customer {
    char name[51];
    int accountNumber;
    int queueNumber;
};

struct CustomerQueue {
    struct Customer items[QUEUE_CAPACITY];
    int front;
    int rear;
    int size;
};

void clearInput(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

int readInteger(const char *message)
{
    int value;

    printf("%s", message);
    while (scanf("%d", &value) != 1) {
        clearInput();
        printf("Invalid input. Please enter a whole number: ");
    }
    clearInput();
    return value;
}

double readMoney(const char *message)
{
    double value;

    printf("%s", message);
    while (scanf("%lf", &value) != 1) {
        clearInput();
        printf("Invalid input. Please enter a valid amount: ");
    }
    clearInput();
    return value;
}

void readText(const char *message, char *destination, int destinationSize)
{
    printf("%s", message);

    if (fgets(destination, destinationSize, stdin) == NULL) {
        destination[0] = '\0';
    }
    destination[strcspn(destination, "\n")] = '\0';

    while (strlen(destination) == 0) {
        printf("This field cannot be empty. Please try again: ");
        if (fgets(destination, destinationSize, stdin) == NULL) {
            destination[0] = '\0';
        }
        destination[strcspn(destination, "\n")] = '\0';
    }
}

void initializeStack(struct TransactionStack *stack)
{
    stack->top = -1;
}

int pushTransaction(struct TransactionStack *stack, struct Transaction transaction)
{
    if (stack->top == STACK_CAPACITY - 1) {
        printf("Transaction history is full. This transaction was not recorded.\n");
        return 0;
    }

    stack->top++;
    stack->items[stack->top] = transaction;
    return 1;
}

void initializeQueue(struct CustomerQueue *queue)
{
    queue->front = -1;
    queue->rear = -1;
    queue->size = 0;
}

int isQueueFull(struct CustomerQueue *queue)
{
    return queue->size == QUEUE_CAPACITY;
}

int isQueueEmpty(struct CustomerQueue *queue)
{
    return queue->size == 0;
}

int enqueueCustomer(struct CustomerQueue *queue, struct Customer customer)
{
    if (isQueueFull(queue)) {
        printf("Waiting queue is full. The customer could not be added.\n");
        return 0;
    }

    if (isQueueEmpty(queue)) {
        queue->front = 0;
    }

    queue->rear = (queue->rear + 1) % QUEUE_CAPACITY;
    queue->items[queue->rear] = customer;
    queue->size++;
    return 1;
}

int dequeueCustomer(struct CustomerQueue *queue, struct Customer *removedCustomer)
{
    if (isQueueEmpty(queue)) {
        printf("Waiting queue is empty. No customer to serve.\n");
        return 0;
    }

    *removedCustomer = queue->items[queue->front];

    if (queue->size == 1) {
        queue->front = -1;
        queue->rear = -1;
    } else {
        queue->front = (queue->front + 1) % QUEUE_CAPACITY;
    }

    queue->size--;
    return 1;
}

void displayQueue(struct CustomerQueue *queue)
{
    int i;
    int index;

    if (isQueueEmpty(queue)) {
        printf("Waiting queue is empty.\n");
        return;
    }

    printf("\nCustomers waiting (next to be served first):\n");
    index = queue->front;

    for (i = 0; i < queue->size; i++) {
        printf("%d. Queue #%d | %s", i + 1, queue->items[index].queueNumber,
               queue->items[index].name);

        if (queue->items[index].accountNumber != 0) {
            printf(" | Account #%d", queue->items[index].accountNumber);
        } else {
            printf(" | No account");
        }
        printf("\n");

        index = (index + 1) % QUEUE_CAPACITY;
    }
}

struct AccountNode *findAccount(struct AccountNode *head, int accountNumber)
{
    struct AccountNode *current = head;

    while (current != NULL) {
        if (current->accountNumber == accountNumber) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void createAccount(struct AccountNode **head, int *nextAccountNumber)
{
    char name[51];
    char accountType[10];
    double initialBalance;
    int typeChoice;
    struct AccountNode *newAccount;
    struct AccountNode *current;

    readText("Enter account holder name: ", name, 51);

    printf("Select account type:\n");
    printf("1. Savings\n");
    printf("2. Current\n");
    typeChoice = readInteger("Enter choice: ");

    if (typeChoice == 1) {
        strcpy(accountType, "Savings");
    } else if (typeChoice == 2) {
        strcpy(accountType, "Current");
    } else {
        printf("Invalid account type. Account was not created.\n");
        return;
    }

    initialBalance = readMoney("Enter initial balance: ");
    if (initialBalance < 0) {
        printf("Initial balance cannot be negative. Account was not created.\n");
        return;
    }

    newAccount = (struct AccountNode *)malloc(sizeof(struct AccountNode));
    if (newAccount == NULL) {
        printf("Memory could not be allocated. Account was not created.\n");
        return;
    }

    newAccount->accountNumber = *nextAccountNumber;
    (*nextAccountNumber)++;
    strcpy(newAccount->holderName, name);
    strcpy(newAccount->accountType, accountType);
    newAccount->balance = initialBalance;
    newAccount->next = NULL;

    if (*head == NULL) {
        *head = newAccount;
    } else {
        current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newAccount;
    }

    printf("Account created successfully. Account number: %d\n",
           newAccount->accountNumber);
}

void displayAllAccounts(struct AccountNode *head)
{
    struct AccountNode *current;

    if (head == NULL) {
        printf("No accounts exist.\n");
        return;
    }

    printf("\nAll bank accounts:\n");
    printf("%-16s %-22s %-12s %-15s\n", "Account No.", "Holder Name", "Type", "Balance");

    current = head;
    while (current != NULL) {
        printf("%-16d %-22s %-12s %-15.2lf\n", current->accountNumber,
               current->holderName, current->accountType, current->balance);
        current = current->next;
    }
}

void searchAccount(struct AccountNode *head)
{
    int accountNumber = readInteger("Enter account number to search: ");
    struct AccountNode *account = findAccount(head, accountNumber);

    if (account == NULL) {
        printf("Account not found.\n");
        return;
    }

    printf("Account found:\n");
    printf("Account number: %d\n", account->accountNumber);
    printf("Holder name: %s\n", account->holderName);
    printf("Account type: %s\n", account->accountType);
    printf("Balance: %.2lf\n", account->balance);
}

void depositMoney(struct AccountNode *head, struct TransactionStack *stack)
{
    int accountNumber;
    double amount;
    struct AccountNode *account;
    struct Transaction transaction;

    accountNumber = readInteger("Enter account number: ");
    account = findAccount(head, accountNumber);

    if (account == NULL) {
        printf("Account not found. Deposit cancelled.\n");
        return;
    }

    amount = readMoney("Enter deposit amount: ");
    if (amount <= 0) {
        printf("Deposit amount must be greater than zero.\n");
        return;
    }

    account->balance += amount;

    strcpy(transaction.type, "DEPOSIT");
    transaction.accountNumber = accountNumber;
    transaction.senderAccount = 0;
    transaction.receiverAccount = accountNumber;
    transaction.amount = amount;
    strcpy(transaction.description, "Money deposited into account");
    pushTransaction(stack, transaction);

    printf("Deposit successful. New balance: %.2lf\n", account->balance);
}

void withdrawMoney(struct AccountNode *head, struct TransactionStack *stack)
{
    int accountNumber;
    double amount;
    struct AccountNode *account;
    struct Transaction transaction;

    accountNumber = readInteger("Enter account number: ");
    account = findAccount(head, accountNumber);

    if (account == NULL) {
        printf("Account not found. Withdrawal cancelled.\n");
        return;
    }

    amount = readMoney("Enter withdrawal amount: ");
    if (amount <= 0) {
        printf("Withdrawal amount must be greater than zero.\n");
        return;
    }

    if (amount > account->balance) {
        printf("Insufficient balance. Withdrawal cancelled.\n");
        return;
    }

    account->balance -= amount;

    strcpy(transaction.type, "WITHDRAWAL");
    transaction.accountNumber = accountNumber;
    transaction.senderAccount = accountNumber;
    transaction.receiverAccount = 0;
    transaction.amount = amount;
    strcpy(transaction.description, "Money withdrawn from account");
    pushTransaction(stack, transaction);

    printf("Withdrawal successful. New balance: %.2lf\n", account->balance);
}

void transferMoney(struct AccountNode *head, struct TransactionStack *stack)
{
    int senderNumber;
    int receiverNumber;
    double amount;
    struct AccountNode *sender;
    struct AccountNode *receiver;
    struct Transaction transaction;

    senderNumber = readInteger("Enter sender account number: ");
    receiverNumber = readInteger("Enter receiver account number: ");

    if (senderNumber == receiverNumber) {
        printf("Sender and receiver cannot be the same account.\n");
        return;
    }

    sender = findAccount(head, senderNumber);
    receiver = findAccount(head, receiverNumber);

    if (sender == NULL) {
        printf("Sender account not found. Transfer cancelled.\n");
        return;
    }
    if (receiver == NULL) {
        printf("Receiver account not found. Transfer cancelled.\n");
        return;
    }

    amount = readMoney("Enter transfer amount: ");
    if (amount <= 0) {
        printf("Transfer amount must be greater than zero.\n");
        return;
    }

    if (amount > sender->balance) {
        printf("Insufficient balance. Transfer cancelled.\n");
        return;
    }

    sender->balance -= amount;
    receiver->balance += amount;

    strcpy(transaction.type, "TRANSFER");
    transaction.accountNumber = senderNumber;
    transaction.senderAccount = senderNumber;
    transaction.receiverAccount = receiverNumber;
    transaction.amount = amount;
    strcpy(transaction.description, "Money transferred between accounts");
    pushTransaction(stack, transaction);

    printf("Transfer successful. Sender balance: %.2lf | Receiver balance: %.2lf\n",
           sender->balance, receiver->balance);
}

void deleteAccount(struct AccountNode **head)
{
    int accountNumber = readInteger("Enter account number to delete: ");
    struct AccountNode *current;
    struct AccountNode *previous = NULL;

    if (*head == NULL) {
        printf("Account not found.\n");
        return;
    }

    current = *head;
    while (current != NULL && current->accountNumber != accountNumber) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("Account not found. Deletion cancelled.\n");
        return;
    }

    if (previous == NULL) {
        *head = current->next;
    } else {
        previous->next = current->next;
    }

    free(current);
    printf("Account deleted successfully.\n");
}

void viewTransactionHistory(struct TransactionStack *stack)
{
    int i;

    if (stack->top == -1) {
        printf("No transactions have been recorded.\n");
        return;
    }

    printf("\nRecent transaction history (newest first):\n");

    for (i = stack->top; i >= 0; i--) {
        printf("Type: %s", stack->items[i].type);

        if (strcmp(stack->items[i].type, "TRANSFER") == 0) {
            printf(" | Sender: %d | Receiver: %d",
                   stack->items[i].senderAccount,
                   stack->items[i].receiverAccount);
        } else {
            printf(" | Account: %d", stack->items[i].accountNumber);
        }

        printf(" | Amount: %.2lf | %s\n",
               stack->items[i].amount,
               stack->items[i].description);
    }
}

void addCustomerToQueue(struct CustomerQueue *queue, int *nextQueueNumber)
{
    struct Customer customer;

    readText("Enter customer name: ", customer.name, 51);
    customer.accountNumber = readInteger("Enter account number (enter 0 if none): ");
    customer.queueNumber = *nextQueueNumber;

    if (enqueueCustomer(queue, customer)) {
        (*nextQueueNumber)++;
        printf("Customer added successfully. Queue number: %d\n",
               customer.queueNumber);
    }
}

void serveNextCustomer(struct CustomerQueue *queue)
{
    struct Customer customer;

    if (dequeueCustomer(queue, &customer)) {
        printf("Serving customer: %s | Queue #%d",
               customer.name, customer.queueNumber);

        if (customer.accountNumber != 0) {
            printf(" | Account #%d", customer.accountNumber);
        }
        printf("\n");
    }
}

void displayMenu(void)
{
    printf("\n========================================\n");
    printf("       BANK MANAGEMENT SYSTEM\n");
    printf("========================================\n");
    printf("1. Create a new account\n");
    printf("2. Display all accounts\n");
    printf("3. Search for an account\n");
    printf("4. Deposit money\n");
    printf("5. Withdraw money\n");
    printf("6. Transfer money between accounts\n");
    printf("7. Delete an account\n");
    printf("8. View recent transaction history\n");
    printf("9. Add a customer to the waiting queue\n");
    printf("10. Serve the next customer\n");
    printf("11. Display the waiting queue\n");
    printf("12. Exit\n");
}

int main(void)
{
    struct AccountNode *head = NULL;
    struct TransactionStack transactionStack;
    struct CustomerQueue customerQueue;
    int nextAccountNumber = 1001;
    int nextQueueNumber = 1;
    int choice;

    initializeStack(&transactionStack);
    initializeQueue(&customerQueue);

    do {
        displayMenu();
        choice = readInteger("Enter your choice (1-12): ");

        switch (choice) {
            case 1:
                createAccount(&head, &nextAccountNumber);
                break;
            case 2:
                displayAllAccounts(head);
                break;
            case 3:
                searchAccount(head);
                break;
            case 4:
                depositMoney(head, &transactionStack);
                break;
            case 5:
                withdrawMoney(head, &transactionStack);
                break;
            case 6:
                transferMoney(head, &transactionStack);
                break;
            case 7:
                deleteAccount(&head);
                break;
            case 8:
                viewTransactionHistory(&transactionStack);
                break;
            case 9:
                addCustomerToQueue(&customerQueue, &nextQueueNumber);
                break;
            case 10:
                serveNextCustomer(&customerQueue);
                break;
            case 11:
                displayQueue(&customerQueue);
                break;
            case 12:
                printf("Thank you for using the Bank Management System.\n");
                break;
            default:
                printf("Invalid menu choice. Please enter a number from 1 to 12.\n");
        }
    } while (choice != 12);

    while (head != NULL) {
        struct AccountNode *temporary = head;
        head = head->next;
        free(temporary);
    }

    return 0;
}
