#include <bits/stdc++.h>
using namespace std;

enum class Day { Friday = 0, Saturday = 1, Sunday = 2 };

struct LeaveInterval {
    Day day;
    string timeRange; // format HH:MM-HH:MM
    bool isReport{false};
};

static const vector<pair<Day, string>> defaultDormitoryLeaves = {
    {Day::Saturday, "18:00-21:00"},
    {Day::Sunday, "09:00-21:00"},
};

static const vector<pair<Day, string>> defaultKyivLeaves = {
    {Day::Friday, "19:30-07:30"},
    {Day::Saturday, "18:00-08:30"},
    {Day::Sunday, "09:00-07:30"},
};

struct Soldier {
    string name;
    bool isKyiv;
    vector<LeaveInterval> leaves;

    void addDefaultLeaves() {
        const auto &defaults = isKyiv ? defaultKyivLeaves : defaultDormitoryLeaves;
        for (const auto &p : defaults) {
            leaves.push_back(LeaveInterval{p.first, p.second, false});
        }
    }
};

int minutesFromString(const string &time) {
    int h = stoi(time.substr(0, 2));
    int m = stoi(time.substr(3, 2));
    return h * 60 + m;
}

pair<int, int> intervalToMinutes(Day day, const string &range) {
    int dayIndex = static_cast<int>(day);
    auto dash = range.find('-');
    string start = range.substr(0, dash);
    string end = range.substr(dash + 1);
    int startMinutes = dayIndex * 24 * 60 + minutesFromString(start);
    int endMinutes = minutesFromString(end);
    // If end is earlier than start, it rolls over to the next day
    if (minutesFromString(end) <= minutesFromString(start)) {
        endMinutes += (dayIndex + 1) * 24 * 60;
    } else {
        endMinutes += dayIndex * 24 * 60;
    }
    return {startMinutes, endMinutes};
}

string dayToString(Day day) {
    switch (day) {
        case Day::Friday: return "П'ятниця";
        case Day::Saturday: return "Субота";
        case Day::Sunday: return "Неділя";
    }
    return "";
}

vector<Soldier> loadGroup(const string &filePath) {
    ifstream in(filePath);
    vector<Soldier> group;
    if (!in.is_open()) {
        cerr << "Не вдалося відкрити файл групи: " << filePath << "\n";
        return group;
    }

    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        bool isKyiv = false;
        if (line.find("(к)") != string::npos) {
            isKyiv = true;
            line = line.substr(0, line.find("(к)"));
            // trim spaces
            while (!line.empty() && isspace(line.back())) line.pop_back();
        }
        Soldier s{line, isKyiv, {}};
        s.addDefaultLeaves();
        group.push_back(std::move(s));
    }
    return group;
}

void printSchedule(const vector<Soldier> &group) {
    cout << "С-41 звільнення\n";
    for (Day day : {Day::Friday, Day::Saturday, Day::Sunday}) {
        cout << "\n– " << dayToString(day) << ":\n\n";

        map<string, vector<string>> timeToNames;
        for (const auto &s : group) {
            for (const auto &l : s.leaves) {
                if (l.day == day) {
                    string label = s.name;
                    if (l.isReport) label += " (рапорт)";
                    timeToNames[l.timeRange].push_back(label);
                }
            }
        }

        vector<pair<pair<int, int>, string>> ordered;
        for (const auto &entry : timeToNames) {
            ordered.push_back({intervalToMinutes(day, entry.first), entry.first});
        }
        sort(ordered.begin(), ordered.end(), [](const auto &a, const auto &b) {
            return a.first.first < b.first.first;
        });

        if (ordered.empty()) {
            cout << "(звільнень немає)\n";
            continue;
        }

        for (const auto &ord : ordered) {
            const string &timeRange = ord.second;
            cout << "(" << timeRange << "):\n";
            for (const auto &name : timeToNames[timeRange]) {
                cout << name << "\n";
            }
            cout << "\n";
        }
    }
}

bool overlaps(const pair<int, int> &a, const pair<int, int> &b) {
    return max(a.first, b.first) < min(a.second, b.second);
}

void removeOverlappingLeaves(Soldier &s, int start, int end) {
    vector<LeaveInterval> filtered;
    for (const auto &l : s.leaves) {
        auto interval = intervalToMinutes(l.day, l.timeRange);
        if (!overlaps(interval, {start, end})) {
            filtered.push_back(l);
        }
    }
    s.leaves.swap(filtered);
}

