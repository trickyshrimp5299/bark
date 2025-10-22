// ...existing code...
#include <bits/stdc++.h>
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
#endif

using namespace std;
using ms = chrono::milliseconds;

static std::mt19937 rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());

int randInt(int a, int b){ return std::uniform_int_distribution<int>(a,b)(rng); }
double randReal(){ return std::uniform_real_distribution<double>(0,1)(rng); }

// Cross-platform sleep
void sleepMs(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

#if defined(_WIN32) || defined(_WIN64)
// Enable ANSI on Windows consoles if available
void enableAnsi() {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        if (h == INVALID_HANDLE_VALUE) return;
        DWORD mode;
        if (!GetConsoleMode(h, &mode)) return;
        SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);
}
#else
void enableAnsi() { /* no-op on POSIX (terminals usually support ANSI) */ }
#endif

// Minimal getch for UNIX
#if !(defined(_WIN32) || defined(_WIN64))
int _kbhit() {
        int bytes;
        ioctl(0, FIONREAD, &bytes);
        return bytes > 0;
}
int _getch() {
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        int c = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return c;
}
#endif

// Terminal helpers
void clearScreen() { cout << "\x1b[2J\x1b[H"; }
void hideCursor() { cout << "\x1b[?25l"; }
void showCursor() { cout << "\x1b[?25h"; }
void setColorDim() { cout << "\x1b[2m"; }
void resetAttrs() { cout << "\x1b[0m"; }

// Typewriter with jitter
void typeText(const string &s, int baseDelay = 25) {
        for (char c : s) {
                cout << c << flush;
                int jitter = randInt(-baseDelay/2, baseDelay/2);
                sleepMs(max(1, baseDelay + jitter));
                if (randReal() < 0.02) {
                        char g = char(33 + randInt(0, 30));
                        cout << g << flush;
                        sleepMs(40);
                        cout << '\b' << ' ' << '\b' << flush;
                }
        }
}

// Horizontal static line
string staticLine(int width) {
        string out; out.reserve(width);
        for (int i=0;i<width;i++) {
                int r = randInt(0, 100);
                if (r < 60) out += (randReal() < 0.5 ? ' ' : '.');
                else if (r < 85) out += '#';
                else out += char(33 + randInt(0, 30));
        }
        return out;
}

// Flicker effect
void flickerFrame(int width, int height) {
        for (int y=0;y<height;y++) {
                if (randReal() < 0.12) {
                        cout << setfill(' ') << setw(width) << "" << '\n';
                } else {
                        cout << staticLine(width) << '\n';
                }
        }
}

// Simple "broadcast" frames
void showBroadcastFrame(int width, int height, const vector<string>& lines) {
        int padTop = max(0, (height - (int)lines.size())/2);
        for (int i=0;i<padTop;i++) cout << "\n";
        for (auto &ln : lines) {
                int pad = max(0, (width - (int)ln.size())/2);
                cout << string(pad, ' ');
                cout << ln << "\n";
        }
        int padBottom = height - padTop - (int)lines.size();
        for (int i=0;i<padBottom;i++) cout << "\n";
}

// Simple UI: show status bar (extended: shows LOCK & fragments)
void showStatus(int freq, int sanity, bool locked, int collected) {
        cout << "\x1b[s"; // save cursor
        cout << "\x1b[1;1H"; // move to top-left
        cout << "\x1b[47;30m"; // white bg, black text
        cout << " FREQ: " << setw(3) << freq << " Hz ";
        cout << " | SANITY: ";
        int bars = sanity / 10;
        for (int i=0;i<10;i++) cout << (i<bars ? '#' : '-');
        cout << " ";
        cout << (locked ? " LOCKED " : "        ");
        cout << " | FRAG: " << setw(2) << collected;
        cout << " ";
        cout << "\x1b[0m";
        cout << "\x1b[u"; // restore cursor
}

// Story fragments
vector<string> fragments = {
        "we interrupted regular programming",
        "do not adjust your receiver",
        "they are near the static",
        "channels remember your name",
        "wait for the signal to finish",
        "someone is scraping the walls",
        "the numbers will not stop",
        "the mirror hums at midnight",
        "we are broadcasting from below"
};

// Show inventory/collected fragments
void showInventory(int termW, int termH, const vector<string>& collected) {
        clearScreen();
        vector<string> lines;
        lines.push_back("COLLECTED FRAGMENTS");
        lines.push_back("");
        if (collected.empty()) {
                lines.push_back("(none)");
        } else {
                for (size_t i=0;i<collected.size();i++) {
                        string s = to_string(i+1) + ". " + collected[i];
                        // wrap long lines
                        if ((int)s.size() > termW-4) s = s.substr(0, termW-7) + "...";
                        lines.push_back(s);
                }
        }
        lines.push_back("");
        lines.push_back("press any key to return");
        showBroadcastFrame(termW, termH, lines);
        while (!_kbhit()) sleepMs(50);
        _getch();
}

