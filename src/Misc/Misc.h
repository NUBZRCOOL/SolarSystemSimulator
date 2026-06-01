#pragma once

#ifdef _WIN32
#include <windows.h>
#endif
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include "math.h"
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_glfw.h"
#include "../../imgui/imgui_impl_opengl3.h"
#include "../../imgui/implot.h"
#include "../../imgui/implot_internal.h"
#include <iomanip>
#include <sstream>
#include <ctime>


struct DatePickerState {
    bool is_initialized = false;
    std::tm stored_tm = {};   // Track calendar components cleanly
    double stored_ms = 0.0;   // Track precise sub-seconds
    int picker_level = 0;
};

inline time_t timegm(std::tm *tm) // calculate seconds without timezone
{
#ifdef _WIN32
    return _mkgmtime(tm);
#else
    return timegm(tm);
#endif
}

double StringToTimestamp(const std::string& datetime_str) {
    std::tm tm = {};
    std::stringstream ss(datetime_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    std::time_t s = timegm(&tm);
    double ms = 0.0;
    
    size_t dot = datetime_str.find_first_of('.');
    if (dot != std::string::npos) {
        ms = std::stod("0." + datetime_str.substr(dot + 1));
    }
    return static_cast<double>(s) + ms;
}

// Formats Unix timestamp back to UTC string 
std::string TimestampToString(double t, int gmtOffsetHours) {
    double local_time = t + (gmtOffsetHours * 3600.0);
    std::time_t s = static_cast<std::time_t>(std::floor(local_time));
    int ms = static_cast<int>(std::round((local_time - s) * 1000.0));
    if (ms >= 1000) { s += 1; ms -= 1000; } // Handle rounding overflows
    if (ms < 0) { s -= 1; ms += 1000; }
    
    std::tm *tm = std::gmtime(&s);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
    
    std::stringstream ss;
    ss << buffer << "." << std::setw(3) << std::setfill('0') << ms;
    
    // Add GMT offset display with formatting
    char sign = (gmtOffsetHours >= 0) ? '+' : '-';
    int absOffset = abs(gmtOffsetHours);
    ss << " (UTC" << sign << absOffset << ")";
    
    return ss.str();
}

int GetSystemTimezoneOffset() {
    #ifdef _WIN32
        TIME_ZONE_INFORMATION tzi;
        GetTimeZoneInformation(&tzi);
        return -tzi.Bias / 60;  // Bias is in minutes, negative for east of GMT
    #else
        time_t t = time(NULL);
        struct tm lt = *localtime(&t);
        struct tm gt = *gmtime(&t);
        int offset = (lt.tm_hour - gt.tm_hour);
        // Adjust for day boundaries
        if (lt.tm_yday != gt.tm_yday) {
            offset += (lt.tm_yday > gt.tm_yday) ? 24 : -24;
        }
        return offset;
    #endif
}

void ImPlot_DateTimePicker(const char* popup_id, double& current_unix_time, int& timezone_offset_hours) {
    if (ImGui::BeginPopup(popup_id)) {
        
        // Use a clean, simple static state container that mirrors standard GUI logic
        static bool is_initialized = false;
        static int sel_year = 2000, sel_month = 0, sel_day = 1;
        static int sel_hour = 0, sel_min = 0, sel_sec = 0;
        static int sel_ms = 0;

        // 1. First frame layout sync
        if (!is_initialized) {
            double local_time = current_unix_time + (timezone_offset_hours * 3600.0);
            std::time_t full_s = static_cast<std::time_t>(std::floor(local_time));
            double sub_seconds = local_time - full_s;
            
            std::tm* tm_local = std::gmtime(&full_s);
            if (tm_local) {
                sel_year  = tm_local->tm_year + 1900;
                sel_month = tm_local->tm_mon; // 0-11
                sel_day   = tm_local->tm_mday;
                sel_hour  = tm_local->tm_hour;
                sel_min   = tm_local->tm_min;
                sel_sec   = tm_local->tm_sec;
                sel_ms    = static_cast<int>(sub_seconds * 1000.0 + 0.5);
            }
            is_initialized = true;
        }

        // Timezone offset selector (at the top of the popup)
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Timezone Settings");
        ImGui::PushItemWidth(120);
        
        // GMT offset slider with +/- buttons
        if (ImGui::InputInt("GMT Offset", &timezone_offset_hours, 1, 1)) {
            // Clamp to reasonable range (-12 to +14)
            if (timezone_offset_hours < -12) timezone_offset_hours = -12;
            if (timezone_offset_hours > 14) timezone_offset_hours = 14;
            // Re-initialize the picker with new offset
            is_initialized = false;
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Reset##tzreset")) {
            timezone_offset_hours = GetSystemTimezoneOffset();
            is_initialized = false;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("GMT offset affects both the date picker display and the shown timestamp.\n"
                             "Positive = East of GMT (e.g., GMT+2)\n"
                             "Negative = West of GMT (e.g., GMT-5)\n"
                             "Click 'Reset' to use your system timezone.");
        }
        ImGui::PopItemWidth();
        
        // Show current offset as formatted string
        char offset_display[32];
        snprintf(offset_display, sizeof(offset_display), "Current: GMT%+d", timezone_offset_hours);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.0f), "%s", offset_display);
        
        ImGui::Separator();

        // 2. NATIVE IMGUI MONTH & YEAR PICKERS
        ImGui::Text("Date Selection");
        
        // Year Input
        ImGui::PushItemWidth(6 * ImGui::GetFontSize());
        ImGui::InputInt("##year_input", &sel_year, 1, 10);
        if (sel_year < 1) sel_year = 1;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Month Dropdown
        const char* months[] = { "January", "February", "March", "April", "May", "June", 
                                 "July", "August", "September", "October", "November", "December" };
        ImGui::PushItemWidth(8 * ImGui::GetFontSize());
        if (ImGui::BeginCombo("##month_combo", months[sel_month])) {
            for (int i = 0; i < 12; i++) {
                bool is_selected = (sel_month == i);
                if (ImGui::Selectable(months[i], is_selected)) {
                    sel_month = i;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Day Input - Calculate max days allowed in the chosen month
        std::tm days_lookup = {};
        days_lookup.tm_year = sel_year - 1900;
        days_lookup.tm_mon = sel_month + 1; // Next month
        days_lookup.tm_mday = 0;            // Day 0 of next month = Last day of current month
        timegm(&days_lookup);
        int max_days = days_lookup.tm_mday;

        ImGui::PushItemWidth(4 * ImGui::GetFontSize());
        ImGui::InputInt("##day_input", &sel_day, 1, 5);
        if (sel_day < 1) sel_day = 1;
        if (sel_day > max_days) sel_day = max_days;
        ImGui::PopItemWidth();

        ImGui::Separator();

        // 3. TIME PICKER
        ImGui::Text("Time Selection");
        
        // Construct temporary frame structure to feed ImPlot's visual clock interface safely
        std::tm frame_tm = {};
        frame_tm.tm_year = sel_year - 1900;
        frame_tm.tm_mon  = sel_month;
        frame_tm.tm_mday = sel_day;
        frame_tm.tm_hour = sel_hour;
        frame_tm.tm_min  = sel_min;
        frame_tm.tm_sec  = sel_sec;
        
        ImPlotTime im_time = ImPlotTime::FromDouble(static_cast<double>(timegm(&frame_tm)) + (sel_ms / 1000.0));
        ImPlot::GetStyle().Use24HourClock = true;
        
        if (ImPlot::ShowTimePicker("##time", &im_time)) {
            double t_val = im_time.ToDouble();
            std::time_t t_secs = static_cast<std::time_t>(std::floor(t_val));
            std::tm* decoded = std::gmtime(&t_secs);
            if (decoded) {
                sel_hour = decoded->tm_hour;
                sel_min  = decoded->tm_min;
                sel_sec  = decoded->tm_sec;
            }
        }

        // Millisecond precision sub-slider
        ImGui::SameLine(); 
        ImGui::Text("."); 
        ImGui::SameLine();
        ImGui::PushItemWidth(4 * ImGui::GetFontSize());
        if (ImGui::InputInt("##ms", &sel_ms, 0, 0)) {
            if (sel_ms < 0) sel_ms = 0;
            if (sel_ms > 999) sel_ms = 999;
        }
        ImGui::PopItemWidth();

        ImGui::Separator();

        // Show preview of selected local time
        std::tm preview_tm = {};
        preview_tm.tm_year = sel_year - 1900;
        preview_tm.tm_mon = sel_month;
        preview_tm.tm_mday = sel_day;
        preview_tm.tm_hour = sel_hour;
        preview_tm.tm_min = sel_min;
        preview_tm.tm_sec = sel_sec;
        
        char preview_str[128];
        strftime(preview_str, sizeof(preview_str), "%Y-%m-%d %H:%M:%S", &preview_tm);
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.5f, 1.0f), 
                          "Selected: %s.%03d %s", 
                          preview_str, sel_ms, offset_display);

        ImGui::Separator();

        // 4. DIALOG PROCESSING BUTTONS
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            std::tm final_tm = {};
            final_tm.tm_year = sel_year - 1900;
            final_tm.tm_mon  = sel_month;
            final_tm.tm_mday = sel_day;
            final_tm.tm_hour = sel_hour;
            final_tm.tm_min  = sel_min;
            final_tm.tm_sec  = sel_sec;

            std::time_t final_secs = timegm(&final_tm);
            double final_local_time = static_cast<double>(final_secs) + (sel_ms / 1000.0);
            
            // Adjust back to global simulation time scale removing local offset configuration
            current_unix_time = final_local_time - (timezone_offset_hours * 3600.0);
            
            is_initialized = false; // Reset setup trigger flag for subsequent popup clicks
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            is_initialized = false; // Discard changes cleanly
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Now", ImVec2(80, 0))) {
            // Set to current system time in UTC
            auto now = std::chrono::system_clock::now();
            auto now_utc = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
            double now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
            current_unix_time = static_cast<double>(now_utc) + (now_ms / 1000.0);
            is_initialized = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}