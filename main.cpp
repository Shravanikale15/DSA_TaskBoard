#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <queue>
#include <map>
#include <clocale>
#include <windows.h>
using namespace std;

enum DataType { DT_INT, DT_STRING, DT_BOOL, DT_FLOAT, DT_DATE, DT_LINK};

struct Cell {
    DataType type;
    string stringValue;
    int intValue;
    bool boolValue;
    float floatValue;
    string dateValue;
    string linkValue;

    void setValue(string val) { type = DT_STRING; stringValue = val; }
    void setValue(int val) { type = DT_INT; intValue = val; }
    void setValue(bool val) { type = DT_BOOL; boolValue = val; }
    void setValue(float val) { type = DT_FLOAT; floatValue = val; }
    void setDate(string val) { type = DT_DATE; dateValue = val; }
    void setLink(string url) { type = DT_LINK; stringValue = url; }

    string getAsString() const {
        switch (type) {
            case DT_INT: return to_string(intValue);
            case DT_STRING: return stringValue;
            case DT_BOOL: return boolValue ? "Yes" : "No";
            case DT_FLOAT: return to_string(floatValue);
            case DT_DATE: return dateValue;
            //case LINK: return stringValue;
        }
        return "";
    }
};

struct Task {
    int id;
    string name;
    string priority;
    string deadline;
    string status;
    vector<Cell> extraColumns;
};

int getPriorityValue(const string& p) {
    if (p == "High") return 3;
    if (p == "Medium") return 2;
    if (p == "Low") return 1;
    return 0; // Unknown
}

struct CompareTasks {
    bool operator()(const Task& a, const Task& b) {
        int pa = getPriorityValue(a.priority);
        int pb = getPriorityValue(b.priority);
        if (pa == pb)
            return a.deadline > b.deadline; // earlier deadline = higher priority
        return pa < pb; // higher priority = top of the queue
    }
};


struct ToDoList {
    string name;
    vector<string> columnNames;
    vector<DataType> columnTypes;
    vector<Task> tasks;
    int nextId = 1;
};

time_t parseDeadline(const string& deadline) {
    istringstream ss(deadline);
    int year, month, day, hour, minute;
    char sep1, sep2, space, sep3;

    ss >> year >> sep1 >> month >> sep2 >> day >> space >> hour >> sep3 >> minute;

    if (ss.fail()) {
        cerr << "Failed to parse deadline: " << deadline << endl;
        return time(0) + 999999;  // Push task far in future to avoid alert
    }

    tm timeStruct = {};
    timeStruct.tm_year = year - 1900;
    timeStruct.tm_mon = month - 1;
    timeStruct.tm_mday = day;
    timeStruct.tm_hour = hour;
    timeStruct.tm_min = minute;
    timeStruct.tm_sec = 0;

    return mktime(&timeStruct);
}


struct TimeBasedCompare {
    bool operator()(const Task& a, const Task& b) {
        return parseDeadline(a.deadline) > parseDeadline(b.deadline);  // Min-heap: earliest deadline first
    }
};

void showCategorizedAlerts(const ToDoList &list) {
    time_t now = time(0);
    bool found = false;

    cout << "\n===== Task Alerts (by Due Date) =====\n";

    for (const auto& task : list.tasks) {
        time_t taskTime = parseDeadline(task.deadline);
        double daysLeft = difftime(taskTime, now) / (60.0 * 60.0 * 24.0);

        if (daysLeft < 0) continue; // Skip past tasks

        string category;

        if (daysLeft <= 0.5) {
            category = "🔴 Due Today";
        } else if (daysLeft <= 2) {
            category = "🟠 Due in 1-2 Days";
        } else if (daysLeft <= 6) {
            category = "🟡 Due in 3-6 Days";
        } else {
            category = "🟢 Due in 1+ Week";
        }

        cout << category << ":    Task #" << task.id << " - \"" << task.name << "\""
             << " | Deadline: " << task.deadline << endl;

        found = true;
    }

    if (!found) {
        cout << "No upcoming tasks within a week.\n";
    }
}


