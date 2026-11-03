#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <limits>
using namespace std;

// Smart Digital Expense Tracker with Budget Prediction
// Educational console-based C++ project.
// Data is stored in users.txt, expenses.txt and budgets.txt.

struct User {
    string username, password;
};

struct Expense {
    int id;
    string username, date, category, description;
    double amount;
};

struct Budget {
    string username, month;
    double amount;
};

const string USER_FILE = "users.txt";
const string EXPENSE_FILE = "expenses.txt";
const string BUDGET_FILE = "budgets.txt";

string currentDate() {
    time_t now = time(nullptr);
    tm *local = localtime(&now);
    stringstream ss;
    ss << put_time(local, "%Y-%m-%d");
    return ss.str();
}

string currentMonth() {
    string d = currentDate();
    return d.substr(0, 7);
}

string cleanField(string value) {
    replace(value.begin(), value.end(), '|', '/');
    replace(value.begin(), value.end(), '\n', ' ');
    return value;
}

vector<User> loadUsers() {
    vector<User> data;
    ifstream in(USER_FILE);
    User u;
    while (getline(in, u.username, '|') && getline(in, u.password))
        data.push_back(u);
    return data;
}

vector<Expense> loadExpenses() {
    vector<Expense> data;
    ifstream in(EXPENSE_FILE);
    string line;
    while (getline(in, line)) {
        stringstream ss(line);
        Expense e;
        string amount;
        if (getline(ss, amount, '|') &&
            getline(ss, e.username, '|') &&
            getline(ss, e.date, '|') &&
            getline(ss, e.category, '|') &&
            getline(ss, e.description)) {
            try {
                e.id = stoi(amount.substr(0, amount.find(',')));
                // The first field stores ID; amount is stored at the end.
                stringstream row(line);
                string idText, user, date, category, description, amountText;
                getline(row, idText, '|');
                getline(row, user, '|');
                getline(row, date, '|');
                getline(row, category, '|');
                getline(row, description, '|');
                getline(row, amountText);
                e.id = stoi(idText);
                e.username = user;
                e.date = date;
                e.category = category;
                e.description = description;
                e.amount = stod(amountText);
                data.push_back(e);
            } catch (...) {
                // Ignore damaged or incomplete records.
            }
        }
    }
    return data;
}

vector<Budget> loadBudgets() {
    vector<Budget> data;
    ifstream in(BUDGET_FILE);
    Budget b;
    string amount;
    while (getline(in, b.username, '|') &&
           getline(in, b.month, '|') &&
           getline(in, amount)) {
        try {
            b.amount = stod(amount);
            data.push_back(b);
        } catch (...) {}
    }
    return data;
}

void saveUsers(const vector<User>& data) {
    ofstream out(USER_FILE);
    for (const auto& u : data)
        out << cleanField(u.username) << '|' << cleanField(u.password) << '\n';
}

void saveExpenses(const vector<Expense>& data) {
    ofstream out(EXPENSE_FILE);
    for (const auto& e : data)
        out << e.id << '|' << cleanField(e.username) << '|'
            << e.date << '|' << cleanField(e.category) << '|'
            << cleanField(e.description) << '|' << fixed
            << setprecision(2) << e.amount << '\n';
}

void saveBudgets(const vector<Budget>& data) {
    ofstream out(BUDGET_FILE);
    for (const auto& b : data)
        out << cleanField(b.username) << '|' << b.month << '|'
            << fixed << setprecision(2) << b.amount << '\n';
}

int nextExpenseId(const vector<Expense>& expenses) {
    int id = 1;
    for (const auto& e : expenses) id = max(id, e.id + 1);
    return id;
}

double monthlyTotal(const vector<Expense>& expenses,
                    const string& username, const string& month) {
    double total = 0;
    for (const auto& e : expenses)
        if (e.username == username && e.date.substr(0, 7) == month)
            total += e.amount;
    return total;
}

double getBudget(const vector<Budget>& budgets,
                 const string& username, const string& month) {
    for (const auto& b : budgets)
        if (b.username == username && b.month == month)
            return b.amount;
    return 0;
}

void budgetWarning(const vector<Expense>& expenses,
                   const vector<Budget>& budgets, const string& username) {
    string month = currentMonth();
    double limit = getBudget(budgets, username, month);
    double spent = monthlyTotal(expenses, username, month);
    if (limit <= 0) {
        cout << "No budget set for " << month << ". Please set a budget first.\n";
    } else {
        cout << "Budget: Rs. " << limit << " | Spent: Rs. " << spent << '\n';
        if (spent >= limit)
            cout << "WARNING: Your monthly budget has been reached/exceeded!\n";
        else if (spent >= limit * 0.80)
            cout << "Alert: You have used 80% or more of your budget.\n";
        else
            cout << "Good! You are within your budget.\n";
    }
}