// Small auto-scan routine (cancellable)
void autoScan(int &frequency, int termW, int termH) {
        int start = max(0, frequency - 30);
        int end = min(999, frequency + 30);
        for (int f = start; f <= end; f += (f%2==0?1:2)) {
                frequency = f;
                clearScreen();
                showStatus(frequency, 100, false, 0);
                vector<string> s = {"AUTO-SCAN", "", "scanning: " + to_string(f)};
                showBroadcastFrame(termW, termH, s);
                if (_kbhit()) { int c = _getch(); (void)c; break; }
                sleepMs(30);
        }
}

// ...existing code...
int main() {
        ios::sync_with_stdio(false);
        cin.tie(nullptr);
        enableAnsi();
        hideCursor();
        srand((unsigned)time(nullptr));

        int termW = 80, termH = 24;
        // Try to query terminal size (best effort)
#if defined(_WIN32) || defined(_WIN64)
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
                termW = csbi.srWindow.Right - csbi.srWindow.Left + 1;
                termH = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        }
#else
        struct winsize w;
        if (ioctl(0, TIOCGWINSZ, &w) == 0) {
                termW = w.ws_col;
                termH = w.ws_row;
        }
#endif

        int frequency = 400; // arbitrary tuning param
        int sanity = 100;
        int ticks = 0;
        bool running = true;
        bool locked = false;
        string currentFragment;
        vector<string> collectedFragments;

        // secret frequency is randomized so game is replayable
        int secret = randInt(40, 900);

        // Intro + interactive menu
        clearScreen();
        vector<string> intro = {
                "ANALOG BROADCAST - DO NOT ADJUST",
                "",
                "Controls:",
                "  LEFT/RIGHT or A/D - tune",
                "  SPACE - attempt lock",
                "  I - view collected fragments",
                "  S - auto-scan (cancel any time)",
                "  R - randomize secret (makes it harder)",
                "  P - pause",
                "  Q - quit",
                "",
                "Choose difficulty: 1=Easy  2=Normal  3=Hard"
        };
        showBroadcastFrame(termW, termH, intro);
        while (!_kbhit()) sleepMs(50);
        int choice = _getch();
        int difficulty = 2;
        if (choice == '1') difficulty = 1;
        else if (choice == '3') difficulty = 3;
        // adjust secret proximity based on difficulty
        int sweetRange = (difficulty==1?12:(difficulty==3?4:8));
        // small tutorial output
        clearScreen();
        showBroadcastFrame(termW, termH, {"TUNING... press any key to begin"});
        while (!_kbhit()) sleepMs(50);
        _getch();

        // Main loop
        auto startTime = chrono::steady_clock::now();
        int holdDir = 0; // -1 left, 1 right, 0 none
        int holdTicks = 0;

        while (running) {
                ticks++;
                if (!locked && ticks % 40 == 0) sanity = max(0, sanity - randInt(1,4));

                clearScreen();
                showStatus(frequency, sanity, locked, (int)collectedFragments.size());

                double mode = randReal();
                if (locked) {
                        if (randReal() < 0.7) {
                                vector<string> story;
                                story.push_back("LOCK ACQUIRED");
                                story.push_back("");
                                story.push_back(currentFragment);
                                story.push_back("");
                                story.push_back("...listening...");
                                showBroadcastFrame(termW, termH, story);
                        } else {
                                flickerFrame(termW, termH);
                        }
                } else {
                        if (mode < 0.35) {
                                flickerFrame(termW, termH);
                        } else if (mode < 0.7) {
                                int take = randInt(0, (int)fragments.size()-1);
                                string frag = fragments[take];
                                for (char &c : frag) if (randReal() < 0.12 && c!=' ') c = char(33 + randInt(0,30));
                                vector<string> lines = {"-- SIGNAL --", "", frag};
                                showBroadcastFrame(termW, termH, lines);
                        } else {
                                vector<string> noise;
                                for (int i=0;i<7;i++) noise.push_back(staticLine(termW/2));
                                showBroadcastFrame(termW, termH, noise);
                        }
                }

                if (frequency % 37 == 0) {
                        cout << "\x1b[41m" << string(max(0, termW-2), ' ') << "\x1b[0m\n";
                } else {
                        cout << "\n";
                }

                // Input handling (non-blocking), improved: hold to accelerate tuning
                for (int dt = 0; dt < 40; dt += 10) {
                        if (_kbhit()) {
                                int c = _getch();
#if defined(_WIN32) || defined(_WIN64)
                                if (c == 0 || c == 224) {
                                        int k = _getch();
                                        if (k == 75) { // left
                                                holdDir = -1; holdTicks = 0;
                                                frequency = max(0, frequency - max(1, randInt(1,4)));
                                        } else if (k == 77) { // right
                                                holdDir = 1; holdTicks = 0;
                                                frequency = min(999, frequency + max(1, randInt(1,4)));
                                        }
                                } else
#endif
                                {
                                        if (c == 'q' || c == 'Q') { running = false; break; }
                                        if (c == ' ') {
                                                int dist = abs(frequency - secret);
                                                double chance = max(0.0, 0.25 - dist*0.01 * (3.0/difficulty));
                                                if (dist < sweetRange || (randReal() < chance)) {
                                                        locked = true;
                                                        currentFragment = fragments[randInt(0, (int)fragments.size()-1)];
                                                        collectedFragments.push_back(currentFragment);
                                                        sanity = max(0, sanity - randInt(2,8));
                                                        // small audible bell if terminal supports it
                                                        cout << '\a' << flush;
                                                } else {
                                                        sanity = max(0, sanity - randInt(1,4));
                                                        for (int b=0;b<3;b++) {
                                                                cout << "\x1b[7m" << string(termW, ' ') << "\x1b[0m\n";
                                                                sleepMs(60);
                                                        }
                                                }
                                        } else if (c == 'a' || c == 'A') {
                                                holdDir = -1; holdTicks = 0;
                                                frequency = max(0, frequency - 1);
                                        }
                                        else if (c == 'd' || c == 'D') {
                                                holdDir = 1; holdTicks = 0;
                                                frequency = min(999, frequency + 1);
                                        }
                                        else if (c == 'i' || c == 'I') {
                                                showInventory(termW, termH, collectedFragments);
                                        }
                                        else if (c == 's' || c == 'S') {
                                                autoScan(frequency, termW, termH);
                                        }
                                        else if (c == 'r' || c == 'R') {
                                                secret = randInt(40, 900);
                                                clearScreen();
                                                showBroadcastFrame(termW, termH, {"SECRET FREQ RANDOMIZED", "", "press any key to continue"});
                                                while (!_kbhit()) sleepMs(50);
                                                _getch();
                                        }
                                        else if (c == 'p' || c == 'P') {
                                                clearScreen();
                                                showBroadcastFrame(termW, termH, {"PAUSED", "", "press any key to resume"});
                                                while (!_kbhit()) sleepMs(50);
                                                _getch();
                                        }
                                }
                        } else {
                                // if holding direction, accelerate tuning a bit
                                if (holdDir != 0) {
                                        holdTicks++;
                                        int speed = 1 + holdTicks/3;
                                        if (holdDir < 0) frequency = max(0, frequency - speed);
                                        else frequency = min(999, frequency + speed);
                                        // slowly decay hold if no actual keypress seen
                                        if (holdTicks > 20) { holdDir = 0; holdTicks = 0; }
                                }
                        }
                        sleepMs(10);
                }

                if (locked && randReal() < 0.05) {
                        string reveal = fragments[randInt(0, (int)fragments.size()-1)];
                        currentFragment += " / " + reveal;
                        collectedFragments.push_back(reveal);
                        sanity = max(0, sanity - randInt(1,3));
                }

                if (sanity <= 0) {
                        clearScreen();
                        vector<string> end = {
                                "CONNECTION LOST",
                                "",
                                "you listened too long",
                                "",
                                "the signal took everything"
                        };
                        showBroadcastFrame(termW, termH, end);
                        cout << "\n";
                        typeText("...press any key to exit...");
                        while (!_kbhit()) sleepMs(50);
                        _getch();
                        break;
                }

                if (locked && currentFragment.size() > 140) {
                        clearScreen();
                        vector<string> win = {
                                "TRANSMISSION COMPLETE",
                                "",
                                "you stitched the broadcast together",
                                "",
                                "the world flickers, then resumes"
                        };
                        showBroadcastFrame(termW, termH, win);
                        cout << "\n";
                        typeText("...press any key to exit...");
                        while (!_kbhit()) sleepMs(50);
                        _getch();
                        break;
                }

                sleepMs(120);
        }

        resetAttrs();
        showCursor();
        cout << "\n";
        return 0;
}