void displayMenu() {
    cout << "\n====== TASK MANAGER MENU ======\n";
    cout << "1. Add Column\n";
    cout << "2. Add Task (Row)\n";
    cout << "3. Print To-Do List\n";
    cout << "4. Update Cell\n";
    cout << "5. Delete Task (Row)\n";
    cout << "6. Delete Column\n";
    cout << "7. Save to CSV\n";
    cout << "8. Load from CSV\n";
    cout << "9. Sort by Column\n";
    cout << "10. Filter by Column Value\n";
    cout << "11. Get Stats (Task Count)\n";
    cout << "12. Undo Last Operation\n";
    cout << "13. Prioritize Tasks\n";
    cout << "14. Send Alerts\n";
    cout << "15. Remove completed tasks\n";
    cout << "16. View full cell\n";
    cout << "0. Exit\n\n";
}

void addColumn(ToDoList &list) {
    string name;
    int type;
    cout << "Enter column name: ";
    getline(cin, name);
    cout << "Data Type (1-INT, 2-STRING, 3-BOOL, 4-FLOAT): ";
    cin >> type;
    cin.ignore();
    DataType dtype = static_cast<DataType>(type - 1);
    list.columnNames.push_back(name);
    list.columnTypes.push_back(dtype);

    for (auto &task : list.tasks) {
        Cell c; c.type = dtype;
        cout << "Enter value for Task ID " << task.id << ": ";
        switch (dtype) {
            case DT_INT: cin >> c.intValue; break;
            case DT_STRING: getline(cin, c.stringValue); break;
            case DT_BOOL: cin >> c.boolValue; break;
            case DT_FLOAT: cin >> c.floatValue; break;
            default: getline(cin, c.stringValue); break;
        }
        cin.ignore();
        task.extraColumns.push_back(c);
    }
}

void addTask(ToDoList &list) {
    Task t;
    t.id = list.nextId++;

    cout << "Enter Task Name: ";
    getline(cin, t.name);
    cout << "Priority (High/Medium/Low): ";
    getline(cin, t.priority);
    cout << "Deadline (yyyy-mm-dd hh:mm): ";
    getline(cin, t.deadline);
    t.status = "Pending";

    for (size_t i = 0; i < list.columnTypes.size(); ++i) {
        DataType dtype = list.columnTypes[i];
        string colName = list.columnNames[i];
        Cell c;
        c.type = dtype;

        cout << "Enter value for '" << colName << "': ";
        switch (dtype) {
            case DT_INT: cin >> c.intValue; cin.ignore(); break;
            case DT_BOOL: cin >> c.boolValue; cin.ignore(); break;
            case DT_FLOAT: cin >> c.floatValue; cin.ignore(); break;
            case DT_STRING: getline(cin, c.stringValue); break;
            case DT_DATE: getline(cin, c.dateValue); break;
            case DT_LINK: getline(cin, c.linkValue); break;
            default: getline(cin, c.stringValue); break;
        }

        t.extraColumns.push_back(c);
    }


    list.tasks.push_back(t);
    cout << "✅ Task added successfully.\n";
}

string fitToWidth(const string &str, int width) {
    if ((int)str.length() <= width)
        return str + string(width - str.length(), ' '); // pad spaces
    else if (width >= 4)
        return str.substr(0, width - 3) + "..."; // truncate + ellipsis
    else
        return str.substr(0, width); // if too small, just cut
}

void printToDoList(const ToDoList &list) {
    cout << "\n=== " << list.name << " ===\n";
    cout << left << setw(5) << "ID" << setw(20) << "Task Name"
         << setw(10) << "Priority" << setw(20) << "Deadline"
         << setw(15) << "Status";

    for (const auto &col : list.columnNames)
        cout << setw(25) << col;
    cout << "\n";

    int totalWidth = 5 + 20 + 10 + 20 + 15 + (25 * list.columnNames.size());
    cout << string(totalWidth, '-') << "\n";

    for (const auto &task : list.tasks) {
        cout << left
            << setw(5) << task.id
            << setw(20) << fitToWidth(task.name, 20)
            << setw(10) << fitToWidth(task.priority, 10)
            << setw(20) << fitToWidth(task.deadline, 20)
            << setw(15) << fitToWidth(task.status, 15);

        for (const auto &cell : task.extraColumns)
            cout << setw(25) << fitToWidth(cell.getAsString(), 20);

        cout << "\n";
    }
}