Day selectDay() {
    cout << "Оберіть день (1 - П'ятниця, 2 - Субота, 3 - Неділя): ";
    int choice; cin >> choice;
    switch (choice) {
        case 1: return Day::Friday;
        case 2: return Day::Saturday;
        case 3: return Day::Sunday;
        default: cout << "Невірний вибір, встановлено П'ятницю.\n"; return Day::Friday;
    }
}

string inputTimeRange() {
    cout << "Введіть інтервал у форматі HH:MM-HH:MM: ";
    string range; cin >> range;
    return range;
}

void assignNarjad(Soldier &s) {
    Day day = selectDay();
    auto startEnd = intervalToMinutes(day, "14:00-18:00");
    // End extends to next day 18:00
    startEnd.second += 24 * 60;
    removeOverlappingLeaves(s, startEnd.first, startEnd.second);
    cout << "Наряд додано. Звільнення скориговано.\n";
}

void assignChp(Soldier &s) {
    Day day = selectDay();
    auto interval = intervalToMinutes(day, "18:00-18:00");
    // already accounts for next day
    removeOverlappingLeaves(s, interval.first, interval.second);
    cout << "ЧП додано. Звільнення скориговано.\n";
}

void assignBR(Soldier &s) {
    Day day = selectDay();
    cout << "Кількість діб БР (мінімум 1): ";
    int days; cin >> days;
    days = max(1, days);
    auto interval = intervalToMinutes(day, "18:00-18:00");
    interval.second += (days - 1) * 24 * 60;
    removeOverlappingLeaves(s, interval.first, interval.second);
    cout << "БР додано на " << days << " діб. Звільнення скориговано.\n";
}

void addReportLeave(Soldier &s) {
    Day day = selectDay();
    string range = inputTimeRange();
    s.leaves.push_back(LeaveInterval{day, range, true});
    cout << "Звільнення за рапортом додано.\n";
}

void revokeSundayLeave(Soldier &s) {
    vector<LeaveInterval> filtered;
    for (const auto &l : s.leaves) {
        if (l.day != Day::Sunday) filtered.push_back(l);
    }
    s.leaves.swap(filtered);
    cout << "Звільнення на неділю скасовано.\n";
}

void showActions() {
    cout << "Доступні дії:\n";
    cout << "1 - Наряд\n";
    cout << "2 - ЧП\n";
    cout << "3 - БР\n";
    cout << "4 - Додати рапорт\n";
    cout << "5 - Забрати звільнення на неділю\n";
    cout << "6 - Показати розклад\n";
    cout << "0 - Повернутися до вибору солдата\n";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<Soldier> group = loadGroup("group_cN.txt");
    if (group.empty()) {
        cout << "Файл групи порожній або відсутній. Додайте курсантів у group_cN.txt.\n";
        return 0;
    }

    while (true) {
        cout << "\nСписок курсантів:\n";
        for (size_t i = 0; i < group.size(); ++i) {
            cout << i + 1 << ". " << group[i].name;
            if (group[i].isKyiv) cout << " (к)";
            cout << "\n";
        }
        cout << "0. Вихід\n";

        cout << "Оберіть курсанта: ";
        int choice; cin >> choice;
        if (choice == 0) break;
        if (choice < 1 || static_cast<size_t>(choice) > group.size()) {
            cout << "Невірний вибір.\n";
            continue;
        }

        Soldier &selected = group[choice - 1];
        while (true) {
            cout << "\nВибрано: " << selected.name << (selected.isKyiv ? " (к)" : "") << "\n";
            showActions();
            cout << "Ваш вибір: ";
            int action; cin >> action;
            switch (action) {
                case 1: assignNarjad(selected); break;
                case 2: assignChp(selected); break;
                case 3: assignBR(selected); break;
                case 4: addReportLeave(selected); break;
                case 5: revokeSundayLeave(selected); break;
                case 6: printSchedule(group); break;
                case 0: goto end_actions;
                default: cout << "Невідома команда.\n"; break;
            }
        }
        end_actions:;
    }

    cout << "\nФінальний графік:\n\n";
    printSchedule(group);
    return 0;
}
