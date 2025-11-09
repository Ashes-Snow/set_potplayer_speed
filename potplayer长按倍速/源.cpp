#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <conio.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "psapi.lib")

// PotPlayer按键配置
const char speed_increase_key = 'c';       // 增加速度按键
const char reset_speed_key = 'z';          // 重置速度按键
const int toggle_speed_key = VK_RIGHT;     // 切换速度按键
const int fast_forward_key = VK_RIGHT;     // 快进按键
const int rewind_key = VK_LEFT;            // 回退按键

// 线程安全的状态变量
std::atomic<bool> is_speed_key_pressed(false);
std::atomic<bool> is_rw_key_pressed(false);
std::atomic<double> key_press_start_time(0.0);
std::atomic<bool> is_speed_applied(false);
std::atomic<bool> program_running(true);
std::atomic<bool> potplayer_running(false);  // 用于标记PotPlayer是否已启动

// 获取当前时间戳(秒)
double get_current_time() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration<double>(duration).count();
}

// 检查PotPlayer是否处于焦点
bool is_potplayer_focused() {
    HWND foreground_hwnd = GetForegroundWindow();
    if (!foreground_hwnd) return false;

    // 获取窗口标题
    char title[256];
    GetWindowTextA(foreground_hwnd, title, sizeof(title));
    std::string window_title(title);

    // 检查标题是否包含PotPlayer特征
    if (window_title.find("PotPlayer") != std::string::npos ||
        window_title.find("PotPlay") != std::string::npos) {
        return true;
    }

    // 获取进程ID
    DWORD process_id;
    GetWindowThreadProcessId(foreground_hwnd, &process_id);
    if (process_id == 0) return false;

    // 检查进程名称
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
    if (!hProcess) return false;

    char process_name[256];
    GetModuleBaseNameA(hProcess, NULL, process_name, sizeof(process_name));
    CloseHandle(hProcess);

    return std::string(process_name).find("PotPlayer") != std::string::npos;
}

// 检查PotPlayer程序是否已启动
bool is_potplayer_running() {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;

    if (Process32First(hSnapshot, &entry)) {
        do {
            // 将WCHAR转换为char
            char exe_file[260];
            WideCharToMultiByte(CP_ACP, 0, entry.szExeFile, -1, exe_file, sizeof(exe_file), NULL, NULL);

            // 检查是否为PotPlayer
            if (std::string(exe_file).find("PotPlayer") != std::string::npos) {
                CloseHandle(hSnapshot);
                return true;
            }
        } while (Process32Next(hSnapshot, &entry));
    }

    CloseHandle(hSnapshot);
    return false;
}

// 发送按键事件
void send_key(char key) {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = VkKeyScanA(key) & 0xFF;
    SendInput(1, &input, sizeof(INPUT));

    // 模拟按键释放
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

// 发送组合键事件
void send_hotkey(char modifier, int key) {
    INPUT inputs[4] = { 0 };

    // 按下修饰键
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = modifier;
    SendInput(1, &inputs[0], sizeof(INPUT));

    // 按下按键
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = key;
    SendInput(1, &inputs[1], sizeof(INPUT));

    // 释放按键
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = key;
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &inputs[2], sizeof(INPUT));

    // 释放修饰键
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = modifier;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &inputs[3], sizeof(INPUT));
}

// 设置3倍速
void set_three_times_speed() {
    if (is_potplayer_focused()) {
        send_key(speed_increase_key);
        std::cout << "已设置为3倍速" << std::endl;
        is_speed_applied = true;
    }
}

// 重置为正常速度
void reset_to_normal_speed() {
    if (is_potplayer_focused()) {
        send_key(reset_speed_key);
        std::cout << "已恢复正常速度" << std::endl;
        is_speed_applied = false;
    }
}

// 快进5秒
void fast_forward_5_seconds() {
    if (is_potplayer_focused()) {
        send_hotkey(VK_CONTROL, VK_RIGHT);
        std::cout << "已快进5秒" << std::endl;
    }
}

// 回退5秒
void rewind_5_seconds() {
    if (is_potplayer_focused()) {
        send_hotkey(VK_CONTROL, VK_LEFT);
        std::cout << "已回退5秒" << std::endl;
    }
}

// 持续回退线程函数
void continuous_rewind() {
    while (is_rw_key_pressed && program_running) {
        if (is_potplayer_focused()) {
            rewind_5_seconds();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));  // 增加间隔，减少 CPU 占用
    }
}

// 按键状态处理线程
void handle_key_states() {
    std::thread rw_thread;

    while (program_running) {
        if (!potplayer_running.load()) {
            // 如果PotPlayer没有启动，等待直到程序启动
            potplayer_running.store(is_potplayer_running());  // 更新PotPlayer状态
            std::this_thread::sleep_for(std::chrono::seconds(1));  // 每秒检查一次
            continue;
        }

        // 检测速度切换键状态
        bool current_speed_state = (GetAsyncKeyState(toggle_speed_key) & 0x8000) != 0;

        if (current_speed_state && !is_speed_key_pressed) {
            is_speed_key_pressed = true;
            key_press_start_time = get_current_time();
            is_speed_applied = false;
        }
        else if (!current_speed_state && is_speed_key_pressed) {
            is_speed_key_pressed = false;
            if (is_speed_applied) {
                reset_to_normal_speed();
            }
            else {
                fast_forward_5_seconds();
            }
        }

        // 检测回退键状态
        bool current_rw_state = (GetAsyncKeyState(rewind_key) & 0x8000) != 0;

        if (current_rw_state && !is_rw_key_pressed) {
            is_rw_key_pressed = true;
            if (rw_thread.joinable()) {
                rw_thread.join();
            }
            rw_thread = std::thread(continuous_rewind);
        }
        else if (!current_rw_state && is_rw_key_pressed) {
            is_rw_key_pressed = false;
            if (rw_thread.joinable()) {
                rw_thread.join();
            }
        }

        // 检查是否需要设置三倍速
        if (is_speed_key_pressed && !is_speed_applied) {
            double elapsed = get_current_time() - key_press_start_time;
            if (elapsed >= 0.3) {
                set_three_times_speed();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));  // 控制检查频率
    }

    if (rw_thread.joinable()) {
        rw_thread.join();
    }
}

// 检查退出热键(Ctrl+Alt+T)
bool check_exit_hotkey() {
    return (GetAsyncKeyState(VK_CONTROL) & 0x8000) &&
        (GetAsyncKeyState(VK_MENU) & 0x8000) &&
        (GetAsyncKeyState('T') & 0x8000);
}

int main() {
    HWND hwnd = GetConsoleWindow();  // 获取当前控制台窗口句柄
    ShowWindow(hwnd, SW_HIDE);       // 隐藏控制台窗口

    std::cout << "程序已启动：" << std::endl;
    std::cout << "  - 按住右方向键0.3秒后切换到3倍速，松开恢复1倍速" << std::endl;
    std::cout << "  - 快速按下并松开右方向键快进5秒" << std::endl;
    std::cout << "  - 按住左方向键持续回退5秒，松开关闭" << std::endl;
    std::cout << "按Ctrl+Alt+T立即终止程序" << std::endl;

    // 启动按键处理线程
    std::thread monitor_thread(handle_key_states);

    // 主循环检测退出热键
    while (program_running) {
        if (check_exit_hotkey()) {
            std::cout << "\n强制终止程序..." << std::endl;
            program_running = false;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 主循环空闲时检测退出热键
    }

    // 等待线程结束
    if (monitor_thread.joinable()) {
        monitor_thread.join();
    }

    return 0;
}