void updateCell(ToDoList &list) {
    int id;
    string colName;
    cout << "Enter Task ID: ";
    cin >> id;
    cin.ignore();
    cout << "Enter Column Name (e.g. TaskName, Priority, Deadline, Status or your custom column): ";
    getline(cin, colName);

    for (auto &task : list.tasks) {
        if (task.id == id) {
            if (colName == "TaskName") {
                cout << "New Task Name: ";
                getline(cin, task.name);
            } else if (colName == "Priority") {
                cout << "New Priority: ";
                getline(cin, task.priority);
            } else if (colName == "Deadline") {
                cout << "New Deadline: ";
                getline(cin, task.deadline);
            } else if (colName == "Status") {
                cout << "New Status: ";
                getline(cin, task.status);
            } else {
                // Search in extra columns
                bool found = false;
                for (size_t i = 0; i < list.columnNames.size(); ++i) {
                    if (list.columnNames[i] == colName) {
                        Cell &cell = task.extraColumns[i];
                        cout << "Enter new value for '" << colName << "': ";
                        switch (cell.type) {
                            case DT_INT: cin >> cell.intValue; break;
                            case DT_STRING: getline(cin, cell.stringValue); cin.ignore(); break;
                            case DT_BOOL: cin >> cell.boolValue; break;
                            case DT_FLOAT: cin >> cell.floatValue; break;
                            case DT_DATE: cin.ignore(); getline(cin, cell.dateValue); break;
                            case DT_LINK: cin.ignore(); getline(cin, cell.linkValue); break;
                            default: cin.ignore(); getline(cin, cell.stringValue); break;
                        }
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    cout << "⚠️ Column not found!\n";
                    return;
                }
            }

            cout << "✅ Update complete.\n";
            return;
        }
    }

    cout << "❌ Task ID not found!\n";
}


void deleteTask(ToDoList &list) {
    int id; cout << "Enter Task ID to delete: "; cin >> id; cin.ignore();
    auto it = remove_if(list.tasks.begin(), list.tasks.end(), [id](Task &t) { return t.id == id; });
    if (it != list.tasks.end()) {
        list.tasks.erase(it, list.tasks.end());
        cout << "Task deleted.\n";
    } else {
        cout << "Task not found.\n";
    }
}


void deleteColumn(ToDoList &list) {
    string colName;
    cout << "Enter column name to delete: ";
    getline(cin, colName);

    auto it = find(list.columnNames.begin(), list.columnNames.end(), colName);

    if (it != list.columnNames.end()) {
        int index = distance(list.columnNames.begin(), it);

        // Remove from structure
        list.columnNames.erase(it);
        list.columnTypes.erase(list.columnTypes.begin() + index);

        for (auto &task : list.tasks)
            task.extraColumns.erase(task.extraColumns.begin() + index);

        cout << "✅ Column '" << colName << "' deleted.\n";
    } else {
        cout << "❌ Column '" << colName << "' not found.\n";
    }
}


void saveToCSV(const ToDoList &list) {
    string fname;
    cout << "Enter filename to save: ";
    getline(cin, fname);
    ofstream out(fname + ".csv");
    out << "ID,Name,Priority,Deadline,Status";
    for (auto col : list.columnNames) out << "," << col;
    out << "\n";

    for (auto &t : list.tasks) {
        out << t.id << "," << t.name << "," << t.priority << "," << t.deadline << "," << t.status;
        for (auto &cell : t.extraColumns)
            out << "," << cell.getAsString();
        out << "\n";
    }
    out.close();
    cout << "Saved to " << fname << ".csv\n";
}

void loadFromCSV(ToDoList &list) {
    string fname;
    cout << "Enter filename to load: ";
    getline(cin, fname);
    ifstream in(fname + ".csv");
    if (!in) {
        cout << "File not found.\n";
        return;
    }

    string line;
    getline(in, line); // Read header this time!

    // Extract all column names
    vector<string> headers;
    size_t pos = 0;
    while (pos < line.size()) {
        size_t next = line.find(',', pos);
        if (next == string::npos) next = line.size();
        headers.push_back(line.substr(pos, next - pos));
        pos = next + 1;
    }

    // First 5 are standard fields; the rest are extra columns
    list.columnNames.clear();
    for (size_t i = 5; i < headers.size(); ++i) {
        list.columnNames.push_back(headers[i]);
        list.columnTypes.push_back(DT_STRING); // Default all loaded columns to string
    }
    
    list.tasks.clear();
    while (getline(in, line)) {
        vector<string> tokens;
        size_t pos = 0;
        while (pos < line.size()) {
            size_t next = line.find(',', pos);
            if (next == string::npos) next = line.size();
            tokens.push_back(line.substr(pos, next - pos));
            pos = next + 1;
        }

        if (tokens.size() < 5) continue;

        Task t;
        t.id = stoi(tokens[0]);
        t.name = tokens[1];
        t.priority = tokens[2];
        t.deadline = tokens[3];
        t.status = tokens[4];

        for (size_t i = 5; i < tokens.size(); ++i) {
            Cell c;
            c.setValue(tokens[i]);
            t.extraColumns.push_back(c);
        }

        list.tasks.push_back(t);
    }

    int maxId = 0;
    for (const auto& t : list.tasks) {
        if (t.id > maxId) maxId = t.id;
    }
    list.nextId = maxId + 1;

    cout << "Loaded successfully.\n";
}


void sortByColumn(ToDoList &list) {
    cout << "\nWhich column do you want to sort by?\n";
    cout << "0 - ID\n1 - Task Name\n2 - Priority\n3 - Deadline\n4 - Status\n";

    // List custom columns dynamically
    for (int i = 0; i < list.columnNames.size(); ++i) {
        cout << (i + 5) << " - " << list.columnNames[i] << "\n";
    }

    int choice;
    cout << "Enter column index: ";
    cin >> choice;
    cin.ignore();

    if (choice == 0) {
        sort(list.tasks.begin(), list.tasks.end(), [](const Task &a, const Task &b) {
            return a.id < b.id;
        });
    } else if (choice == 1) {
        sort(list.tasks.begin(), list.tasks.end(), [](const Task &a, const Task &b) {
            return a.name < b.name;
        });
    } else if (choice == 2) {
        sort(list.tasks.begin(), list.tasks.end(), [](const Task &a, const Task &b) {
            return a.priority < b.priority;
        });
    } else if (choice == 3) {
        sort(list.tasks.begin(), list.tasks.end(), [](const Task &a, const Task &b) {
            return a.deadline < b.deadline;
        });
    } else if (choice == 4) {
        sort(list.tasks.begin(), list.tasks.end(), [](const Task &a, const Task &b) {
            return a.status < b.status;
        });
    } else if (choice >= 5 && choice < 5 + list.columnNames.size()) {
        int colIndex = choice - 5;
        DataType dtype = list.columnTypes[colIndex];

        // Sort by type
        switch (dtype) {
            case DT_INT:
                sort(list.tasks.begin(), list.tasks.end(), [colIndex](const Task &a, const Task &b) {
                    return a.extraColumns[colIndex].intValue < b.extraColumns[colIndex].intValue;
                });
                break;
            case DT_STRING:
            case DT_DATE:
            case DT_LINK:
                sort(list.tasks.begin(), list.tasks.end(), [colIndex](const Task &a, const Task &b) {
                    return a.extraColumns[colIndex].stringValue < b.extraColumns[colIndex].stringValue;
                });
                break;
            case DT_BOOL:
                sort(list.tasks.begin(), list.tasks.end(), [colIndex](const Task &a, const Task &b) {
                    return a.extraColumns[colIndex].boolValue < b.extraColumns[colIndex].boolValue;
                });
                break;
            case DT_FLOAT:
                sort(list.tasks.begin(), list.tasks.end(), [colIndex](const Task &a, const Task &b) {
                    return a.extraColumns[colIndex].floatValue < b.extraColumns[colIndex].floatValue;
                });
                break;
        }
    } else {
        cout << "Invalid column index.\n";
        return;
    }

    cout << "Sorted successfully.\n";
}
 
 
void filterTasks(ToDoList &list) {
    stack<vector<Task>> filterHistory;
    vector<Task> filteredTasks = list.tasks;

    while (true) {
        cout << "\nFilter Menu:\n";
        cout << "1. Filter by standard column (Status, Priority, Deadline, etc)\n";
        cout << "2. Filter by extra column\n";
        cout << "3. Undo last filter\n";
        cout << "4. Show filtered tasks\n";
        cout << "5. Exit filtering\n";
        cout << "Enter choice: ";

        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            string colName, value;
            cout << "Enter standard column name (id/name/priority/status/deadline): ";
            getline(cin, colName);
            cout << "Enter value to filter by: ";
            getline(cin, value);

            filterHistory.push(filteredTasks);
            vector<Task> newFiltered;

            for (const auto &task : filteredTasks) {
                if ((colName == "id" && to_string(task.id) == value) ||
                    (colName == "name" && task.name == value) ||
                    (colName == "priority" && task.priority == value) ||
                    (colName == "status" && task.status == value) ||
                    (colName == "deadline" && task.deadline == value)) {
                    newFiltered.push_back(task);
                }
            }

            if (newFiltered.empty()) cout << "No tasks match that filter.\n";
            else filteredTasks = newFiltered;
        }

        else if (choice == 2) {
            string colName, value;
            cout << "Available extra columns:\n";
            for (int i = 0; i < list.columnNames.size(); ++i) {
                cout << i << ": " << list.columnNames[i] << "\n";
            }
            cout << "Enter column index to filter by: ";
            int idx;
            cin >> idx;
            cin.ignore();

            if (idx < 0 || idx >= list.columnNames.size()) {
                cout << "Invalid index.\n";
                continue;
            }

            cout << "Enter value to filter by: ";
            getline(cin, value);

            filterHistory.push(filteredTasks);
            vector<Task> newFiltered;
            for (const auto &task : filteredTasks) {
                if (idx < task.extraColumns.size() &&
                    task.extraColumns[idx].getAsString() == value) {
                    newFiltered.push_back(task);
                }
            }

            if (newFiltered.empty()) cout << "No tasks match that filter.\n";
            else filteredTasks = newFiltered;
        }

        else if (choice == 3) {
            if (filterHistory.empty()) {
                cout << "No filter to undo.\n";
            } else {
                filteredTasks = filterHistory.top();
                filterHistory.pop();
                cout << "Undid last filter.\n";
            }
        }

        else if (choice == 4) {
            if (filteredTasks.empty()) {
                cout << "No tasks to show.\n";
            } else {
                cout << "\nFiltered Task List:\n";
                cout << left << setw(5) << "ID" << setw(20) << "Task Name"
                     << setw(10) << "Priority" << setw(20) << "Deadline"
                     << setw(15) << "Status";

                for (const auto &col : list.columnNames)
                    cout << setw(15) << col;
                cout << "\n";

                for (const auto &task : filteredTasks) {
                    cout << left << setw(5) << task.id
                         << setw(20) << task.name
                         << setw(10) << task.priority
                         << setw(20) << task.deadline
                         << setw(15) << task.status;
                    for (const auto &cell : task.extraColumns)
                        cout << setw(15) << cell.getAsString();
                    cout << "\n";
                }
            }
        }

        else if (choice == 5) {
            cout << "Exiting filter mode.\n";
            break;
        }

        else {
            cout << "Invalid choice.\n";
        }
    }
}


