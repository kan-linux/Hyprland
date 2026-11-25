#include "Logger.hpp"
#include "RollingLogFollow.hpp"

#include "../../event/EventBus.hpp"

#include "../../config/ConfigValue.hpp"

#include <chrono>
#include <cstdio>
#include <print>
#include <unordered_set>

using namespace Log;

CLogger::CLogger() : m_isTrace(Env::isTrace()) {
    m_logger.setLogLevel(m_isTrace ? Hyprutils::CLI::LOG_TRACE : Hyprutils::CLI::LOG_DEBUG);
}

void CLogger::log(Hyprutils::CLI::eLogLevel level, const std::string_view& str) {
    if (!m_logsEnabled)
        return;

    if (level == Hyprutils::CLI::LOG_TRACE && !m_isTrace)
        return;

    if (SRollingLogFollow::get().isRunning())
        SRollingLogFollow::get().addLog(str);

    m_logger.log(level, str);
}

void CLogger::log(Hyprutils::CLI::eLogLevel level, const std::string_view loc, const std::string_view str) {
    if (!m_logsEnabled)
        return;

    if (level == Hyprutils::CLI::LOG_TRACE && !m_isTrace)
        return;

    log(level, std::format("[{}] {}", loc, str));
}

void CLogger::initIS(const std::string_view& IS) {
    // NOLINTNEXTLINE
    m_logger.setOutputFile(std::string{IS} + (ISDEBUG ? "/hyprlandd.log" : "/hyprland.log"));
    m_logger.setEnableRolling(true);
    m_logger.setEnableColor(false);
    m_logger.setEnableStdout(true);
    m_logger.setTime(false);
}

void CLogger::initCallbacks() {
    static auto P = Event::bus()->m_events.config.reloaded.listen([this]() { recheckCfg(); });
    recheckCfg();
}

void CLogger::recheckCfg() {
    static auto PDISABLELOGS  = CConfigValue<Config::BOOL>("debug:disable_logs");
    static auto PDISABLETIME  = CConfigValue<Config::BOOL>("debug:disable_time");
    static auto PENABLESTDOUT = CConfigValue<Config::BOOL>("debug:enable_stdout_logs");
    static auto PENABLECOLOR  = CConfigValue<Config::BOOL>("debug:colored_stdout_logs");

    m_logger.setEnableStdout(!*PDISABLELOGS && *PENABLESTDOUT);
    m_logsEnabled = !*PDISABLELOGS;
    m_logger.setTime(!*PDISABLETIME);
    m_logger.setEnableColor(*PENABLECOLOR);
}

const std::string& CLogger::rolling() {
    return m_logger.rollingLog();
}

Hyprutils::CLI::CLogger& CLogger::hu() {
    return m_logger;
}

// kanlinux: internal diagnostic marker for Hyprland — see Logger.hpp
void diagMark(const std::string& what) {
    static std::unordered_set<std::string> seen;
    if (!seen.insert(what).second)
        return;
    static const auto T0 = std::chrono::steady_clock::now();
    const auto        MS = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - T0).count();
    std::println(stderr, "diag[{}ms] {}", MS, what);
    if (FILE* f = fopen("/tmp/kanlinux-hyprland.log", "a")) {
        std::println(f, "diag[{}ms] {}", MS, what);
        fclose(f);
    }
}
