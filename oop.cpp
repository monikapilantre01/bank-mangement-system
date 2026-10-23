
#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
using namespace std;

// ================= BASE CLASS =================
class Account {
protected:
    int accNo;
    string name;
    double balance;
    string pin;

public:
    Account() : accNo(0), name(""), balance(0), pin("") {}

    Account(int a, string n, double b, string p)
        : accNo(a), name(n), balance(b), pin(p) {}

    virtual ~Account() = default;

    // Verify the entered PIN
    bool verifyPin(string enteredPin) {
        return pin == enteredPin;
    }

    virtual void display() {
        cout << "\nAccount No: " << accNo;
        cout << "\nName: " << name;
        cout << "\nBalance: " << balance << endl;
    }

    virtual bool deposit(double amt) {
        if (amt <= 0) {
            cout << "Amount must be positive!\n";
            return false;
        }

        balance += amt;
        cout << "Deposited Successfully\n";
        return true;
    }

    virtual bool withdraw(double amt) {
        if (amt <= 0) {
            cout << "Amount must be positive!\n";
            return false;
        }

        if (amt <= balance) {
            balance -= amt;
            cout << "Withdrawn Successfully\n";
            return true;
        }

        cout << "Insufficient Balance\n";
        return false;
    }

    virtual void interest() = 0;

    // Save a newly created account
    virtual void saveToFile() {
        ofstream fout("bankdata.txt", ios::app);

        if (!fout) {
            cout << "Unable to open file!\n";
            return;
        }

        fout << accNo << " " << name << " "
             << balance << " " << pin << '\n';

        fout.close();
        cout << "Account saved to file.\n";
    }

    // Update an existing account record
    void updateFile() {
        ifstream fin("bankdata.txt");
        if (!fin) {
            cout << "Unable to open bankdata.txt!\n";
            return;
        }

        ofstream temp("temp.txt");
        if (!temp) {
            fin.close();
            cout << "Unable to create temporary file!\n";
            return;
        }

        int a;
        string n, p;
        double b;
        bool found = false;

        while (fin >> a >> n >> b >> p) {
            if (a == accNo) {
                // Replace the matching record.
                // Skip any duplicate records for this account.
                if (!found) {
                    temp << accNo << " " << name << " "
                         << balance << " " << pin << '\n';
                    found = true;
                }
            } else {
                // Preserve other accounts.
                temp << a << " " << n << " "
                     << b << " " << p << '\n';
            }
        }

        fin.close();
        temp.close();

        if (!found) {
            remove("temp.txt");
            cout << "Account record not found!\n";
            return;
        }

        if (remove("bankdata.txt") != 0) {
            cout << "Unable to replace original file!\n";
            return;
        }

        if (rename("temp.txt", "bankdata.txt") != 0) {
            cout << "Unable to rename temporary file!\n";
            return;
        }

        cout << "Account record updated successfully!\n";
    }

    // Read all saved accounts without displaying PINs
    virtual void loadFromFile() {
        ifstream fin("bankdata.txt");

        if (!fin) {
            cout << "No accounts found yet.\n";
            return;
        }

        cout << "\n--- Accounts in File ---\n";

        int a;
        string n, p;
        double b;

        while (fin >> a >> n >> b >> p) {
            cout << "AccNo: " << a
                 << " | Name: " << n
                 << " | Balance: " << b << '\n';
        }

        fin.close();
    }
};

// ================= SAVINGS CLASS =================
class Savings : public Account {
    double rate;

public:
    Savings(int a, string n, double b, double r, string p)
        : Account(a, n, b, p), rate(r) {}

    void interest() override {
        double i = balance * rate / 100;
        cout << "Interest: " << i << endl;
    }
};

// ================= CURRENT CLASS =================
class Current : public Account {
public:
    Current(int a, string n, double b, string p)
        : Account(a, n, b, p) {}

    void interest() override {
        cout << "No interest in current account\n";
    }
};

// ================= BANK SYSTEM =================
class BankSystem {
    Account* acc = nullptr;