void getStats(const ToDoList &list) {
    cout << "Total tasks: " << list.tasks.size() << "\n";
}

void undoLastOperation(ToDoList &list, stack<ToDoList> &undoStack) {
    if (undoStack.empty()) {
        cout << "No previous state to undo.\n";
        return;
    }
    list = undoStack.top();
    undoStack.pop();
    cout << "Undo successful.\n";
}


void scheduleTasks(const ToDoList& list) {
    priority_queue<Task, vector<Task>, CompareTasks> pq;
    for (const auto& task : list.tasks) {
        pq.push(task);
    }

    cout << "\n=== Task Execution Order (High Priority + Earliest Deadline) ===\n";
    while (!pq.empty()) {
        Task t = pq.top(); pq.pop();
        cout << "Task #" << t.id << ": " << t.name
             << " [Priority: " << t.priority
             << ", Deadline: " << t.deadline << ", Status: " << t.status << "]\n";
    }
}

time_t parseDeadline(const string &deadline); // function declaration

void removeCompletedTasks(ToDoList &list) {
    int before = list.tasks.size();

    // Remove all tasks whose status is "Completed" (case-sensitive match)
    list.tasks.erase(remove_if(list.tasks.begin(), list.tasks.end(), [](const Task &t) {
        return t.status == "Completed";
    }), list.tasks.end());

    int after = list.tasks.size();
    cout << "🗑️ Removed " << (before - after) << " completed task(s).\n";
}