void registerUser() {
    vector<User> users = loadUsers();
    User u;
    cout << "\nCreate username: ";
    cin >> u.username;
    for (const auto& existing : users) {
        if (existing.username == u.username) {
            cout << "Username already exists. Try another one.\n";
            return;
        }
    }
    cout << "Create password: ";
    cin >> u.password;
    users.push_back(u);
    saveUsers(users);
    cout << "Registration successful!\n";
}

bool login(string& activeUser) {
    vector<User> users = loadUsers();
    string username, password;
    cout << "\nUsername: ";
    cin >> username;
    cout << "Password: ";
    cin >> password;
    for (const auto& u : users) {
        if (u.username == username && u.password == password) {
            activeUser = username;
            cout << "Login successful. Welcome, " << activeUser << "!\n";
            return true;
        }
    }
    cout << "Invalid username or password.\n";
    return false;
}

void addExpense(const string& username) {
    vector<Expense> expenses = loadExpenses();
    Expense e;
    e.id = nextExpenseId(expenses);
    e.username = username;
    cout << "\nEnter date (YYYY-MM-DD) or press Enter for today: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, e.date);
    if (e.date.empty()) e.date = currentDate();

    cout << "Category (Food/Travel/Study/Shopping/Bills/Other): ";
    getline(cin, e.category);
    cout << "Description: ";
    getline(cin, e.description);
    cout << "Amount (Rs.): ";
    while (!(cin >> e.amount) || e.amount <= 0) {
        cout << "Enter a valid positive amount: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    expenses.push_back(e);
    saveExpenses(expenses);
    cout << "Expense saved successfully! Expense ID: " << e.id << '\n';

    vector<Budget> budgets = loadBudgets();
    budgetWarning(expenses, budgets, username);
}

void viewHistory(const string& username) {
    vector<Expense> expenses = loadExpenses();
    bool found = false;
    cout << "\n--- EXPENSE HISTORY ---\n";
    cout << left << setw(6) << "ID" << setw(13) << "Date"
         << setw(14) << "Category" << setw(12) << "Amount" << "Description\n";
    cout << string(65, '-') << '\n';
    for (const auto& e : expenses) {
        if (e.username == username) {
            found = true;
            cout << left << setw(6) << e.id << setw(13) << e.date
                 << setw(14) << e.category << setw(12) << fixed
                 << setprecision(2) << e.amount << e.description << '\n';
        }
    }
    if (!found) cout << "No expenses found.\n";
}

void categoryReport(const string& username) {
    vector<Expense> expenses = loadExpenses();
    string month = currentMonth();
    vector<pair<string, double>> totals;
    for (const auto& e : expenses) {
        if (e.username == username && e.date.substr(0, 7) == month) {
            auto it = find_if(totals.begin(), totals.end(),
                [&](const pair<string, double>& p) { return p.first == e.category; });
            if (it == totals.end()) totals.push_back({e.category, e.amount});
            else it->second += e.amount;
        }
    }
    cout << "\n--- CATEGORY-WISE EXPENSE (" << month << ") ---\n";
    if (totals.empty()) { cout << "No expenses this month.\n"; return; }
    sort(totals.begin(), totals.end(),
         [](const auto& a, const auto& b) { return a.second > b.second; });
    for (const auto& p : totals)
        cout << left << setw(18) << p.first << "Rs. "
             << fixed << setprecision(2) << p.second << '\n';
}

void monthlyReport(const string& username) {
    vector<Expense> expenses = loadExpenses();
    vector<Budget> budgets = loadBudgets();
    string month = currentMonth();
    double spent = monthlyTotal(expenses, username, month);
    double limit = getBudget(budgets, username, month);
    cout << "\n--- MONTHLY REPORT: " << month << " ---\n";
    cout << "Total expense: Rs. " << fixed << setprecision(2) << spent << '\n';
    if (limit > 0) {
        cout << "Monthly budget: Rs. " << limit << '\n';
        cout << "Remaining: Rs. " << limit - spent << '\n';
    } else cout << "Monthly budget: Not set\n";
    budgetWarning(expenses, budgets, username);
}

void setMonthlyBudget(const string& username) {
    vector<Budget> budgets = loadBudgets();
    string month;
    double amount;
    cout << "Enter month (YYYY-MM) or press Enter for current month ("
         << currentMonth() << "): ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, month);
    if (month.empty()) month = currentMonth();
    cout << "Enter budget amount (Rs.): ";
    while (!(cin >> amount) || amount <= 0) {
        cout << "Enter a valid positive amount: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    bool updated = false;
    for (auto& b : budgets) {
        if (b.username == username && b.month == month) {
            b.amount = amount;
            updated = true;
            break;
        }
    }
    if (!updated) budgets.push_back({username, month, amount});
    saveBudgets(budgets);
    cout << "Budget saved for " << month << ".\n";
}

void predictExpense(const string& username) {
    vector<Expense> expenses = loadExpenses();
    vector<Budget> budgets = loadBudgets();
    string month = currentMonth();
    double spent = monthlyTotal(expenses, username, month);
    double budget = getBudget(budgets, username, month);

    // Estimate month-end spending using daily average so far.
    int day = 1;
    time_t now = time(nullptr);
    tm *local = localtime(&now);
    day = local->tm_mday;
    int daysInMonth = 31;
    int year = local->tm_year + 1900;
    int m = local->tm_mon + 1;
    if (m == 4 || m == 6 || m == 9 || m == 11) daysInMonth = 30;
    else if (m == 2) {
        bool leap = (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
        daysInMonth = leap ? 29 : 28;
    }
    double prediction = (spent / day) * daysInMonth;
    cout << "\n--- EXPENSE PREDICTION (" << month << ") ---\n";
    cout << "Spent so far: Rs. " << fixed << setprecision(2) << spent << '\n';
    cout << "Predicted month-end expense: Rs. " << prediction << '\n';
    if (budget > 0) {
        cout << "Monthly budget: Rs. " << budget << '\n';
        if (prediction > budget)
            cout << "Prediction: Your budget may be exceeded by Rs. "
                 << prediction - budget << ".\n";
        else cout << "Prediction: You are likely to stay within budget.\n";
    } else cout << "Set a monthly budget to compare the prediction.\n";
}

void searchExpense(const string& username) {
    vector<Expense> expenses = loadExpenses();
    string key;
    cout << "Enter date, category or description keyword to search: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, key);
    bool found = false;
    for (const auto& e : expenses) {
        if (e.username == username &&
            (e.date.find(key) != string::npos ||
             e.category.find(key) != string::npos ||
             e.description.find(key) != string::npos)) {
            cout << "ID: " << e.id << " | " << e.date << " | "
                 << e.category << " | Rs. " << e.amount
                 << " | " << e.description << '\n';
            found = true;
        }
    }
    if (!found) cout << "No matching expense found.\n";
}

void deleteExpense(const string& username) {
    vector<Expense> expenses = loadExpenses();
    int id;
    cout << "Enter Expense ID to delete: ";
    cin >> id;
    auto oldSize = expenses.size();
    expenses.erase(remove_if(expenses.begin(), expenses.end(),
        [&](const Expense& e) { return e.username == username && e.id == id; }),
        expenses.end());
    if (expenses.size() == oldSize) cout << "Expense ID not found.\n";
    else {
        saveExpenses(expenses);
        cout << "Expense deleted successfully.\n";
    }
}

void userMenu(const string& username) {
    int choice;
    do {
        cout << "\n========== SMART DIGITAL EXPENSE TRACKER ==========\n";
        cout << "Logged in as: " << username << "\n";
        cout << "1. Add Daily Expense\n";
        cout << "2. View Expense History\n";
        cout << "3. Category-wise Expense\n";
        cout << "4. Monthly Expense Report\n";
        cout << "5. Set Monthly Budget\n";
        cout << "6. Budget Warning\n";
        cout << "7. Expense Prediction\n";
        cout << "8. Search Expense\n";
        cout << "9. Delete Expense\n";
        cout << "10. Logout\n";
        cout << "Enter your choice: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            choice = 0;
        }
        switch (choice) {
            case 1: addExpense(username); break;
            case 2: viewHistory(username); break;
            case 3: categoryReport(username); break;
            case 4: monthlyReport(username); break;
            case 5: setMonthlyBudget(username); break;
            case 6: {
                auto expenses = loadExpenses();
                auto budgets = loadBudgets();
                budgetWarning(expenses, budgets, username);
                break;
            }
            case 7: predictExpense(username); break;
            case 8: searchExpense(username); break;
            case 9: deleteExpense(username); break;
            case 10: cout << "Logged out successfully.\n"; break;
            default: cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 10);
}

int main() {
    int choice;
    string activeUser;
    do {
        cout << "\n===============================================\n";
        cout << "       SMART DIGITAL EXPENSE TRACKER\n";
        cout << "===============================================\n";
        cout << "1. User Registration\n";
        cout << "2. Login System\n";
        cout << "3. Exit\n";
        cout << "Enter your choice: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            choice = 0;
        }
        switch (choice) {
            case 1: registerUser(); break;
            case 2:
                if (login(activeUser)) userMenu(activeUser);
                break;
            case 3: cout << "Thank you for using the Expense Tracker!\n"; break;
            default: cout << "Invalid choice. Please enter 1, 2 or 3.\n";
        }
    } while (choice != 3);
    return 0;
}