    // Authenticate the active account holder
    bool authenticate() {
        string enteredPin;

        cout << "Enter your PIN: ";
        cin >> enteredPin;

        if (acc->verifyPin(enteredPin)) {
            cout << "PIN verified successfully.\n";
            return true;
        }

        cout << "Incorrect PIN. Access denied.\n";
        return false;
    }

public:
    ~BankSystem() {
        delete acc;
    }

    // 1. Create account
    void createAccount() {
        int type, a;
        string n, p;
        double b;

        cout << "\n1. Savings\n2. Current\n";
        cout << "Choose account type: ";
        cin >> type;

        if (type != 1 && type != 2) {
            cout << "Invalid account type!\n";
            return;
        }

        cout << "Enter Account No: ";
        cin >> a;

        cout << "Enter Name (one word): ";
        cin >> n;

        cout << "Enter Initial Balance: ";
        cin >> b;

        if (b < 0) {
            cout << "Initial balance cannot be negative!\n";
            return;
        }

        cout << "Create a PIN: ";
        cin >> p;

        if (p.length() != 4) {
            cout << "PIN must contain exactly 4 characters.\n";
            return;
        }

        for (char ch : p) {
            if (ch < '0' || ch > '9') {
                cout << "PIN must contain digits only.\n";
                return;
            }
        }

        double r = 0;

        if (type == 1) {
            cout << "Enter Interest Rate: ";
            cin >> r;

            if (r < 0) {
                cout << "Interest rate cannot be negative!\n";
                return;
            }
        }

        Account* newAcc = nullptr;

        if (type == 1) {
            newAcc = new Savings(a, n, b, r, p);
        } else {
            newAcc = new Current(a, n, b, p);
        }

        // Keep the new account active for this session.
        delete acc;
        acc = newAcc;

        acc->saveToFile();
    }

    // 2. Deposit
    void depositMoney() {
        if (!acc) {
            cout << "Create an account first.\n";
            return;
        }

        if (!authenticate()) return;

        double amt;
        cout << "Enter amount to deposit: ";
        cin >> amt;

        if (acc->deposit(amt)) {
            acc->updateFile();
        }
    }

    // 3. Withdraw
    void withdrawMoney() {
        if (!acc) {
            cout << "Create an account first.\n";
            return;
        }

        if (!authenticate()) return;

        double amt;
        cout << "Enter amount to withdraw: ";
        cin >> amt;

        if (acc->withdraw(amt)) {
            acc->updateFile();
        }
    }

    // 4. Display balance and account details
    void showDetails() {
        if (!acc) {
            cout << "Create an account first.\n";
            return;
        }

        if (!authenticate()) return;

        acc->display();
    }

    // 5. Calculate interest
    void showInterest() {
        if (!acc) {
            cout << "Create an account first.\n";
            return;
        }

        if (!authenticate()) return;

        acc->interest();
    }

    // 6. Display saved records
    void loadAccounts() {
        if (acc) {
            // This is an all-records listing for demonstration.
            acc->loadFromFile();
        } else {
            Current temp(0, "", 0, "");
            temp.loadFromFile();
        }
    }

    // 7. Menu
    void menu() {
        int choice;

        while (true) {
            cout << "\n===== BANK MENU =====";
            cout << "\n1. Create Account";
            cout << "\n2. Deposit";
            cout << "\n3. Withdraw";
            cout << "\n4. Display Account Details";
            cout << "\n5. Interest";
            cout << "\n6. Show All Saved Accounts";
            cout << "\n0. Exit";
            cout << "\nEnter choice: ";

            if (!(cin >> choice)) {
                cout << "Invalid input. Exiting.\n";
                return;
            }

            switch (choice) {
                case 1: createAccount(); break;
                case 2: depositMoney(); break;
                case 3: withdrawMoney(); break;
                case 4: showDetails(); break;
                case 5: showInterest(); break;
                case 6: loadAccounts(); break;
                case 0:
                    cout << "Exiting...\n";
                    return;
                default:
                    cout << "Invalid choice!\n";
            }
        }
    }
};

// ================= MAIN =================
int main() {
    BankSystem b;
    b.menu();
    return 0;
}