void viewFullCell(const ToDoList &list) {
    int id;
    string column;
    cout << "Enter Task ID: ";
    cin >> id;
    cin.ignore();
    cout << "Enter column name (TaskName, Priority, Deadline, Status or extra column): ";
    getline(cin, column);

    for (const auto &task : list.tasks) {
        if (task.id == id) {
            if (column == "TaskName") cout << "Full Task Name: " << task.name << "\n";
            else if (column == "Priority") cout << "Full Priority: " << task.priority << "\n";
            else if (column == "Deadline") cout << "Full Deadline: " << task.deadline << "\n";
            else if (column == "Status") cout << "Full Status: " << task.status << "\n";
            else {
                auto it = find(list.columnNames.begin(), list.columnNames.end(), column);
                if (it != list.columnNames.end()) {
                    int idx = distance(list.columnNames.begin(), it);
                    if (idx < task.extraColumns.size()) {
                        cout << "Full Value of \"" << column << "\": "
                             << task.extraColumns[idx].getAsString() << "\n";
                    } else {
                        cout << "No value for that column.\n";
                    }
                } else {
                    cout << "Column name not found.\n";
                }
            }
            return;
        }
    }
    cout << "Task ID not found!\n";
}


int main() {
    setlocale(LC_ALL, "en_US.UTF-8");
    ToDoList todo;
    todo.name = "Smart Task List";
    stack<ToDoList> undoStack;
    int choice;
    SetConsoleOutputCP(CP_UTF8);

    while (true) {
        displayMenu();
        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore();
        cout << "\n";
        if (choice == 0) break;
        if (choice != 3 && choice != 11) undoStack.push(todo);

        switch (choice) {
            case 1: undoStack.push(todo);addColumn(todo); break;
            case 2: undoStack.push(todo);addTask(todo); break;
            case 3: printToDoList(todo); break;
            case 4: undoStack.push(todo);updateCell(todo); break;
            case 5: undoStack.push(todo);deleteTask(todo); break;
            case 6: undoStack.push(todo);deleteColumn(todo); break;
            case 7: saveToCSV(todo); break;
            case 8: loadFromCSV(todo); break;
            case 9: undoStack.push(todo);sortByColumn(todo); break;
            case 10: undoStack.push(todo);filterTasks(todo); break;
            case 11: getStats(todo); break;
            case 12: undoLastOperation(todo, undoStack); break;
            case 13: scheduleTasks(todo); break;
            case 14: showCategorizedAlerts(todo); break;
            case 15: removeCompletedTasks(todo); break;
            case 16: viewFullCell(todo); break;
            default: cout << "Invalid choice.\n";
        }
    }

    return 0;
}
